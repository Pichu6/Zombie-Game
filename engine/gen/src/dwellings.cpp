// Spec 51: the dwelling ledger, the house plan and its interior. docs/tier0/specs/51-dwellings.md
#include "zs/gen/dwellings.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/keyed.h"

namespace zs::gen {
namespace {

enum Field : std::uint32_t { kArea = 1, kFurniture = 2 };

double lookup(const std::map<std::string, double>& m, const std::string& key, double fallback) {
  const auto it = m.find(key);
  return it == m.end() ? fallback : it->second;
}

bool listed(const std::vector<std::string>& v, const std::string& s) { return std::find(v.begin(), v.end(), s) != v.end(); }

std::vector<std::string> strings(const toml::node_view<const toml::node>& n) {
  std::vector<std::string> out;
  if (const auto* a = n.as_array())
    for (const auto& e : *a) out.push_back(e.value<std::string>().value());
  return out;
}

std::map<std::string, double> numbers(const toml::node_view<const toml::node>& n) {
  std::map<std::string, double> out;
  if (const auto* t = n.as_table())
    for (const auto& [k, v] : *t) out[std::string(k.str())] = v.value<double>().value();
  return out;
}

// The fewest cells a room of this type needs: its door cell, its furniture and a way between them.
int min_cells(const Room& r) {
  if (r.type == "main") return 10;
  if (r.type == "kitchen") return 7;
  if (r.type == "bedroom") return r.occupants + 2;
  return 4;
}

}  // namespace

DwellingRules load_dwelling_rules(const std::string& gen_dir) {
  DwellingRules r;
  const toml::table d = toml::parse_file(gen_dir + "/dwellings.toml");
  const toml::node_view<const toml::node> dv{d};
  r.min_area_m2 = dv["min_area_m2"].value_or(r.min_area_m2);
  r.spread = dv["spread"].value_or(r.spread);
  r.main_room_below_m2 = dv["main_room_below_m2"].value_or(r.main_room_below_m2);
  r.max_bedrooms = dv["max_bedrooms"].value_or(r.max_bedrooms);
  r.food_kj_per_person_day = dv["food_kj_per_person_day"].value_or(r.food_kj_per_person_day);
  r.fridge_days = dv["fridge_days"].value_or(r.fridge_days);
  r.no_fridge = strings(dv["no_fridge"]);
  r.no_bathroom = strings(dv["no_bathroom"]);
  r.area_per_person_m2 = numbers(dv["area_per_person_m2"]);
  r.pantry_days = numbers(dv["pantry_days"]);
  r.room_weight = numbers(dv["room_weight"]);
  r.lot_front = dv["lot"]["front"].value_or(r.lot_front);
  r.lot_side = dv["lot"]["side"].value_or(r.lot_side);
  r.lot_back = dv["lot"]["back"].value_or(r.lot_back);
  const toml::table i = toml::parse_file(gen_dir + "/interiors.toml");
  const toml::node_view<const toml::node> iv{i};
  r.blocking = strings(iv["blocking"]);
  if (const auto* rooms = iv["room"].as_table())
    for (const auto& [k, v] : *rooms) r.kits[std::string(k.str())] = strings(toml::node_view<const toml::node>{v}["kit"]);
  return r;
}

std::string income_group(const Mix& mix) {
  if (mix.nation && !mix.nation->income.empty()) return mix.nation->income;
  if (mix.country.rfind("base_", 0) == 0) return mix.country.substr(5);
  return "upper_middle";
}

int Dwelling::beds() const {
  int n = 0;
  for (const Room& r : rooms) n += r.occupants;
  return n;
}

Dwelling plan_house(int index, int members, bool couple, const DwellingRules& rules, const std::string& income,
                    std::uint64_t seed, int max_width) {
  const Keyed r(mix64(seed, hash_str("dwellings")));
  Dwelling d;
  d.household = index;
  d.income = income;
  const int n = std::max(1, members);
  const auto id = static_cast<std::uint64_t>(index);
  // Rule 2: floor area from the members and the income group.
  const double area = std::max(rules.min_area_m2, n * lookup(rules.area_per_person_m2, income, 30) * (1 + rules.spread * r.bell(id, kArea)));
  d.target_area_m2 = area;
  d.fridge = !listed(rules.no_fridge, income);
  d.bathroom = !listed(rules.no_bathroom, income);
  // Rule 3: the rooms. The head and a partner share a bedroom; the others two to a room.
  const int others = std::max(0, n - 1 - (couple ? 1 : 0));
  const int bedrooms = std::min(rules.max_bedrooms, 1 + (others + 1) / 2);
  std::vector<int> occupants(static_cast<std::size_t>(bedrooms), 0);
  occupants[0] = 1 + (couple ? 1 : 0);
  for (int i = 0; i < others; ++i) occupants[static_cast<std::size_t>(std::min(bedrooms - 1, bedrooms == 1 ? 0 : 1 + i / 2))]++;
  std::vector<Room> rooms;
  if (area < rules.main_room_below_m2) {
    rooms.push_back({"main"});
  } else {
    rooms.push_back({"living"});
    rooms.push_back({"kitchen"});
  }
  if (d.bathroom) rooms.push_back({"bath"});
  for (int b = 0; b < bedrooms; ++b) rooms.push_back({"bedroom", 0, 0, 0, 0, occupants[static_cast<std::size_t>(b)]});
  // Rule 4: the footprint, a corridor down its middle and two bands of rooms.
  const int width = std::clamp(static_cast<int>(std::lround(std::sqrt(area * 1.3))), 5, std::max(5, max_width));
  double weights = 0;
  for (const Room& room : rooms) weights += lookup(rules.room_weight, room.type, 1);
  const double net = area * (width - 1) / width;
  // Bands first: the living or main room on the left, kitchen and bathroom on the right, bedrooms to the lighter band.
  std::vector<int> left, right;
  std::vector<double> target(rooms.size());
  double load_left = 0, load_right = 0;  // m2 wanted in each band
  for (std::size_t i = 0; i < rooms.size(); ++i) {
    target[i] = net * lookup(rules.room_weight, rooms[i].type, 1) / weights;
    const bool to_left = rooms[i].type == "main" || rooms[i].type == "living" ? true
                         : rooms[i].type == "kitchen" || rooms[i].type == "bath" ? false
                                                                                 : load_left <= load_right;
    (to_left ? left : right).push_back(static_cast<int>(i));
    (to_left ? load_left : load_right) += target[i];
  }
  if (right.empty()) {  // keep both bands in use: the last room crosses over
    load_left -= target[static_cast<std::size_t>(left.back())];
    load_right += target[static_cast<std::size_t>(left.back())];
    right.push_back(left.back());
    left.pop_back();
  }
  // Each band is as wide as its share of the rooms, at least 2 cells.
  const int left_w = std::clamp(static_cast<int>(std::lround((width - 1) * load_left / (load_left + load_right))), 2, width - 3);
  const int right_w = width - 1 - left_w;
  for (auto* band : {&left, &right})
    for (int i : *band) {
      Room& room = rooms[static_cast<std::size_t>(i)];
      room.w = band == &left ? left_w : right_w;
      room.x = band == &left ? 0 : left_w + 1;
      room.h = std::max({2, static_cast<int>(std::lround(target[static_cast<std::size_t>(i)] / room.w)), (min_cells(room) + room.w - 1) / room.w});
    }
  auto band_depth = [&](const std::vector<int>& band) {
    int s = 0;
    for (int i : band) s += rooms[static_cast<std::size_t>(i)].h;
    return s;
  };
  const int depth = std::max({4, static_cast<int>(std::ceil(area / width)), band_depth(left), band_depth(right)});
  for (auto* band : {&left, &right}) {
    int y = 0;
    for (int i : *band) {
      rooms[static_cast<std::size_t>(i)].y = y;
      y += rooms[static_cast<std::size_t>(i)].h;
    }
    rooms[static_cast<std::size_t>(band->back())].h += depth - y;  // the last room takes what is left
  }
  d.rooms = std::move(rooms);
  d.width = width;
  d.depth = depth;
  d.corridor_x = left_w;
  // Rule 5: the lot.
  d.house_x = rules.lot_side;
  d.house_y = rules.lot_front;
  d.lot_width = width + 2 * rules.lot_side;
  d.lot_depth = rules.lot_front + depth + rules.lot_back;
  // Rule 7: the food follows the household.
  const double day = n * rules.food_kj_per_person_day;
  d.pantry_kj = static_cast<std::int64_t>(std::llround(day * lookup(rules.pantry_days, income, 7)));
  d.fridge_kj = d.fridge ? static_cast<std::int64_t>(std::llround(day * rules.fridge_days)) : 0;
  return d;
}

std::vector<Dwelling> plan_dwellings(const Town& town, const DwellingRules& rules, const std::string& income, std::uint64_t seed) {
  std::vector<Dwelling> out;
  std::map<std::string, int> on_street;
  for (const Household& hh : town.households) {
    bool couple = false;
    for (std::size_t m : hh.members) couple = couple || town.pops[m].role == "partner";
    Dwelling d = plan_house(hh.index, static_cast<int>(hh.members.size()), couple, rules, income, seed);
    // Rule 1: numbered along the town's streets, unique per street.
    d.street = town.streets.empty() ? std::string("Main Street") : town.streets[static_cast<std::size_t>(hh.index) % town.streets.size()];
    d.number = 101 + on_street[d.street]++;
    d.address = std::to_string(d.number) + " " + d.street;
    out.push_back(std::move(d));
  }
  return out;
}

void assign_addresses(Town& town, const std::vector<Dwelling>& dwellings) {
  for (const Dwelling& d : dwellings) town.households[static_cast<std::size_t>(d.household)].address = d.address;
}

namespace {

struct Placed {
  std::string kind, id;
  int x = 0, y = 0;  // footprint cells
  std::vector<std::pair<std::string, std::int64_t>> amounts;
};

// The cell inside a room, beside its door onto the corridor.
std::pair<int, int> door_cell(const Dwelling& d, const Room& r) {
  return {r.x < d.corridor_x ? d.corridor_x - 1 : d.corridor_x + 1, r.y + r.h / 2};
}

// Rule 6: furniture along the walls, never on the door cell, never cutting a free cell off from the door.
std::vector<Placed> furnish(const Dwelling& d, const DwellingRules& rules, const Keyed& r) {
  std::vector<Placed> out;
  std::vector<char> used(static_cast<std::size_t>(d.width * d.depth), 0), blocks(used.size(), 0);
  auto at = [&](int x, int y) { return static_cast<std::size_t>(y * d.width + x); };
  bool sink_filled = false;
  for (std::size_t ri = 0; ri < d.rooms.size(); ++ri) {
    const Room& room = d.rooms[ri];
    const auto [dx, dy] = door_cell(d, room);
    used[at(dx, dy)] = 1;  // kept free
    std::vector<std::pair<int, int>> walls, inner;
    for (int y = room.y; y < room.y + room.h; ++y)
      for (int x = room.x; x < room.x + room.w; ++x) {
        if (x == dx && y == dy) continue;
        const bool edge = x == room.x || y == room.y || x == room.x + room.w - 1 || y == room.y + room.h - 1;
        (edge ? walls : inner).emplace_back(x, y);
      }
    auto connected = [&]() {
      std::vector<std::pair<int, int>> open{{dx, dy}};
      std::vector<char> seen(used.size(), 0);
      seen[at(dx, dy)] = 1;
      int free_cells = 0;
      for (int y = room.y; y < room.y + room.h; ++y)
        for (int x = room.x; x < room.x + room.w; ++x) free_cells += blocks[at(x, y)] ? 0 : 1;
      for (std::size_t i = 0; i < open.size(); ++i) {
        const auto [cx, cy] = open[i];
        for (const auto& [nx, ny] : {std::pair{cx + 1, cy}, std::pair{cx - 1, cy}, std::pair{cx, cy + 1}, std::pair{cx, cy - 1}}) {
          if (nx < room.x || ny < room.y || nx >= room.x + room.w || ny >= room.y + room.h) continue;
          if (seen[at(nx, ny)] || blocks[at(nx, ny)]) continue;
          seen[at(nx, ny)] = 1;
          open.emplace_back(nx, ny);
        }
      }
      return static_cast<int>(open.size()) == free_cells;
    };
    const std::uint64_t id = mix64(static_cast<std::uint64_t>(d.household), ri);
    std::uint32_t k = 0;
    auto place = [&](const std::string& kind, bool blocking, std::vector<std::pair<std::string, std::int64_t>> amounts) {
      std::vector<std::pair<int, int>> cells = walls;
      if (!blocking) cells.insert(cells.end(), inner.begin(), inner.end());
      if (cells.empty()) return false;
      const std::size_t start = r.below(id, kFurniture, cells.size(), k++);
      for (std::size_t j = 0; j < cells.size(); ++j) {
        const auto [x, y] = cells[(start + j) % cells.size()];
        if (used[at(x, y)]) continue;
        if (blocking) {
          blocks[at(x, y)] = 1;
          if (!connected()) {
            blocks[at(x, y)] = 0;
            continue;
          }
        }
        used[at(x, y)] = 1;
        out.push_back({kind, "", x, y, std::move(amounts)});
        return true;
      }
      return false;
    };
    for (int b = 0; b < room.occupants; ++b) place(room.seat, room.seat != "bed" && listed(rules.blocking, room.seat), {});
    const auto kit = rules.kits.find(room.type);
    if (kit == rules.kits.end()) continue;
    for (const std::string& kind : kit->second) {
      if (kind == "fridge" && !d.fridge) continue;
      std::vector<std::pair<std::string, std::int64_t>> amounts;
      if (kind == "pantry") amounts = {{"food_kj", d.pantry_kj}};
      if (kind == "fridge") amounts = {{"food_kj", d.fridge_kj}};
      const bool kitchen_sink = kind == "sink" && (room.type == "kitchen" || room.type == "main") && !sink_filled;
      if (kitchen_sink) amounts = {{"tap_water_l", 10000}};
      if (place(kind, listed(rules.blocking, kind), std::move(amounts)) && kitchen_sink) sink_filled = true;
    }
  }
  return out;
}

}  // namespace

std::string house_toml(const Town& town, const Dwelling& d, const DwellingRules& rules, std::uint64_t seed) {
  const Household& hh = town.households[static_cast<std::size_t>(d.household)];
  std::ostringstream header;
  header << "# Generated by zs gen house (spec 51): household " << d.household + 1 << " of " << town.name << ", " << d.address
         << ", " << hh.members.size() << (hh.members.size() == 1 ? " person, " : " people, ") << d.income << " income, " << d.floor_area() << " m2.";
  return building_toml(d, rules, seed, header.str(), "home", hh.home);  // rule 7: the household's census home
}

namespace {

// One floor of a building as a site-map grid at double resolution (spec 09). Floor 0 stands on its lot (the street in
// front, a path to the door, fences around); an upper floor (spec 70) is the footprint alone, with no doors out.
std::string floor_grid(const Dwelling& d, int level) {
  const int lw = d.lot_width, ld = d.lot_depth, hx = d.house_x, hy = d.house_y;
  const bool ground = level == 0;
  std::vector<char> floor(static_cast<std::size_t>(lw * ld), ground ? '"' : ' ');
  if (ground) {
    for (int x = 0; x < lw; ++x) floor[static_cast<std::size_t>(x)] = '_';  // the street
    for (int y = 1; y < hy; ++y) floor[static_cast<std::size_t>(y * lw + hx + d.corridor_x)] = ':';  // the path
  }
  std::vector<int> room_of(static_cast<std::size_t>(d.width * d.depth), -1);  // -1: the corridor
  for (std::size_t i = 0; i < d.rooms.size(); ++i) {
    const Room& rm = d.rooms[i];
    if (rm.floor != level) continue;
    for (int y = rm.y; y < rm.y + rm.h; ++y)
      for (int x = rm.x; x < rm.x + rm.w; ++x) room_of[static_cast<std::size_t>(y * d.width + x)] = static_cast<int>(i);
  }
  for (int y = 0; y < d.depth; ++y)
    for (int x = 0; x < d.width; ++x) {
      const int i = room_of[static_cast<std::size_t>(y * d.width + x)];
      const bool tiled = i >= 0 && (d.rooms[static_cast<std::size_t>(i)].type == "kitchen" || d.rooms[static_cast<std::size_t>(i)].type == "bath");
      floor[static_cast<std::size_t>((hy + y) * lw + hx + x)] = tiled ? ',' : '.';
    }
  // Edges: top of (x, y) and left of (x, y).
  std::vector<char> top(static_cast<std::size_t>((ld + 1) * lw), ' '), left(static_cast<std::size_t>(ld * (lw + 1)), ' ');
  auto set_top = [&](int x, int y, char c) { top[static_cast<std::size_t>(y * lw + x)] = c; };
  auto set_left = [&](int x, int y, char c) { left[static_cast<std::size_t>(y * (lw + 1) + x)] = c; };
  if (ground) {
    for (int y = 1; y < ld; ++y) {  // fences on the sides and back
      set_left(0, y, '=');
      set_left(lw, y, '=');
    }
    for (int x = 0; x < lw; ++x) set_top(x, ld, '=');
  }
  for (int y = 0; y < d.depth; ++y)  // inside walls between different rooms (or a room and the corridor)
    for (int x = 0; x < d.width; ++x) {
      const int here = room_of[static_cast<std::size_t>(y * d.width + x)];
      if (x + 1 < d.width && room_of[static_cast<std::size_t>(y * d.width + x + 1)] != here) set_left(hx + x + 1, hy + y, '|');
      if (y + 1 < d.depth && room_of[static_cast<std::size_t>((y + 1) * d.width + x)] != here) set_top(hx + x, hy + y + 1, '-');
    }
  for (int x = 0; x < d.width; ++x) {  // the outside walls
    set_top(hx + x, hy, '#');
    set_top(hx + x, hy + d.depth, '#');
  }
  for (int y = 0; y < d.depth; ++y) {
    set_left(hx, hy + y, '#');
    set_left(hx + d.width, hy + y, '#');
  }
  if (ground) {
    set_top(hx + d.corridor_x, hy, 'E');            // the front door, on the street side
    set_top(hx + d.corridor_x, hy + d.depth, 'E');  // the back door
  }
  for (const Room& rm : d.rooms) {
    if (rm.floor != level) continue;
    const bool on_left = rm.x < d.corridor_x;
    const int mid_y = rm.y + rm.h / 2, mid_x = rm.x + rm.w / 2;
    set_left(hx + (on_left ? d.corridor_x : d.corridor_x + 1), hy + mid_y, 'D');  // every room opens on the corridor
    set_left(hx + (on_left ? 0 : d.width), hy + mid_y, 'W');                       // a window on its side wall
    if (rm.y == 0) set_top(hx + mid_x, hy, 'W');
    if (rm.y + rm.h == d.depth) set_top(hx + mid_x, hy + d.depth, 'W');
  }
  std::ostringstream grid;
  for (int gy = 0; gy <= 2 * ld; ++gy) {
    for (int gx = 0; gx <= 2 * lw; ++gx) {
      const int x = gx / 2, y = gy / 2;
      if (gy % 2 == 0) grid << (gx % 2 == 0 ? '+' : top[static_cast<std::size_t>(y * lw + (gx - 1) / 2)]);
      else grid << (gx % 2 == 0 ? left[static_cast<std::size_t>(y * (lw + 1) + x)] : floor[static_cast<std::size_t>(y * lw + (gx - 1) / 2)]);
    }
    grid << "\n";
  }
  return grid.str();
}

}  // namespace

// Spec 69 rule 8, shared by spec 72: the two-band plan houses use (spec 51 rule 4), from each room's target area: the
// `front` room at the front on the left, the others to the shallower band; a room is never smaller than its door cell,
// its seats, its kit and a way between. Spec 70: with a depth limit (a site's frame), a room that fits neither band
// starts the next floor up.
void lay_out_rooms(Dwelling& d, std::vector<Room> rooms, const std::vector<double>& target, int min_width, int max_width,
                   const DwellingRules& dr, int max_depth, const std::string& front) {
  double total = 0;
  for (double t : target) total += t;
  const int width = std::clamp(static_cast<int>(std::lround(std::sqrt(total * 1.3))), min_width, max_width);
  const int left_w = (width - 1) / 2, right_w = width - 1 - left_w;
  const auto height = [&](std::size_t i, int w) {
    const auto kit = dr.kits.find(rooms[i].type);
    const int need = rooms[i].occupants + (kit == dr.kits.end() ? 0 : static_cast<int>(kit->second.size())) + 2;
    const int h = std::max({2, static_cast<int>(std::ceil(target[i] / w)), (need + w - 1) / w});
    return max_depth > 0 ? std::min(h, max_depth) : h;
  };
  std::vector<std::vector<int>> left(1), right(1);  // per floor, the rooms of each band in order
  std::vector<int> depth_left(1, 0), depth_right(1, 0);
  for (std::size_t i = 0; i < rooms.size(); ++i) {
    int f = static_cast<int>(left.size()) - 1;
    const int hl = height(i, left_w), hr = height(i, right_w);
    const bool fits_left = max_depth <= 0 || depth_left[f] + hl <= max_depth;
    const bool fits_right = max_depth <= 0 || depth_right[f] + hr <= max_depth;
    bool to_left = rooms[i].type == front || depth_left[f] <= depth_right[f];
    if (to_left && !fits_left && fits_right) to_left = false;
    if (!to_left && !fits_right && fits_left) to_left = true;
    if (!fits_left && !fits_right) {  // the next floor up
      left.emplace_back();
      right.emplace_back();
      depth_left.push_back(0);
      depth_right.push_back(0);
      ++f;
      to_left = true;
    }
    Room& room = rooms[i];
    room.floor = f;
    room.w = to_left ? left_w : right_w;
    room.x = to_left ? 0 : left_w + 1;
    room.h = to_left ? hl : hr;
    (to_left ? left : right)[static_cast<std::size_t>(f)].push_back(static_cast<int>(i));
    (to_left ? depth_left : depth_right)[static_cast<std::size_t>(f)] += room.h;
  }
  int depth = 0;
  for (std::size_t f = 0; f < left.size(); ++f) depth = std::max({depth, depth_left[f], depth_right[f]});
  for (auto* bands : {&left, &right})
    for (const std::vector<int>& band : *bands) {
      int y = 0;
      for (int i : band) {
        rooms[static_cast<std::size_t>(i)].y = y;
        y += rooms[static_cast<std::size_t>(i)].h;
      }
      if (!band.empty()) rooms[static_cast<std::size_t>(band.back())].h += depth - y;  // the last room takes what is left
    }
  d.rooms = std::move(rooms);
  d.floors = static_cast<int>(left.size());
  d.width = width;
  d.depth = depth;
  d.corridor_x = left_w;
  d.target_area_m2 = total;
  d.house_x = dr.lot_side;
  d.house_y = dr.lot_front;
  d.lot_width = width + 2 * dr.lot_side;
  d.lot_depth = dr.lot_front + depth + dr.lot_back;
  d.fridge = false;
}

std::string building_toml(const Dwelling& d, const DwellingRules& rules, std::uint64_t seed, const std::string& header,
                          const std::string& marker_kind, const std::string& marker_id) {
  const Keyed r(mix64(seed, hash_str("interior")));
  const int floors = std::max(1, d.floors);
  // Furniture floor by floor: each floor's rooms alone, keyed apart from the ground floor's (spec 70).
  std::vector<std::pair<int, Placed>> objects;
  for (int f = 0; f < floors; ++f) {
    Dwelling on = d;
    on.rooms.clear();
    for (const Room& rm : d.rooms)
      if (rm.floor == f) on.rooms.push_back(rm);
    if (f > 0) on.household = d.household + 100'000 * f;
    for (Placed& p : furnish(on, rules, r)) objects.emplace_back(f, std::move(p));
  }
  if (!marker_kind.empty()) objects.emplace_back(0, Placed{marker_kind, marker_id, d.corridor_x, 0, {}});
  std::ostringstream out;
  out << header << "\n\n";
  out << "blocking = [";
  for (std::size_t i = 0; i < rules.blocking.size(); ++i) out << (i ? ", " : "") << '"' << rules.blocking[i] << '"';
  out << "]\n\n[map]\nname = \"" << d.address << "\"\nwidth = " << d.lot_width << "\nheight = " << d.lot_depth << "\nbuild_quality = 50\n\n";
  for (int f = 0; f < floors; ++f) out << (f ? "\n" : "") << "[[level]]\nz = " << f << "\ngrid = '''\n" << floor_grid(d, f) << "'''\n";
  for (int f = 0; f + 1 < floors; ++f)  // spec 70: stairs at the back of the corridor, floor to floor
    out << "\n[[stairs]]\na = [" << f << ", " << d.house_x + d.corridor_x << ", " << d.house_y + d.depth - 1 << "]\nb = [" << f + 1 << ", "
        << d.house_x + d.corridor_x << ", " << d.house_y + d.depth - 1 << "]\n";
  for (const auto& [f, o] : objects) {
    out << "\n[[object]]\nkind = \"" << o.kind << "\"\nat = [" << f << ", " << d.house_x + o.x << ", " << d.house_y + o.y << "]\n";
    if (!o.id.empty()) out << "id = \"" << o.id << "\"\n";
    for (const auto& [key, v] : o.amounts) out << key << " = " << v << "\n";
  }
  return out.str();
}

}  // namespace zs::gen
