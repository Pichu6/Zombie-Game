// Spec 69: a town's police stations, their rooms and their interiors. docs/tier0/specs/69-the-police-station.md
#include "zs/gen/stations.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

#include "toml.hpp"
#include "zs/gen/keyed.h"
#include "zs/gen/names.h"

namespace zs::gen {
namespace {

enum Field : std::uint32_t { kStreet = 1, kNumber = 2, kName = 3 };

// The i-th of n shares of `total`: the first `total % n` take one more.
int share(int total, int n, int i) { return total / n + (i < total % n ? 1 : 0); }

double room_m2(const StationRules& r, const std::string& type) {
  const auto it = r.room_m2.find(type);
  if (it == r.room_m2.end()) throw std::runtime_error("stations.toml: no [room_m2] " + type);
  return it->second;
}

// Rule 8: the two-band plan houses use (spec 51 rule 4), from each room's target area: the lobby at the front on the
// left, the others to the lighter band; a room is never smaller than its door cell, its seats, its kit and a way between.
void lay_out(Dwelling& d, std::vector<Room> rooms, const std::vector<double>& target, const StationRules& rules,
             const DwellingRules& dr) {
  double total = 0;
  for (double t : target) total += t;
  const int width = std::clamp(static_cast<int>(std::lround(std::sqrt(total * 1.3))), rules.min_width, rules.max_width);
  const int left_w = (width - 1) / 2, right_w = width - 1 - left_w;
  std::vector<int> left, right;
  double load_left = 0, load_right = 0;
  for (std::size_t i = 0; i < rooms.size(); ++i) {
    const bool to_left = rooms[i].type == "lobby" || (rooms[i].type != "lobby" && load_left <= load_right);
    (to_left ? left : right).push_back(static_cast<int>(i));
    (to_left ? load_left : load_right) += target[i];
  }
  for (auto* band : {&left, &right})
    for (int i : *band) {
      Room& room = rooms[static_cast<std::size_t>(i)];
      room.w = band == &left ? left_w : right_w;
      room.x = band == &left ? 0 : left_w + 1;
      const auto kit = dr.kits.find(room.type);
      const int need = room.occupants + (kit == dr.kits.end() ? 0 : static_cast<int>(kit->second.size())) + 2;
      room.h = std::max({2, static_cast<int>(std::ceil(target[static_cast<std::size_t>(i)] / room.w)), (need + room.w - 1) / room.w});
    }
  auto band_depth = [&](const std::vector<int>& band) {
    int s = 0;
    for (int i : band) s += rooms[static_cast<std::size_t>(i)].h;
    return s;
  };
  const int depth = std::max(band_depth(left), band_depth(right));
  for (auto* band : {&left, &right}) {
    int y = 0;
    for (int i : *band) {
      rooms[static_cast<std::size_t>(i)].y = y;
      y += rooms[static_cast<std::size_t>(i)].h;
    }
    if (!band->empty()) rooms[static_cast<std::size_t>(band->back())].h += depth - y;  // the last room takes what is left
  }
  d.rooms = std::move(rooms);
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

}  // namespace

StationRules load_station_rules(const std::string& gen_dir) {
  const std::string path = gen_dir + "/stations.toml";
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  StationRules r;
  r.desk_m2 = t["desk_m2"].value_or(r.desk_m2);
  r.locker_m2 = t["locker_m2"].value_or(r.locker_m2);
  r.min_width = t["min_width"].value_or(r.min_width);
  r.max_width = t["max_width"].value_or(r.max_width);
  if (const auto* m = t["room_m2"].as_table())
    for (const auto& [k, v] : *m) r.room_m2[std::string(k.str())] = v.value<double>().value_or(0);
  for (const char* need : {"lobby", "office", "evidence", "cell", "washroom"}) (void)room_m2(r, need);
  if (r.min_width < 5 || r.max_width < r.min_width) throw std::runtime_error(path + ": widths out of range");
  return r;
}

std::vector<Station> plan_stations(const Catalog& cat, const Town& town, const AgencyPlan& agency, int shifts,
                                   const StationRules& rules, const DwellingRules& dwelling_rules, std::uint64_t seed) {
  const Keyed r(mix64(seed, hash_str("stations")));
  const int n = std::max(1, agency.stations);
  const BusinessType* police = cat.business("police");
  if (!police) throw std::runtime_error("businesses.toml: no police type");
  const Culture& culture = cat.culture(town.culture);
  std::vector<Station> out;
  for (int i = 0; i < n; ++i) {
    Station s;
    s.index = i;
    s.shifts = std::max(1, shifts);
    // Rule 6: the agency's staff split over its stations.
    s.sworn = share(agency.sworn, n, i);
    s.investigators = share(agency.investigators, n, i);
    s.patrol = s.sworn - s.investigators;
    s.civilians = share(agency.civilians, n, i);
    s.cells = std::max(1, share(agency.cells, n, i));
    // Rule 7: a name from the town's directory and an address on its streets. Houses are numbered from 101 (spec 51), so a
    // station's number below 100 never takes a house's.
    const auto id = static_cast<std::uint64_t>(i);
    s.street = town.streets.empty() ? std::string("Main Street") : town.streets[r.below(id, kStreet, town.streets.size())];
    s.number = r.range(id, kNumber, 1, 99);
    s.address = std::to_string(s.number) + " " + s.street;
    s.name = business_name(cat, *police, culture, culture, town.name, r, mix64(id, kName));
    if (n > 1) s.name += ", station " + std::to_string(i + 1);
    // Rule 8: the rooms from the staff.
    const int on_shift = std::max(1, (s.patrol + s.shifts - 1) / s.shifts);
    std::vector<Room> rooms;
    std::vector<double> target;
    auto add = [&](const std::string& type, double m2, int seats = 0, const std::string& seat = "bed") {
      Room room;
      room.type = type;
      room.occupants = seats;
      room.seat = seat;
      rooms.push_back(room);
      target.push_back(m2);
    };
    add("lobby", room_m2(rules, "lobby"));
    add("squad", std::max(9.0, rules.desk_m2 * on_shift), on_shift, "desk");
    add("office", room_m2(rules, "office"), 1, "desk");
    if (s.investigators > 0) add("investigators", rules.desk_m2 * s.investigators, s.investigators, "desk");
    if (s.civilians > 0) add("records", rules.desk_m2 * s.civilians, s.civilians, "desk");
    add("locker", std::max(6.0, rules.locker_m2 * (s.sworn + 2)), s.sworn, "locker");
    add("evidence", room_m2(rules, "evidence"));
    for (int w = 0; w < 1 + (s.sworn + s.civilians) / 30; ++w) add("washroom", room_m2(rules, "washroom"));
    for (int c = 0; c < s.cells; ++c) add("cell", room_m2(rules, "cell"), 1, "bed");
    s.plan.household = 1'000'000 + i;  // keys the furniture apart from every house
    s.plan.street = s.street;
    s.plan.number = s.number;
    s.plan.address = s.address;
    lay_out(s.plan, std::move(rooms), target, rules, dwelling_rules);
    out.push_back(std::move(s));
  }
  return out;
}

std::string station_toml(const Town& town, const Station& s, const DwellingRules& dwelling_rules, std::uint64_t seed) {
  std::ostringstream header;
  header << "# Generated by zs gen station (spec 69): " << s.name << ", " << s.address << ", " << town.name << "; " << s.sworn
         << " sworn (" << s.patrol << " patrol, " << s.investigators << " investigators), " << s.civilians << " civilians, " << s.cells
         << (s.cells == 1 ? " holding cell, " : " holding cells, ") << s.plan.floor_area() << " m2.";
  return building_toml(s.plan, dwelling_rules, seed, header.str(), "police_station", s.name);
}

}  // namespace zs::gen
