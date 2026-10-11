// The cards (atlas design §5, spec 55 slice 3): a country, a town, a pop, a house and the world, each shown as what
// it has against what the design says it needs (data/atlas/cards.toml). Read-only views over the generators.
#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "zs/gen/place.h"

namespace zs::atlas {

struct TownCard {
  int index = 0;        // in the planet's town list
  int country = 0;
  int rank = 0;
  double people = 0;
  std::uint64_t cell = 0;
  double lat = 0, lon = 0;
};

struct CountryCard {
  std::string name, income, profile;
  double area_km2 = 0, people = 0, endowment = 0;
  bool landlocked = false;
  int capital_held = 0, periods = 0, sectors = 0;
  std::vector<TownCard> towns;  // by rank
};

struct WorldCard {
  std::uint64_t seed = 0;
  std::size_t cells = 0;
  double land_km2 = 0, people = 0;
  std::size_t countries = 0, towns = 0;
  int start_polities = 0, periods = 0;
  std::size_t events = 0;
  double fed = 0;  // people the land can feed
};

struct CardsState;
std::shared_ptr<CardsState> make_cards(const std::string& data_dir);
// For checks without a click: pick the first town, then the first pop and their house, as they become available.
void cards_autopick(CardsState& st, bool on, float scroll_to = -1.0f);  // scroll_to: pixels, -1 = the end

// Editing on the world card (slice 5, decision 0074 rule 4): the world's own settings, edited as a draft and applied
// to the world (unlocked) or to a fork of it (locked, once it has a save); presets next to the executable.
struct WorldEditor {
  gen::WorldSettings* settings = nullptr;  // the world's record, written on apply
  bool locked = false;
  std::string world_name;
  std::function<void()> apply;                                                   // the record was changed: save it, redraw
  std::function<void(const std::string& new_name, const gen::WorldSettings&)> fork;
  std::function<std::vector<std::string>()> presets;
  std::function<gen::WorldSettings(const std::string&)> load_preset;
  std::function<void(const std::string&, const gen::WorldSettings&)> save_preset;
};

// "realistic" when every setting is in its realistic band, else "sandbox" (sandbox parameters §1.4).
std::string realism_of(const CardsState& st, const gen::WorldSettings& s);
// How many settings differ from Earth's values.
int touched_count(const CardsState& st, const gen::WorldSettings& s);

// Draws the Cards page for a generated planet. `select_country` (-1: none) comes from the planet click.
void draw_cards(CardsState& st, const WorldCard& world, const std::vector<CountryCard>& countries, int select_country,
                WorldEditor* editor = nullptr);
// The country the cards page shows (-1: none), so the planet can highlight it.
int cards_country(const CardsState& st);

}  // namespace zs::atlas
