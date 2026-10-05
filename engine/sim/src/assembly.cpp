#include "zs/assembly.h"

#include <algorithm>
#include <stdexcept>

namespace zs {

double made_quality(std::span<const std::pair<int, double>> parts, const QualityCaps& caps) {
  double sum = 0, weights = 0;
  for (const auto& [q, w] : parts) { sum += q * w; weights += w; }
  const double mean = weights > 0 ? sum / weights : 0.0;
  return std::min({mean, caps.skill, caps.tool, caps.method_ceiling, caps.station});
}

DefId Assemblies::add_def(ItemDef def) {
  if (find_def(def.id)) throw std::invalid_argument("duplicate item def " + def.id);
  defs_.push_back(std::move(def));
  return static_cast<DefId>(defs_.size() - 1);
}

std::optional<DefId> Assemblies::find_def(const std::string& id) const {
  for (std::size_t i = 0; i < defs_.size(); ++i)
    if (defs_[i].id == id) return static_cast<DefId>(i);
  return std::nullopt;
}

EntityId Assemblies::create(World& w, DefId def_id, int quality, EntityId parent) {
  const EntityId id = w.new_entity();
  Instance inst;
  inst.id = id;
  inst.def = def_id;
  inst.quality = std::clamp(quality, 0, 100);
  inst.slots.assign(defs_.at(def_id).slots.size(), 0);
  instances_[id] = inst;
  w.locations.place(id, parent);
  const auto slots = defs_.at(def_id).slots;  // copy: create() may grow defs_ users' references
  for (std::size_t i = 0; i < slots.size(); ++i) {
    if (slots[i].default_part.empty()) continue;
    const auto part_def = find_def(slots[i].default_part);
    if (!part_def) throw std::runtime_error("unknown default part " + slots[i].default_part);
    instances_.at(id).slots[i] = create(w, *part_def, quality, id);
  }
  return id;
}

bool Assemblies::mount(World& w, EntityId parent, std::size_t slot, EntityId part) {
  Instance& p = instances_.at(parent);
  const SlotDef& s = defs_.at(p.def).slots.at(slot);
  if (p.slots[slot] != 0) return false;
  if (defs_.at(instances_.at(part).def).family != s.accepts) return false;
  if (instances_.contains(w.locations.parent_of(part))) return false;  // already mounted somewhere
  if (!w.locations.place(part, parent)) return false;
  p.slots[slot] = part;
  return true;
}

EntityId Assemblies::unmount(World& w, EntityId parent, std::size_t slot, EntityId destination,
                             std::int64_t condition_loss) {
  Instance& p = instances_.at(parent);
  const EntityId part = p.slots.at(slot);
  if (part == 0) return 0;
  p.slots[slot] = 0;
  instances_.at(part).condition.damage(condition_loss);
  w.locations.place(part, destination);
  return part;
}

std::int64_t Assemblies::mass_g(EntityId id) const {
  const Instance& inst = instances_.at(id);
  std::int64_t m = defs_.at(inst.def).mass_g;
  for (EntityId part : inst.slots)
    if (part != 0) m += mass_g(part);
  return m;
}

bool Assemblies::works(EntityId id) const {
  const Instance& inst = instances_.at(id);
  const ItemDef& d = defs_.at(inst.def);
  if (inst.condition.failed(d.failure_threshold)) return false;
  for (std::size_t i = 0; i < d.slots.size(); ++i) {
    if (!d.slots[i].required) continue;
    if (inst.slots[i] == 0 || !works(inst.slots[i])) return false;
  }
  return true;
}

std::optional<std::size_t> Assemblies::slot_index(EntityId id, const std::string& slot_name) const {
  const auto& slots = defs_.at(instances_.at(id).def).slots;
  for (std::size_t i = 0; i < slots.size(); ++i)
    if (slots[i].name == slot_name) return i;
  return std::nullopt;
}

void Assemblies::save(ArchiveWriter& w) const {
  w.u64(defs_.size());  // definitions come from data; the count guards against a mismatch
  w.u64(instances_.size());
  for (const auto& [id, inst] : instances_) {
    w.u64(id);
    w.u16(inst.def);
    w.u8(static_cast<std::uint8_t>(inst.quality));
    w.i64(inst.condition.value);
    w.i64(inst.condition.max);
    w.u64(inst.slots.size());
    for (EntityId s : inst.slots) w.u64(s);
  }
}

void Assemblies::load(ArchiveReader& r) {
  if (r.u64() != defs_.size()) throw std::runtime_error("save: item definitions differ");
  instances_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    Instance inst;
    inst.id = r.u64();
    inst.def = r.u16();
    inst.quality = r.u8();
    inst.condition.value = r.i64();
    inst.condition.max = r.i64();
    inst.slots.resize(r.u64());
    for (EntityId& s : inst.slots) s = r.u64();
    instances_[inst.id] = std::move(inst);
  }
}

namespace {
constexpr const char* kPropNames[kPropCount] = {"edge",          "blunt",     "lever",      "grip",     "container_ml",
                                                "cloth_area_m2", "absorbent", "flat_board", "fastener", "rigid"};
}  // namespace

const char* to_string(Prop p) { return kPropNames[static_cast<std::size_t>(p)]; }

std::optional<Prop> prop_from(const std::string& name) {
  for (std::size_t i = 0; i < kPropCount; ++i)
    if (name == kPropNames[i]) return static_cast<Prop>(i);
  return std::nullopt;
}

const MaterialDef* Assemblies::find_material(const std::string& id) const {
  for (const MaterialDef& m : materials_)
    if (m.id == id) return &m;
  return nullptr;
}

double Assemblies::prop(EntityId id, Prop p) const {
  const Instance& inst = instances_.at(id);
  const ItemDef& d = defs_.at(inst.def);
  if (inst.condition.failed(d.failure_threshold)) return 0;
  const double base = d.props[static_cast<std::size_t>(p)];
  if (p == Prop::ContainerMl || p == Prop::ClothAreaM2) return base;  // sizes do not scale with Q
  const double condition = static_cast<double>(inst.condition.value) / static_cast<double>(kConditionFull);
  return base * (0.5 + inst.quality / 100.0) * (0.5 + 0.5 * condition);
}

}  // namespace zs
