// The cards (atlas design §5, spec 55 slice 3): a country, a town, a pop, a house and the world, each shown as what
// it has against what the design says it needs (data/atlas/cards.toml). Read-only views over the generators.
#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "zs/gen/place.h"

struct SDL_Renderer;

namespace zs::atlas {

struct TownCard {
  int index = 0;        // in the planet's town list
  int country = 0;
  int rank = 0;
  double people = 0;
  std::uint64_t cell = 0;
  double lat = 0, lon = 0;
  std::string plants;  // spec 93: its plants by kind
};

struct CountryCard {
  std::string name, income, profile;
  std::string origin, alliance;  // spec 91: how the history made it, the alliance it is in
  std::string industry;          // spec 93: its plants by kind
  std::string energy;            // spec 94: its electricity: demand and mix
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
// Spec 92: what the town card needs from the Atlas: the renderer for its pictures and, for a planet town (by index), the
// town the one world would open there (spec 73's record), made on a worker thread; the world's seed for its plots.
void cards_set_town_source(CardsState& st, SDL_Renderer* renderer,
                           std::function<std::shared_ptr<const gen::TownRecord>(int town)> record_of, std::uint64_t seed);
// Spec 96: a town's power and outage as a line for its card (by town index), asked once per town.
void cards_set_power_source(CardsState& st, std::function<std::string(int town)> power_of);
// Spec 95: a town's food supply as a line for its card (by town index), asked once per town.
void cards_set_supply_source(CardsState& st, std::function<std::string(int town)> supply_of);
// True while a town's streets are being laid out (checks wait for them).
bool cards_busy(const CardsState& st);
// For checks without a click: pick the first town, then the first pop and their house, as they become available.
void cards_autopick(CardsState& st, bool on, float scroll_to = -1.0f);  // scroll_to: pixels, -1 = the end

// The world's setup (decision 0074, spec 84): chosen when a world is made, in the New world dialog, as a draft with a
// slider per setting, the start date, presets and a preview. A made world's settings are shown read-only on its card:
// its planet is generated from them, so another setup is another world.
struct SetupDraft {
  gen::WorldSettings s;
  char date_text[16] = "";
  char preset_name[64] = "";
  int preset_pick = -1;
  std::string note, error;
};
struct SetupHooks {
  std::function<std::vector<std::string>()> presets;
  std::function<gen::WorldSettings(const std::string&)> load_preset;
  std::function<void(const std::string&, const gen::WorldSettings&)> save_preset;
  // Spec 84 rule 3: a preview of the draft before Create; null leaves the button out.
  std::function<void(const gen::WorldSettings&)> preview;
  std::function<void()> draw_preview;
};
void reset_setup(SetupDraft& d, const gen::WorldSettings& s = {});
// Draws the setup editor over the draft; false while the start date does not parse.
bool draw_world_setup(const CardsState& st, SetupDraft& d, const SetupHooks& hooks);

// A made world on its card: its settings, read-only.
struct WorldRecord {
  const gen::WorldSettings* settings = nullptr;
  std::string world_name;
};

// "realistic" when every setting is in its realistic band, else "sandbox" (sandbox parameters §1.4).
std::string realism_of(const CardsState& st, const gen::WorldSettings& s);
// How many settings differ from Earth's values.
int touched_count(const CardsState& st, const gen::WorldSettings& s);

// Draws the Cards page for a generated planet. `select_country` (-1: none) comes from the planet click.
void draw_cards(CardsState& st, const WorldCard& world, const std::vector<CountryCard>& countries, int select_country,
                const WorldRecord* record = nullptr);
// The country the cards page shows (-1: none), so the planet can highlight it.
int cards_country(const CardsState& st);

}  // namespace zs::atlas
