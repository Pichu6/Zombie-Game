// Spec 51: every household's dwelling (the ledger made at town creation), its house plan and its interior, written
// as a site map (spec 09). The record is the truth; the plan and the furniture are a pure function of it and a seed.
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "zs/gen/pops.h"

namespace zs::gen {

struct DwellingRules {
  double min_area_m2 = 24, spread = 0.15, main_room_below_m2 = 36;
  int max_bedrooms = 4;
  double food_kj_per_person_day = 10000, fridge_days = 2;
  std::vector<std::string> no_fridge, no_bathroom;
  std::map<std::string, double> area_per_person_m2, pantry_days, room_weight;
  int lot_front = 3, lot_side = 2, lot_back = 5;
  // Interiors.
  std::vector<std::string> blocking;
  std::map<std::string, std::vector<std::string>> kits;  // room type -> furniture in placement order
};
// Reads data/gen/dwellings.toml and data/gen/interiors.toml from `gen_dir`.
DwellingRules load_dwelling_rules(const std::string& gen_dir);

// The town's income group: the country's (spec 49), or the base profile's suffix (base_low -> low); upper_middle
// without a country.
std::string income_group(const Mix& mix);

// A room: a rectangle of the footprint (cells), its type and how many sleep in it.
struct Room {
  std::string type;  // main, living, kitchen, bath, bedroom; spec 69 adds a station's rooms
  int x = 0, y = 0, w = 0, h = 0;
  int occupants = 0;
  std::string seat = "bed";  // what each occupant gets (spec 69: a desk, a locker; a bed never blocks)
  int floor = 0;             // spec 70: the floor it is on (0: the ground floor)
};

struct Dwelling {
  int household = 0;
  std::string street, address;
  int number = 0;
  std::string income;
  double target_area_m2 = 0;
  int width = 0, depth = 0;  // the footprint in 1 m cells; width faces the street
  int floors = 1;            // spec 70: floors of the same footprint, joined by stairs at the back of the corridor
  int corridor_x = 0;        // the corridor's column inside the footprint
  int lot_width = 0, lot_depth = 0, house_x = 0, house_y = 0;  // the lot, and the footprint's origin in it
  std::vector<Room> rooms;
  bool fridge = true, bathroom = true;
  std::int64_t pantry_kj = 0, fridge_kj = 0;
  int floor_area() const { return width * depth * (floors > 1 ? floors : 1); }
  int beds() const;
};

// One household's house (rules 2-5, 7): its members, whether the head lives with a partner, the income group; spec 72: no
// wider than `max_width` (a plot's frontage less its sides). Its street and address are left empty.
Dwelling plan_house(int index, int members, bool couple, const DwellingRules& rules, const std::string& income,
                    std::uint64_t seed, int max_width = 14);
// One dwelling per household, in household order (rule 1). Its own keyed draws: the town does not change.
std::vector<Dwelling> plan_dwellings(const Town& town, const DwellingRules& rules, const std::string& income, std::uint64_t seed);
// Writes each dwelling's address into its household (rule 1).
void assign_addresses(Town& town, const std::vector<Dwelling>& dwellings);
// The house on its lot with its interior, as a site-map TOML (rules 4-8).
std::string house_toml(const Town& town, const Dwelling& d, const DwellingRules& rules, std::uint64_t seed);
// Spec 72 (spec 69's station planner, shared): rooms with target areas laid out in two bands either side of a corridor,
// the width from the total area within [min_width, max_width], the first room of type `front` at the front on the left;
// with max_depth > 0 a room that fits neither band starts the next floor up. Sets the plan's footprint and a lot around it.
void lay_out_rooms(Dwelling& d, std::vector<Room> rooms, const std::vector<double>& target, int min_width, int max_width,
                   const DwellingRules& rules, int max_depth, const std::string& front = "lobby");
// Any building planned as a Dwelling (a corridor down the middle, rooms in two bands, a lot around it) as a site-map TOML:
// floors, walls, doors, windows and the furniture of each room; `header` is its comment line, and `marker` an object put
// at the front door (a house's census home; spec 69: the station). `key` keys the furniture.
std::string building_toml(const Dwelling& d, const DwellingRules& rules, std::uint64_t seed, const std::string& header,
                          const std::string& marker_kind = "", const std::string& marker_id = "");

}  // namespace zs::gen
