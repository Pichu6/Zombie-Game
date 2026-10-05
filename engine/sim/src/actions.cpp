// Actors, primitives, processes and the action executor.
// Specs: docs/tier0/specs/14-action-executor.md, 15-items-recipes.md
#include "zs/actions.h"

#include <algorithm>
#include <stdexcept>

#include "toml.hpp"
#include "zs/detmath.h"

namespace zs {
namespace {

// MET values: Compendium of Physical Activities (Ainsworth et al. 2011) where a matching activity
// exists (walking, carpentry 3.0-3.5, hammering nails 4.5? marked estimate), noise at 1 m estimates.
constexpr PrimitiveInfo kPrimitives[] = {
    {"walk", 3.0, 0, Requires::Moving},           // speed-driven: Pandolf replaces the MET (spec 11)
    {"run", 9.0, 0, Requires::Moving},
    {"crawl", 3.5, 0, Requires::Moving},          // estimate
    {"climb", 6.0, 0, Requires::Moving},          // estimate
    {"hold", 1.8, 0, Requires::Manipulation},
    {"strike", 4.5, 95, Requires::Manipulation},  // hammering: ~4.5 MET (estimate); a hammer blow ~95 dB at 1 m
    {"push_pull", 5.0, 50, Requires::Moving},     // estimate
    {"cut", 3.0, 55, Requires::Manipulation},     // estimate
    {"pry", 4.0, 80, Requires::Manipulation},     // estimate: wood splitting and nails squealing
    {"fine", 1.8, 30, Requires::Manipulation},    // e.g. first aid
    {"consume", 1.5, 35, Requires::Consciousness},
    {"rest", 1.0, 0, Requires::Consciousness},
    {"sleep", 0.95, 0, Requires::Consciousness},
    {"exercise", 6.0, 45, Requires::Moving},      // calisthenics, vigorous (Compendium 02020)
    {"search", 2.5, 40, Requires::Manipulation},  // estimate
    {"wait", 1.3, 0, Requires::Consciousness},
};
static_assert(sizeof(kPrimitives) / sizeof(kPrimitives[0]) == static_cast<std::size_t>(Primitive::Count));

constexpr const char* kTradeNames[] = {"everyday", "first_aid", "carpentry", "mechanics", "electronics"};
// Everyone knows everyday processes; first aid and carpentry at a novice level; trades are learned.
constexpr std::array<int, kTradeCount> kDefaultSkill = {50, 20, 20, -1, -1};

// Salvage loss by joint (spec 08): condition damage, never mass.
std::int64_t salvage_loss(Joint j) {
  switch (j) {
    case Joint::Loose: return 0;
    case Joint::Screwed: return 0;
    case Joint::Nailed: return 50'000;
    case Joint::Welded: return 200'000;
    case Joint::Glued: return 300'000;
  }
  return 0;
}

Trade trade_from(const std::string& s) {
  for (std::size_t i = 0; i < kTradeCount; ++i)
    if (s == kTradeNames[i]) return static_cast<Trade>(i);
  throw std::runtime_error("processes: unknown trade " + s);
}

EffectOp op_from(const std::string& s) {
  static const std::pair<const char*, EffectOp> ops[] = {
      {"eat", EffectOp::Eat},       {"drink", EffectOp::Drink},   {"fill", EffectOp::Fill},
      {"use_stock", EffectOp::UseStock}, {"treat", EffectOp::Treat}, {"board", EffectOp::Board},
      {"salvage", EffectOp::Salvage}, {"take", EffectOp::Take},   {"sleep", EffectOp::Sleep},
      {"power", EffectOp::Power},     {"cover", EffectOp::Cover}};
  for (const auto& [name, op] : ops)
    if (s == name) return op;
  throw std::runtime_error("processes: unknown effect op " + s);
}

Prop need_prop(const std::string& s) {
  auto p = prop_from(s);
  if (!p) throw std::runtime_error("processes: unknown property " + s);
  return *p;
}

std::optional<ResourceId> resource_id(const Ledger& l, const std::string& name) {
  const auto& names = l.resource_names();
  for (std::size_t i = 0; i < names.size(); ++i)
    if (names[i] == name) return static_cast<ResourceId>(i);
  return std::nullopt;
}

}  // namespace

const PrimitiveInfo& primitive_info(Primitive p) { return kPrimitives[static_cast<std::size_t>(p)]; }

std::optional<Primitive> primitive_from(const std::string& name) {
  for (std::size_t i = 0; i < static_cast<std::size_t>(Primitive::Count); ++i)
    if (name == kPrimitives[i].name) return static_cast<Primitive>(i);
  return std::nullopt;
}

std::int64_t pack_cell(const Cell& c) {
  return (static_cast<std::int64_t>(c.z + 128) << 32) | (static_cast<std::int64_t>(c.x & 0xffff) << 16) |
         static_cast<std::int64_t>(c.y & 0xffff);
}
Cell unpack_cell(std::int64_t v) {
  return Cell{static_cast<int>((v >> 32) & 0xff) - 128, static_cast<int>((v >> 16) & 0xffff),
              static_cast<int>(v & 0xffff)};
}

const char* to_string(Reason r) {
  switch (r) {
    case Reason::Ok: return "ok";
    case Reason::UnknownProcess: return "you don't know how";
    case Reason::WrongTarget: return "not on that";
    case Reason::Dead: return "dead";
    case Reason::Unconscious: return "unconscious";
    case Reason::CannotMove: return "can't move";
    case Reason::CannotUseHands: return "can't use hands";
    case Reason::NoTool: return "no suitable tool in hand";
    case Reason::MissingItem: return "missing an item";
    case Reason::MissingStock: return "not enough of a supply";
    case Reason::NoRoute: return "can't get there";
    case Reason::TargetGone: return "it's gone";
    case Reason::InUse: return "someone else is using it";
    case Reason::Full: return "no room";
    case Reason::Interrupted: return "interrupted";
    case Reason::Asleep: return "asleep";
    case Reason::TooHeavy: return "too heavy to carry";
    case Reason::AlreadyDone: return "already that way";
    case Reason::NoFuel: return "no fuel";
  }
  return "?";
}

// ---- Setup ---------------------------------------------------------------------------------------

Actors::Actors(World& w, Assemblies& items, SiteMap& map, Bodies& bodies) : items_(items), map_(map), bodies_(bodies) {
  disinfectant_ = w.ledger.add_resource("disinfectant_ul");
  used_ = w.ledger.add_owner("sink:used");
}

void Actors::load_processes(const std::string& path) {
  const toml::table t = toml::parse_file(path);
  const auto* list = t["process"].as_array();
  if (!list) throw std::runtime_error("processes: no [[process]] tables");
  for (const auto& node : *list) {
    const auto& pt = *node.as_table();
    ProcessDef p;
    p.id = pt["id"].value<std::string>().value();
    p.name = pt["name"].value_or(p.id);
    const auto prim = primitive_from(pt["primitive"].value<std::string>().value());
    if (!prim) throw std::runtime_error("processes: unknown primitive in " + p.id);
    p.primitive = *prim;
    const std::string target = pt["target"].value<std::string>().value();
    const std::string kind = target.substr(0, target.find(':'));
    p.target_match = target.find(':') == std::string::npos ? "" : target.substr(target.find(':') + 1);
    if (kind == "self") p.target = TargetKind::Self;
    else if (kind == "wound") p.target = TargetKind::Wound;
    else if (kind == "object") p.target = TargetKind::Object;
    else if (kind == "item") p.target = TargetKind::Item;
    else if (kind == "edge") p.target = TargetKind::Edge;
    else throw std::runtime_error("processes: unknown target " + target);
    p.trade = trade_from(pt["trade"].value_or(std::string("everyday")));
    p.time_s = pt["time_s"].value_or(10.0);
    p.resumable = pt["resumable"].value_or(false);
    p.source = pt["source"].value_or(std::string{});
    if (const auto* a = pt["tools"].as_array())
      for (const auto& n : *a) {
        const auto& x = *n.as_table();
        p.tools.push_back({need_prop(x["prop"].value<std::string>().value()), x["min"].value_or(0.0), x["ref"].value_or(1.0)});
      }
    if (const auto* a = pt["uses"].as_array())
      for (const auto& n : *a) {
        const auto& x = *n.as_table();
        p.uses.push_back({need_prop(x["prop"].value<std::string>().value()), x["min"].value_or(0.0), x["optional"].value_or(false)});
      }
    if (const auto* a = pt["stocks"].as_array())
      for (const auto& n : *a) {
        const auto& x = *n.as_table();
        p.stocks.push_back({x["resource"].value<std::string>().value(), x["amount"].value<std::int64_t>().value(),
                            x["from"].value_or(std::string{}) == "target"});
      }
    if (const auto* a = pt["effects"].as_array())
      for (const auto& n : *a) {
        const auto& x = *n.as_table();
        p.effects.push_back({op_from(x["op"].value<std::string>().value()), x["resource"].value_or(std::string{}),
                             x["amount"].value_or<std::int64_t>(0), x["how"].value_or(std::string{})});
      }
    processes_.push_back(std::move(p));
  }
}

std::optional<std::size_t> Actors::find_process(const std::string& id) const {
  for (std::size_t i = 0; i < processes_.size(); ++i)
    if (processes_[i].id == id) return i;
  return std::nullopt;
}

void Actors::populate(World& w) {
  auto& l = w.ledger;
  // Amounts in map files use readable units; the ledger uses J and uL.
  struct AmountKey {
    const char* key;
    const char* resource;
    double scale;
  };
  static constexpr AmountKey kAmounts[] = {
      {"food_kj", "food_j", 1e3},          {"tap_water_l", "body_water_ul", 1e6}, {"water_ml", "body_water_ul", 1e3},
      {"disinfectant_ml", "disinfectant_ul", 1e3}, {"gasoline_l", "gasoline_ul", 1e6}};
  liquids_.clear();
  for (const char* name : {"body_water_ul", "disinfectant_ul", "gasoline_ul"})
    if (auto r = resource_id(l, name)) liquids_.push_back(*r);
  auto mint_amounts = [&](OwnerId owner, const MapObject& o) {
    for (const auto& [key, v] : o.amounts) {
      const AmountKey* k = nullptr;
      for (const AmountKey& c : kAmounts)
        if (key == c.key) k = &c;
      const auto r = k ? resource_id(l, k->resource) : std::nullopt;
      if (!r) throw std::runtime_error("map: unknown amount " + key);
      l.mint(owner, *r, static_cast<std::int64_t>(v * k->scale));
    }
  };
  const auto& objs = map_.objects();
  for (std::size_t i = 0; i < objs.size(); ++i) {
    const MapObject& o = objs[i];
    if (o.kind == "item") {
      const auto def = items_.find_def(o.id);
      if (!def) throw std::runtime_error("map: unknown item " + o.id);
      const EntityId id = items_.create(w, *def, 50, floor_at(o.at));
      object_entities_.push_back(id);
      if (items_.def(*def).props[static_cast<std::size_t>(Prop::ContainerMl)] > 0) {
        const OwnerId owner = l.add_owner("container:" + o.id + "#" + std::to_string(id));
        stores_[id] = owner;
        mint_amounts(owner, o);
      }
      continue;
    }
    const EntityId e = w.new_entity();
    object_entities_.push_back(e);
    object_index_[e] = i;
    if (!o.amounts.empty()) {
      const OwnerId owner = l.add_owner("store:" + o.kind + (o.id.empty() ? "" : ":" + o.id));
      stores_[e] = owner;
      mint_amounts(owner, o);
    }
  }
  // Index the map's edge assemblies by where they are.
  for (int z : map_.levels())
    for (int vertical = 0; vertical < 2; ++vertical)
      for (int y = 0; y <= map_.height() - vertical; ++y)
        for (int x = 0; x <= map_.width() - (vertical ? 0 : 1); ++x) {
          const EdgeRef ref{z, x, y, vertical == 1};
          const Edge& ed = map_.edge(ref);
          if (ed.assembly != 0) edge_of_[ed.assembly] = ref;
        }
}

EntityId Actors::add_actor(World& w, EntityId body, const Cell& at) {
  Actor a;
  a.body = body;
  a.pos = at;
  a.skill = kDefaultSkill;
  a.inventory = w.ledger.add_owner("inventory:" + bodies_.get(body).label);
  actors_[body] = a;
  return body;
}

std::optional<EntityId> Actors::find_object(const std::string& kind, const std::string& id) const {
  for (const auto& [e, i] : object_index_)
    if (map_.objects()[i].kind == kind && (id.empty() || map_.objects()[i].id == id)) return e;
  return std::nullopt;
}

std::optional<OwnerId> Actors::store_of(EntityId holder) const {
  auto it = stores_.find(holder);
  if (it == stores_.end()) return std::nullopt;
  return it->second;
}

EntityId Actors::floor_at(const Cell& c) const {
  const auto& lv = map_.levels();
  const auto li = static_cast<std::size_t>(std::find(lv.begin(), lv.end(), c.z) - lv.begin());
  const auto W = static_cast<EntityId>(map_.width()), H = static_cast<EntityId>(map_.height());
  return kFloorBase + (li * H + static_cast<EntityId>(c.y)) * W + static_cast<EntityId>(c.x);
}

std::optional<Cell> Actors::floor_cell(EntityId e) const {
  if (e < kFloorBase || e >= World::kFirstEntity) return std::nullopt;
  const auto W = static_cast<EntityId>(map_.width()), H = static_cast<EntityId>(map_.height());
  const EntityId i = e - kFloorBase;
  const auto li = static_cast<std::size_t>(i / (W * H));
  if (li >= map_.levels().size()) return std::nullopt;
  return Cell{map_.levels()[li], static_cast<int>(i % W), static_cast<int>((i / W) % H)};
}

std::optional<Cell> Actors::cell_of(const World& w, EntityId e) const {
  for (int depth = 0; depth < 64 && e != Locations::kWorld; ++depth) {
    if (auto it = actors_.find(e); it != actors_.end()) return it->second.pos;
    if (auto c = floor_cell(e)) return c;
    if (auto it = object_index_.find(e); it != object_index_.end()) return map_.objects()[it->second].at;
    if (auto it = edge_of_.find(e); it != edge_of_.end()) return Cell{it->second.z, it->second.x, it->second.y};
    e = w.locations.parent_of(e);
  }
  return std::nullopt;
}

std::vector<EntityId> Actors::carried(const World& w, EntityId body) const {
  std::vector<EntityId> out;
  for (EntityId e : w.locations.children(body))
    if (items_.exists(e)) out.push_back(e);
  return out;
}

double Actors::carried_kg(const World& w, EntityId body) const {
  double g = 0;
  for (EntityId e : carried(w, body)) {
    g += static_cast<double>(items_.mass_g(e));
    if (auto s = store_of(e))
      g += static_cast<double>(w.ledger.balance(*s, bodies_.ids().water_ul) + w.ledger.balance(*s, disinfectant_)) / 1000.0;
  }
  return g / 1000.0;
}

void Actors::recompute_carried(const World& w, Actor& a) { a.carried_kg = carried_kg(w, a.body); }

// ---- Checks (shared by preview and execution) ---------------------------------------------------------

std::optional<EntityId> Actors::carried_with(const World& w, const Actor& a, Prop prop, double min) const {
  std::optional<EntityId> best;
  double best_v = 0;
  for (EntityId e : carried(w, a.body)) {
    const double v = items_.prop(e, prop);
    if (v >= min && v > 0 && (!best || v > best_v)) {
      best = e;
      best_v = v;
    }
  }
  return best;
}

std::optional<EntityId> Actors::fitting_part(const World& w, const Actor& a, const std::string& family, Prop prop) const {
  std::optional<EntityId> best;
  double best_v = 0;
  for (EntityId e : carried(w, a.body)) {
    if (items_.def(items_.get(e).def).family != family) continue;  // the slot decides what fits
    const double v = items_.prop(e, prop);
    if (v >= 0.5 && (!best || v > best_v)) {
      best = e;
      best_v = v;
    }
  }
  return best;
}

std::int64_t Actors::carried_stock(const World& w, const Actor& a, ResourceId r) const {
  std::int64_t total = 0;
  for (EntityId e : carried(w, a.body))
    if (auto s = store_of(e)) total += w.ledger.balance(*s, r);
  return total;
}

bool Actors::target_matches(const World& w, const ProcessDef& p, const Actor& a, EntityId target) const {
  switch (p.target) {
    case TargetKind::Self: return target == a.body;
    case TargetKind::Wound:
      for (const Wound& wd : bodies_.get(a.body).wounds)
        if (wd.id == target)
          return p.target_match.empty() || p.target_match == to_string(wd.type);
      return false;
    case TargetKind::Object: {
      auto it = object_index_.find(target);
      return it != object_index_.end() && (p.target_match.empty() || map_.objects()[it->second].kind == p.target_match);
    }
    case TargetKind::Item: {
      if (!items_.exists(target)) return false;
      if (!p.target_match.empty() && items_.def(items_.get(target).def).family != p.target_match) return false;
      // Loose = lying on a floor or at a map object; not mounted, not built into the map, not in hands.
      const EntityId parent = w.locations.parent_of(target);
      return floor_cell(parent).has_value() || object_index_.contains(parent);
    }
    case TargetKind::Edge: {
      auto it = edge_of_.find(target);
      return it != edge_of_.end() &&
             (p.target_match.empty() || items_.def(items_.get(target).def).family == p.target_match);
    }
  }
  return false;
}

std::vector<Cell> Actors::reach_cells(const World& w, const ProcessDef& p, const Actor& a, EntityId target) const {
  switch (p.target) {
    case TargetKind::Self:
    case TargetKind::Wound: return {a.pos};
    case TargetKind::Edge: {
      const EdgeRef& r = edge_of_.at(target);
      const Cell inner{r.z, r.x, r.y};
      const Cell outer = r.vertical ? Cell{r.z, r.x - 1, r.y} : Cell{r.z, r.x, r.y - 1};
      std::vector<Cell> out;
      for (const Cell& c : {inner, outer})
        if (map_.in_bounds(c) && map_.walkable(c)) out.push_back(c);
      return out;
    }
    default: {
      auto c = cell_of(w, target);
      if (!c) return {};
      return {*c};
    }
  }
}

double Actors::work_time(const World&, const Actor& a, const ProcessDef& p, double tool_factor) const {
  const int skill = std::max(0, a.skill[static_cast<std::size_t>(p.trade)]);
  return p.time_s / tool_factor * (1.25 - skill / 200.0);
}

Actors::Found Actors::check(const World& w, const Actor& a, const ProcessDef& p, EntityId target) const {
  Found f;
  if (a.skill[static_cast<std::size_t>(p.trade)] < 0) return {Reason::UnknownProcess};
  const Body& b = bodies_.get(a.body);
  if (!b.alive) return {Reason::Dead};
  if (b.activity.asleep && p.primitive != Primitive::Sleep) return {Reason::Asleep};
  const Capabilities c = bodies_.capabilities(b, w.clock.tick);
  if (c.consciousness < 0.3) return {Reason::Unconscious};
  const Requires need = primitive_info(p.primitive).needs;
  if (need == Requires::Moving && c.moving <= 0) return {Reason::CannotMove};
  if (need == Requires::Manipulation && std::max(c.manipulation_l, c.manipulation_r) < 0.05) return {Reason::CannotUseHands};
  if (!target_matches(w, p, a, target)) {
    const bool gone = (p.target == TargetKind::Item && !items_.exists(target));
    return {gone ? Reason::TargetGone : Reason::WrongTarget};
  }
  if (auto it = reserved_.find(target); it != reserved_.end() && it->second != a.body) return {Reason::InUse};
  if (!p.tools.empty()) {
    double best = 0;
    for (const ToolNeed& t : p.tools)
      if (auto e = carried_with(w, a, t.prop, t.min)) {
        const double factor = std::clamp(items_.prop(*e, t.prop) / t.ref, 0.25, 4.0);
        if (factor > best) {
          best = factor;
          f.tool = *e;
        }
      }
    if (f.tool == 0) return {Reason::NoTool};
    f.tool_factor = best;
  }
  for (const UseNeed& u : p.uses)
    if (!u.optional && !carried_with(w, a, u.prop, u.min)) return {Reason::MissingItem};
  if (p.primitive == Primitive::Sleep && b.activity.asleep) return {Reason::Asleep};
  for (const StockNeed& s : p.stocks) {
    const auto r = resource_id(w.ledger, s.resource);
    if (!r) return {Reason::MissingStock};
    std::int64_t have = 0;
    if (s.from_target) {
      if (auto st = store_of(target)) have = w.ledger.balance(*st, *r);
    } else {
      have = carried_stock(w, a, *r);
    }
    if (have < s.amount) return {Reason::MissingStock};
  }
  for (const Effect& e : p.effects) {
    if (e.op == EffectOp::Power) {
      const Reason r = power_check ? power_check(w, a.body, target, e.how) : Reason::WrongTarget;
      if (r != Reason::Ok) return {r};
    }
    if (e.op == EffectOp::Cover && covered(w, target)) return {Reason::Full};
    if (e.op == EffectOp::Take &&
        a.carried_kg + static_cast<double>(items_.mass_g(target)) / 1000.0 > bodies_.carry_limit_kg(b, w.clock.tick))
      return {Reason::TooHeavy};
    if (e.op == EffectOp::Board) {
      if (!fitting_part(w, a, "plank", Prop::FlatBoard)) return {Reason::MissingItem};
      const Edge& ed = map_.edge(edge_of_.at(target));
      if (ed.barricade != 0) {
        bool room = false;
        const Instance& bar = items_.get(ed.barricade);
        const ItemDef& d = items_.def(bar.def);
        for (std::size_t i = 0; i < d.slots.size(); ++i)
          if (d.slots[i].accepts == "plank" && bar.slots[i] == 0) room = true;
        if (!room) return {Reason::Full};
      } else if (!fitting_part(w, a, "fasteners", Prop::Fastener)) {
        return {Reason::MissingItem};  // the first board needs nails
      }
    }
  }
  // Access: somewhere to stand that can be reached.
  const auto cells = reach_cells(w, p, a, target);
  bool reachable = false;
  for (const Cell& cell : cells)
    if (cell == a.pos || !map_.find_path(a.pos, cell).empty()) {
      reachable = true;
      break;
    }
  if (!reachable) return {Reason::NoRoute};
  return f;
}

double Actors::light_factor(const World& w, const Actor& a, const ProcessDef& p) const {
  if (!light_lux) return 1;
  switch (p.primitive) {
    case Primitive::Fine:
    case Primitive::Strike:
    case Primitive::Pry:
    case Primitive::Cut: break;
    default: return 1;
  }
  const double lux = light_lux(w, a.pos);
  if (lux <= 10) return 0.5;
  if (lux >= 100) return 1;
  return 0.5 + 0.5 * detmath::log10(lux / 10);  // log-linear: the eye works in ratios (estimate)
}

bool Actors::covered(const World& w, EntityId window) const {
  for (EntityId c : w.locations.children(window))
    if (items_.exists(c) && items_.prop(c, Prop::ClothAreaM2) >= 1.0) return true;
  return false;
}

Offer Actors::preview(const World& w, EntityId body, std::size_t process, EntityId target) const {
  const Actor& a = actors_.at(body);
  const ProcessDef& p = processes_.at(process);
  const Found f = check(w, a, p, target);
  Offer o;
  o.process = process;
  o.target = target;
  o.reason = f.reason;
  o.enabled = f.reason == Reason::Ok;
  o.time_s = work_time(w, a, p, f.tool_factor) / light_factor(w, a, p);
  return o;
}

std::vector<Offer> Actors::offers_for(const World& w, EntityId body, EntityId target) const {
  std::vector<Offer> out;
  const Actor& a = actors_.at(body);
  for (std::size_t i = 0; i < processes_.size(); ++i) {
    const ProcessDef& p = processes_[i];
    if (a.skill[static_cast<std::size_t>(p.trade)] < 0) continue;  // knowledge hides the offer
    if (!target_matches(w, p, a, target)) continue;                 // not an action on this thing
    Offer o = preview(w, body, i, target);
    if (o.reason == Reason::WrongTarget) continue;  // e.g. switching on something that takes no power
    out.push_back(o);
  }
  return out;
}

// ---- Execution ----------------------------------------------------------------------------------------

void Actors::apply_command(World& w, const Command& c, EventId cause) {
  auto it = actors_.find(c.subject);
  if (it == actors_.end()) return;
  Actor& a = it->second;
  if (static_cast<CommandType>(c.type) == CommandType::Cancel) {
    a.queue.clear();
    if (a.action.phase != Phase::Idle) interrupt(w, a, Reason::Interrupted);
    return;
  }
  a.queue.emplace_back(c, cause);
}

void Actors::start_next(World& w, Actor& a) {
  while (a.action.phase == Phase::Idle && !a.queue.empty()) {
    const auto [c, cause] = a.queue.front();
    a.queue.pop_front();
    start(w, a, c, cause);
  }
}

void Actors::start(World& w, Actor& a, const Command& c, EventId cause) {
  Action act;
  if (static_cast<CommandType>(c.type) == CommandType::MoveTo) {
    act.goal = unpack_cell(c.a);
    act.path = map_.find_path(a.pos, act.goal);
    if (act.path.empty()) {
      w.emit(EventType::ActionRefused, a.body, cause, -1, static_cast<std::int64_t>(Reason::NoRoute));
      return;
    }
    act.phase = Phase::Approach;
    act.path_next = 1;
    a.action = act;
    a.cause = cause;
    return;
  }
  const auto pi = static_cast<std::size_t>(c.a);
  if (pi >= processes_.size()) {
    w.emit(EventType::ActionRefused, a.body, cause, c.a, static_cast<std::int64_t>(Reason::UnknownProcess));
    return;
  }
  const ProcessDef& p = processes_[pi];
  const auto target = static_cast<EntityId>(c.b);
  const Found f = check(w, a, p, target);
  if (f.reason != Reason::Ok) {
    w.emit(EventType::ActionRefused, a.body, cause, c.a, static_cast<std::int64_t>(f.reason));
    return;
  }
  act.process = static_cast<int>(pi);
  act.target = target;
  act.tool = f.tool;
  act.total_s = work_time(w, a, p, f.tool_factor);
  if (p.resumable)
    if (auto it = resume_.find({act.process, target}); it != resume_.end()) act.progress_s = it->second;
  if (p.target == TargetKind::Item || p.target == TargetKind::Edge) reserved_[target] = a.body;
  // Walk to the nearest place to work from (fixed candidate order, shortest path wins).
  const auto cells = reach_cells(w, p, a, target);
  bool there = false;
  for (const Cell& cell : cells) there = there || cell == a.pos;
  if (there) {
    act.phase = Phase::Work;
  } else {
    for (const Cell& cell : cells) {
      auto path = map_.find_path(a.pos, cell);
      if (!path.empty() && (act.path.empty() || path.size() < act.path.size())) act.path = std::move(path);
    }
    act.phase = Phase::Approach;
    act.path_next = 1;
  }
  a.action = act;
  a.cause = cause;
  if (p.primitive == Primitive::Sleep) {
    Body& b = bodies_.get(a.body);
    b.activity = Activity{};
    b.activity.asleep = true;
    b.posture = Posture::Lie;
    w.emit(EventType::FellAsleep, a.body, cause, 0);
  }
}

void Actors::interrupt(World& w, Actor& a, Reason why) {
  Action& act = a.action;
  if (act.held != 0) {  // an item in the hands mid-transfer drops where the actor stands
    w.locations.place(act.held, floor_at(a.pos));
    act.held = 0;
  }
  if (act.process >= 0) {
    const ProcessDef& p = processes_[static_cast<std::size_t>(act.process)];
    if (p.resumable && act.progress_s > 0) resume_[{act.process, act.target}] = act.progress_s;
    if (auto it = reserved_.find(act.target); it != reserved_.end() && it->second == a.body) reserved_.erase(it);
  }
  w.emit(EventType::ActionInterrupted, a.body, a.cause, act.process, static_cast<std::int64_t>(why));
  a.action = Action{};
  a.queue.clear();
  recompute_carried(w, a);
}

void Actors::finish(World& w, Actor& a) {
  Action& act = a.action;
  if (act.process >= 0) {
    const ProcessDef& p = processes_[static_cast<std::size_t>(act.process)];
    for (const Effect& e : p.effects) apply_effect(w, a, p, e);
    resume_.erase({act.process, act.target});
    if (auto it = reserved_.find(act.target); it != reserved_.end() && it->second == a.body) reserved_.erase(it);
    w.emit(EventType::ActionDone, a.body, a.cause, act.process, static_cast<std::int64_t>(act.target));
  }
  a.action = Action{};
  recompute_carried(w, a);
}

void Actors::apply_effect(World& w, Actor& a, const ProcessDef& p, const Effect& e) {
  auto& l = w.ledger;
  const std::int64_t t = w.clock.tick;
  const EntityId target = a.action.target;
  const ResourceId water = bodies_.ids().water_ul;
  // Takes `amount` of `r` from carried containers in id order into `to`. Returns what moved.
  auto from_containers = [&](ResourceId r, std::int64_t amount, OwnerId to) {
    std::int64_t moved = 0;
    for (EntityId c : carried(w, a.body)) {
      if (moved >= amount) break;
      if (auto s = store_of(c)) moved += l.transfer(t, *s, to, r, amount - moved, a.cause);
    }
    return moved;
  };
  switch (e.op) {
    case EffectOp::Eat:
      if (auto s = store_of(target)) bodies_.eat(w, a.body, *s, e.amount, a.cause);
      break;
    case EffectOp::Drink: {
      const OwnerId body_owner = bodies_.get(a.body).owner;
      if (auto s = store_of(target)) l.transfer(t, *s, body_owner, water, e.amount, a.cause);
      else from_containers(water, e.amount, body_owner);
      break;
    }
    case EffectOp::Fill: {
      const auto s = store_of(target);
      if (!s) break;
      for (EntityId c : carried(w, a.body)) {
        const auto cs = store_of(c);
        if (!cs) continue;
        const auto cap = static_cast<std::int64_t>(items_.prop(c, Prop::ContainerMl) * 1000);
        std::int64_t other = 0;  // a container holds one liquid: never water into a fuel can
        for (ResourceId r : liquids_)
          if (r != water) other += l.balance(*cs, r);
        const std::int64_t room = cap - l.balance(*cs, water);
        if (room > 0 && other == 0) l.transfer(t, *s, *cs, water, room, a.cause);
      }
      break;
    }
    case EffectOp::UseStock:
      if (auto r = resource_id(l, e.resource)) from_containers(*r, e.amount, used_);
      break;
    case EffectOp::Treat: {
      if (e.how == "clean_water") bodies_.clean(a.body, target, Cleaned::Water);
      else if (e.how == "disinfect") bodies_.clean(a.body, target, Cleaned::Disinfectant);
      else {
        // Whatever goes on the wound moves onto it: mass is never lost.
        for (const UseNeed& u : p.uses)
          if (auto item = carried_with(w, a, u.prop, u.min)) w.locations.place(*item, target);
        if (e.how == "dress") bodies_.dress(a.body, target, true);
        else if (e.how == "splint") bodies_.splint(a.body, target, true);
        else if (e.how == "tourniquet") bodies_.tourniquet(a.body, target, true);
      }
      break;
    }
    case EffectOp::Board: {
      const EdgeRef ref = edge_of_.at(target);
      EntityId bar = map_.edge(ref).barricade;
      if (bar == 0) {
        bar = items_.create(w, *items_.find_def("barricade"), 50, target);  // part of the opening, not loose
        map_.set_barricade(ref, bar);
      }
      const ItemDef& d = items_.def(items_.get(bar).def);
      std::vector<std::pair<int, double>> parts;
      for (std::size_t i = 0; i < d.slots.size(); ++i) {
        if (items_.get(bar).slots[i] != 0) continue;
        const Prop need = d.slots[i].accepts == "plank" ? Prop::FlatBoard : Prop::Fastener;
        if (auto item = fitting_part(w, a, d.slots[i].accepts, need)) {
          if (!items_.mount(w, bar, i, *item)) throw std::logic_error("barricade: checked part did not fit");
          if (need == Prop::FlatBoard) break;  // one board per action
        }
      }
      for (EntityId part : items_.get(bar).slots)
        if (part != 0 && items_.def(items_.get(part).def).family == "plank") parts.emplace_back(items_.get(part).quality, 1.0);
      QualityCaps caps;
      caps.skill = std::max(0, a.skill[static_cast<std::size_t>(Trade::Carpentry)]);
      caps.tool = a.action.tool ? items_.get(a.action.tool).quality : 0;
      items_.get(bar).quality = static_cast<int>(made_quality(parts, caps));
      break;
    }
    case EffectOp::Salvage: {
      const ItemDef& d = items_.def(items_.get(target).def);
      const auto slots = d.slots;
      for (std::size_t i = 0; i < slots.size(); ++i)
        items_.unmount(w, target, i, floor_at(a.pos), salvage_loss(slots[i].joint));
      break;
    }
    case EffectOp::Take:
      if (a.action.held != 0) {
        w.locations.place(a.action.held, a.body);
        a.action.held = 0;  // finished: it is carried now
      }
      break;
    case EffectOp::Sleep:
      break;
    case EffectOp::Power:
      if (power_apply) power_apply(w, a.body, target, e.how, a.cause);
      break;
    case EffectOp::Cover:
      for (const UseNeed& u : p.uses)
        if (auto item = carried_with(w, a, u.prop, u.min)) w.locations.place(*item, target);
      break;
  }
}

void Actors::step_move(World& w, Actor& a) {
  Action& act = a.action;
  if (act.door_wait_s > 0) {
    act.door_wait_s -= kTickSeconds;
    return;
  }
  if (act.path_next >= act.path.size()) return;
  const Cell next = act.path[act.path_next];
  const bool stairs = next.z != a.pos.z;
  if (!stairs) {
    const EdgeRef e = SiteMap::edge_between(a.pos, next);
    const Edge& ed = map_.edge(e);
    if (!map_.can_walk(e)) {  // something changed: re-plan once
      act.path = map_.find_path(a.pos, act.path.back());
      act.path_next = 1;
      if (act.path.empty()) interrupt(w, a, Reason::NoRoute);
      return;
    }
    if (!ed.open && !ed.breached && ed.kind != EdgeKind::None) {
      map_.set_open(e, true);  // closed, unlocked door: open it on the way, close it behind
      act.door_wait_s = 2.0;   // estimate
      act.close_behind = true;
      act.opened = e;
      return;
    }
  }
  const Body& b = bodies_.get(a.body);
  act.step_m += bodies_.walk_speed(b, w.clock.tick) * kTickSeconds;
  const double length = stairs ? 4.0 : 1.0;  // a flight of stairs ~4 m of walking (estimate)
  if (act.step_m >= length) {
    act.step_m -= length;
    a.pos = next;
    ++act.path_next;
    if (act.close_behind) {
      map_.set_open(act.opened, false);
      act.close_behind = false;
    }
  }
}

void Actors::set_body_activity(World& w, Actor& a) {
  Body& b = bodies_.get(a.body);
  if (!b.alive || b.activity.asleep) {
    a.noise_db = 0;
    return;
  }
  Activity act;
  act.carried_kg = a.carried_kg;
  a.noise_db = 0;
  if (a.action.phase == Phase::Approach && a.action.door_wait_s <= 0) {
    act.speed_mps = bodies_.walk_speed(b, w.clock.tick);
    if (a.action.path_next < a.action.path.size() && a.action.path[a.action.path_next].z != a.pos.z) act.grade_pct = 50;
    a.noise_db = bodies_.footstep_db(b, act.speed_mps, 0);
    b.posture = Posture::Stand;
  } else if (a.action.phase == Phase::Work && a.action.process >= 0) {
    const Primitive prim = processes_[static_cast<std::size_t>(a.action.process)].primitive;
    act.met = primitive_info(prim).met;
    a.noise_db = primitive_info(prim).noise_db;
    b.posture = prim == Primitive::Rest ? Posture::Sit : Posture::Stand;
  } else {
    act.met = 1.3;  // standing about
  }
  b.activity = act;
}

void Actors::update(World& w) {
  for (auto& [id, a] : actors_) {
    Body& b = bodies_.get(a.body);
    if (!b.alive) {
      if (a.action.phase != Phase::Idle) interrupt(w, a, Reason::Dead);
      a.queue.clear();
      continue;
    }
    if (w.clock.is_body_step()) recompute_carried(w, a);  // catches moves made outside the executor
    if (a.action.phase == Phase::Idle) start_next(w, a);
    Action& act = a.action;
    if (act.phase != Phase::Idle) {
      const ProcessDef* p = act.process >= 0 ? &processes_[static_cast<std::size_t>(act.process)] : nullptr;
      const bool sleeping = p && p->primitive == Primitive::Sleep;
      const Capabilities c = bodies_.capabilities(b, w.clock.tick);
      // Interruptions (spec 14 rule 5).
      if (!sleeping && b.activity.asleep) interrupt(w, a, Reason::Asleep);
      else if (!sleeping && c.consciousness < 0.3) interrupt(w, a, Reason::Unconscious);
      else if (act.phase == Phase::Approach && c.moving <= 0) interrupt(w, a, Reason::CannotMove);
      else if (p && act.phase == Phase::Work && !target_matches(w, *p, a, act.target) && act.held == 0)
        interrupt(w, a, Reason::TargetGone);
      else if (act.phase == Phase::Approach) {
        step_move(w, a);
        if (a.action.phase == Phase::Approach && a.action.path_next >= a.action.path.size()) {
          if (a.action.process < 0) a.action = Action{};  // a plain move ends on arrival
          else a.action.phase = Phase::Work;              // arriving is not working: work starts next tick
        }
      } else if (act.phase == Phase::Work && p) {
        if (p->primitive == Primitive::Sleep) {
          if (!b.activity.asleep) finish(w, a);
        } else {
          if (act.held == 0 && w.locations.parent_of(act.target) != a.body)
            for (const Effect& e : p->effects)
              if (e.op == EffectOp::Take) {  // extract into the hands: from here on it is held
                w.locations.place(act.target, a.body);
                act.held = act.target;
                recompute_carried(w, a);
              }
          const double rate = std::max(0.05, bodies_.work_rate(b, w.clock.tick)) * light_factor(w, a, *p);
          act.progress_s += kTickSeconds * rate;
          if (act.progress_s >= act.total_s) finish(w, a);
        }
      }
    }
    set_body_activity(w, a);
  }
}

// ---- Persistence ------------------------------------------------------------------------------------

namespace {
void save_cell(ArchiveWriter& w, const Cell& c) {
  w.i64(c.z);
  w.i64(c.x);
  w.i64(c.y);
}
Cell load_cell(ArchiveReader& r) {
  Cell c;
  c.z = static_cast<int>(r.i64());
  c.x = static_cast<int>(r.i64());
  c.y = static_cast<int>(r.i64());
  return c;
}
}  // namespace

void Actors::save(ArchiveWriter& w) const {
  w.u64(actors_.size());
  for (const auto& [id, a] : actors_) {
    w.u64(id);
    save_cell(w, a.pos);
    for (int s : a.skill) w.i64(s);
    const Action& act = a.action;
    w.u8(static_cast<std::uint8_t>(act.phase));
    w.i64(act.process);
    w.u64(act.target);
    save_cell(w, act.goal);
    w.u64(act.path.size());
    for (const Cell& c : act.path) save_cell(w, c);
    w.u64(act.path_next);
    w.f64(act.step_m);
    w.f64(act.door_wait_s);
    w.f64(act.progress_s);
    w.f64(act.total_s);
    w.u64(act.tool);
    w.u64(act.held);
    w.boolean(act.close_behind);
    w.i64(act.opened.z); w.i64(act.opened.x); w.i64(act.opened.y); w.boolean(act.opened.vertical);
    w.u64(a.queue.size());
    for (const auto& [c, cause] : a.queue) {
      w.i64(c.tick); w.u16(c.type); w.u64(c.subject); w.i64(c.a); w.i64(c.b); w.u64(cause);
    }
    w.u64(a.cause);
    w.f64(a.carried_kg);
    w.f64(a.noise_db);
  }
  w.u64(reserved_.size());
  for (const auto& [e, by] : reserved_) { w.u64(e); w.u64(by); }
  w.u64(resume_.size());
  for (const auto& [key, v] : resume_) { w.i64(key.first); w.u64(key.second); w.f64(v); }
}

void Actors::load(ArchiveReader& r) {
  if (r.u64() != actors_.size()) throw std::runtime_error("save: actor list differs");
  for (auto& [id, a] : actors_) {
    if (r.u64() != id) throw std::runtime_error("save: actor ids differ");
    a.pos = load_cell(r);
    for (int& s : a.skill) s = static_cast<int>(r.i64());
    Action act;
    act.phase = static_cast<Phase>(r.u8());
    act.process = static_cast<int>(r.i64());
    act.target = r.u64();
    act.goal = load_cell(r);
    act.path.resize(r.u64());
    for (Cell& c : act.path) c = load_cell(r);
    act.path_next = r.u64();
    act.step_m = r.f64();
    act.door_wait_s = r.f64();
    act.progress_s = r.f64();
    act.total_s = r.f64();
    act.tool = r.u64();
    act.held = r.u64();
    act.close_behind = r.boolean();
    act.opened.z = static_cast<int>(r.i64());
    act.opened.x = static_cast<int>(r.i64());
    act.opened.y = static_cast<int>(r.i64());
    act.opened.vertical = r.boolean();
    a.action = act;
    a.queue.clear();
    for (std::uint64_t n = r.u64(); n > 0; --n) {
      Command c;
      c.tick = r.i64(); c.type = r.u16(); c.subject = r.u64(); c.a = r.i64(); c.b = r.i64();
      a.queue.emplace_back(c, r.u64());
    }
    a.cause = r.u64();
    a.carried_kg = r.f64();
    a.noise_db = r.f64();
  }
  reserved_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    const EntityId e = r.u64();
    reserved_[e] = r.u64();
  }
  resume_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    const int p = static_cast<int>(r.i64());
    const EntityId t = r.u64();
    resume_[{p, t}] = r.f64();
  }
}

}  // namespace zs
