// The cards (atlas design §5, spec 55 slice 3): a country, a town, a pop, a house and the world, each shown as what
// it has against what the design says it needs (data/atlas/cards.toml). Read-only views over the generators.
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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
void cards_autopick(CardsState& st, bool on);

// Draws the Cards page for a generated planet. `select_country` (-1: none) comes from the planet click.
void draw_cards(CardsState& st, const WorldCard& world, const std::vector<CountryCard>& countries, int select_country);
// The country the cards page shows (-1: none), so the planet can highlight it.
int cards_country(const CardsState& st);

}  // namespace zs::atlas
