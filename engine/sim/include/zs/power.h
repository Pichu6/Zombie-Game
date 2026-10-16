// The building's power network, the fridge, and the light it makes.
// Specs: docs/tier0/specs/17-power-network.md, 19-light-field.md
//
// Compressed model: one mains feed and one generator behind a transfer switch, a few breakers, and
// receivers that are map objects matched by kind. Solved in watts per circuit, only when something
// changes. Light is lamp lumens spread over the walled space they hang in, plus a share of daylight.
// Spec 82: per building. The grid feeds each building through its service; each building's panel feeds its own copy of
// the circuits; an appliance belongs to the building whose area holds it. The generator and the transfer switch are the
// main building's. Every fridge has its own temperature.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "zs/actions.h"

namespace zs {

enum class Transfer : std::uint8_t { Off, Mains, Generator };
enum class StallCause : std::uint8_t { Overload = 1, Surge = 2, NoFuel = 3 };

struct GeneratorDef {
  double capacity_w = 3000, surge_w = 3500;
  double idle_lph = 0.6, full_lph = 1.9;
  double tank_l = 15;
  double noise_db_1m = 89;
};
struct CircuitDef {
  std::string id;
  double rating_w = 1800;
};
struct ReceiverDef {
  std::string kind;
  double demand_w = 0, surge_w = 0, lumens = 0;
  std::size_t circuit = 0;
  bool on_at_start = false;
};
// Spec 82: a building on the map: its area (levels and a rectangle), its service, whether it holds the generator, and the
// node of spec 35's block district it hangs from.
struct BuildingDef {
  std::string id, name, grid_node;
  int z0 = 0, z1 = 0, x0 = 0, y0 = 0, x1 = 0, y1 = 0;
  double service_w = 24000;
  bool generator = false;
  bool holds(const Cell& c) const { return c.z >= z0 && c.z <= z1 && c.x >= x0 && c.x <= x1 && c.y >= y0 && c.y <= y1; }
};
struct BuildingState {
  bool mains = true;           // spec 82 rule 2: its service is fed (its node in the district is connected)
  std::vector<bool> tripped;   // per circuit
  bool main_tripped = false;   // the service's main breaker
  double load_w = 0;
  double reported_w = -1;      // the demand last reported to the grid tree
};
struct FridgeDef {
  double cold_c = 3, room_c = 20, cool_tau_h = 1, warm_tau_h = 6, life_days_at_4c = 7, halving_c = 4;
};

struct Receiver {
  EntityId object = 0;
  std::size_t def = 0;
  std::size_t building = 0;  // spec 82
  Cell at;
  bool on = false;
  bool powered = false;
};

struct Fridge {  // spec 82: one per fridge
  EntityId object = 0;
  std::size_t building = 0;
  double c = 3, spoil = 0;
};

struct WindowLeak {
  EntityId window = 0;
  EdgeRef edge;
  double room_lux = 0;  // inside, from lamps
  double cover = 0;     // 0 = bare glass, 1 = fully covered
  double leak_lux = 0;  // what shows outside
  double seen_m = 0;    // how far away it stands out against the night (0 = it doesn't)
};

class PlaceSystem;

class Power : public System {
 public:
  Power(World& w, SiteMap& map, Assemblies& items, Actors& actors);
  const char* name() const override { return "power"; }
  void update(World& w) override;
  void on_scheduled(World& w, const ScheduledEvent& e) override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& r) override;

  // Setup, after Actors::populate: finds receivers, the generator, its tank and the fridge, wires
  // the action hooks into Actors and schedules the grid failure (drawn in [day 2, day 5)).
  void load_file(const std::string& path);
  // Spec 96: the grid's failure at a given tick (from the town's grid) instead of spec 17's draw; before setup.
  void set_failure_tick(std::int64_t tick) { fixed_fail_tick_ = tick; }
  void setup(World& w);
  // Spec 42 rule 6: in the one world the fridge warms towards the place's indoor temperature, not room_c.
  void attach_place(const PlaceSystem* place) { place_ = place; }
  // Spec 82: the one world links each building to its node of spec 35's tree: its load is reported as the node's demand,
  // and the block's grid failure is its substation failing. Set by the setup, never saved.
  using LoadReport = std::function<void(World&, std::size_t building, double load_w)>;
  using GridFailedReport = std::function<void(World&, EventId)>;
  void set_grid_link(LoadReport load, GridFailedReport failed) {
    report_load_ = std::move(load);
    report_failed_ = std::move(failed);
    for (BuildingState& st : state_) st.reported_w = st.load_w;  // the setup syncs the tree to the loads as they are
  }

  // Controls (the actions call these; tests may too). Each returns false if it could not be done.
  bool start_generator(World& w, EventId cause);
  bool stop_generator(World& w, EventId cause);
  bool set_transfer(World& w, Transfer t, EventId cause);
  bool set_on(World& w, EntityId object, bool on, EventId cause);
  bool reset_breakers(World& w, EventId cause);
  // Pours fuel from the body's carried containers into the tank, up to its room. Returns uL moved.
  std::int64_t refuel(World& w, EntityId body, EventId cause);
  void fail_grid(World& w, EventId cause);
  // Spec 82 rule 2: whether a building's service is fed, as spec 35's tree says after each solve. A change re-solves the
  // panels; the same value does nothing.
  void set_mains(World& w, std::size_t building, bool alive, EventId cause);

  // Readouts.
  bool mains_alive() const { return mains_alive_; }  // the block's grid has not failed (spec 17)
  bool building_mains(std::size_t b) const { return state_[b].mains; }
  bool generator_running() const { return running_; }
  Transfer transfer() const { return transfer_; }
  double load_w() const { return load_w_; }
  double generator_load_w() const { return running_ && transfer_ == Transfer::Generator ? load_w_ : 0; }
  std::int64_t fuel_ul(const World& w) const;
  ResourceId gasoline() const { return gasoline_; }
  std::int64_t tank_ul() const { return static_cast<std::int64_t>(gen_.tank_l * 1e6); }
  double fuel_lph() const;  // at the current load
  bool tripped(std::size_t circuit) const { return state_[main_].tripped[circuit]; }  // the main building's
  bool powered(EntityId object) const;
  double fridge_temp_c() const;    // the main building's first fridge
  double spoil_progress() const;
  // Spec 82: the buildings and each fridge.
  const std::vector<BuildingDef>& buildings() const { return buildings_; }
  const BuildingState& building_state(std::size_t b) const { return state_[b]; }
  double building_load_w(std::size_t b) const { return state_[b].load_w; }
  const std::vector<Fridge>& fridges() const { return fridges_; }
  double fridge_temp_c(EntityId fridge) const;
  std::int64_t grid_failure_tick() const { return grid_fail_tick_; }
  const std::vector<Receiver>& receivers() const { return receivers_; }
  const std::vector<ReceiverDef>& receiver_defs() const { return rdefs_; }
  const std::vector<CircuitDef>& circuits() const { return circuits_; }
  const GeneratorDef& generator() const { return gen_; }
  EntityId generator_object() const { return gen_object_; }
  Cell generator_cell() const { return gen_cell_; }

  // Light (spec 19).
  static double ambient_lux(double hour);  // outdoors
  double lamp_lux(const Cell& c) const;     // from powered lamps in the walled space of c
  double lux_at(const World& w, const Cell& c) const;
  std::vector<WindowLeak> window_leaks(const World& w) const;
  int space_of(const Cell& c) const;

 private:
  // switched_on: a receiver just turned on (its start surge); source_changed: every running motor
  // starts at once (generator started, panel moved to it).
  void solve(World& w, EventId cause, int switched_on = -1, bool source_changed = false);
  void stall(World& w, StallCause why, EventId cause);
  void step_fuel(World& w);
  void step_fridge(World& w);
  void build_spaces() const;  // spec 73: built again when the map gains levels (a place opened in play)
  std::size_t cell_index(const Cell& c) const;
  int receiver_of(EntityId object) const;

  SiteMap& map_;
  Assemblies& items_;
  Actors& actors_;
  const PlaceSystem* place_ = nullptr;
  GeneratorDef gen_;
  FridgeDef fridge_;
  std::vector<CircuitDef> circuits_;  // every building's panel has its own copy (spec 82)
  std::vector<ReceiverDef> rdefs_;
  std::vector<BuildingDef> buildings_;
  std::size_t main_ = 0;              // the building with the generator
  LoadReport report_load_;            // never saved
  GridFailedReport report_failed_;    // never saved
  ResourceId gasoline_ = 0, food_ = 0, spoiled_ = 0;
  OwnerId burned_ = 0;
  ConversionId spoil_conv_ = 0;
  // Found at setup (derived from the map; not saved).
  std::vector<Receiver> receivers_;  // `on` and `powered` are saved
  EntityId gen_object_ = 0, panel_object_ = 0;
  Cell gen_cell_{};
  mutable std::vector<int> spaces_;          // per cell: walled space id, -1 = not walkable
  mutable std::vector<int> space_cells_;     // cells per space
  // State (saved).
  bool mains_alive_ = true;
  Transfer transfer_ = Transfer::Mains;
  bool running_ = false;
  double load_w_ = 0;
  std::vector<BuildingState> state_;  // per building
  std::vector<Fridge> fridges_;
  double burn_rest_ul_ = 0;
  std::int64_t grid_fail_tick_ = -1;
  std::int64_t fixed_fail_tick_ = -1;  // spec 96, set by the setup (not saved)
};

}  // namespace zs
