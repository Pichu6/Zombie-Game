// The one world (spec 41): the run's Tier 0 systems plus the pop registry, the economy, orders and the grid tree,
// with the census rows as the registry's founders. Spec: docs/tier0/specs/41-one-world.md
#include <memory>

#include "zs/demographics.h"
#include "zs/economy_system.h"
#include "zs/grid_system.h"
#include "zs/orders_system.h"
#include "zs/pop_system.h"
#include "zs/tier0.h"

namespace zs::tier0 {

namespace {

constexpr std::uint64_t kBlockCohort = 1;
constexpr std::uint32_t kBlockRegion = 1;
constexpr std::uint64_t kHouseholdRoster = std::uint64_t{1} << 32;  // the generator's household roster ids (registry_bridge)

// Placeholder money until the data spec (decision 0052 item 4) gives real wallets and budgets.
constexpr std::int64_t kPlaceholderAdultWalletCents = 50'000;
constexpr std::int64_t kPlaceholderRegionBudgetCents = 10'000'000;
constexpr std::int64_t kPlaceholderCountryBudgetCents = 1'000'000'000;

// Rule 2: one founder per census row, slot = row, the row's age and sex; households as rosters.
// Spec 48: the block's country is its place's income group's base country.
std::unique_ptr<PopSystem> make_pops(World& w, const Census& census, const Place& place) {
  const std::string data = data_dir();
  auto pops = std::make_unique<PopSystem>(w, make_pop_params(load_nation(data + "/setup/nations/base_" + place.income + ".toml"), data));
  PopRegistry& reg = pops->setup(w);
  const auto& people = census.people();
  reg.add_cohort({kBlockCohort, kBlockRegion, 0, people.size()});
  std::vector<FounderSpec> specs;
  specs.reserve(people.size());
  for (const Person& p : people)
    specs.push_back({static_cast<std::uint16_t>(p.age < 0 ? 0 : p.age), p.sex == Sex::Female ? PopSex::Female : PopSex::Male});
  reg.set_founder_specs(kBlockCohort, std::move(specs));
  std::vector<std::string> households;  // by first appearance
  for (std::size_t i = 0; i < people.size(); ++i) {
    const PopId id = census_pop_id(static_cast<std::uint32_t>(i));
    if (is_dead(people[i].fate)) reg.set_death_year(id, 0.0);  // died before the run's first tick
    if (people[i].household.empty()) continue;
    std::size_t h = 0;
    while (h < households.size() && households[h] != people[i].household) ++h;
    if (h == households.size()) {
      households.push_back(people[i].household);
      reg.add_roster(kHouseholdRoster | h, kBlockRegion, RosterKind::Household);
    }
    reg.roster_join(kHouseholdRoster | h, id, 0.0);
  }
  return pops;
}

// Rule 3: country, region, the map's three buildings, one owner per person; placeholder money.
std::unique_ptr<EconomySystem> make_economy(World& w, const Census& census, WorldOwners& ids) {
  auto econ = std::make_unique<EconomySystem>(w);
  Owners& o = econ->owners();
  ids.country = o.add(w.ledger, "country", Scope::Country);
  ids.region = o.add(w.ledger, "block town", Scope::Region, ids.country);
  ids.main_building = o.add(w.ledger, "main building", Scope::Building, ids.region);
  ids.house_a = o.add(w.ledger, "house A", Scope::Building, ids.region);
  ids.house_b = o.add(w.ledger, "house B", Scope::Building, ids.region);
  const ResourceId money = econ->money().resource();
  for (const Person& p : census.people()) {
    const OwnerId pop = o.add(w.ledger, p.name, Scope::Pop, ids.region);
    ids.pops.push_back(pop);
    if (p.age >= 18 && !is_dead(p.fate)) w.ledger.mint(pop, money, kPlaceholderAdultWalletCents);
  }
  w.ledger.mint(ids.country, money, kPlaceholderCountryBudgetCents);
  w.ledger.mint(ids.region, money, kPlaceholderRegionBudgetCents);
  econ->start_lots(w);
  econ->start_establishments(w);
  econ->start_populations(w);
  return econ;
}

// Rule 4: nation grid -> district grid -> one load per building. Solved once; not yet coupled to Power.
std::unique_ptr<GridSystem> make_grids(WorldOwners& ids) {
  auto grids = std::make_unique<GridSystem>();
  GridTree& t = grids->tree();
  ids.nation = t.add_grid("nation");
  t.add_node(ids.nation, {"power plant", NodeKind::Source, 50'000'000});
  const std::size_t town = t.add_node(ids.nation, {"block town", NodeKind::Load});
  t.add_edge(ids.nation, 0, town);
  ids.district = t.add_grid("block district", ids.nation);
  const std::size_t feed = t.add_node(ids.district, {"substation", NodeKind::Hub});
  for (const char* name : {"main building", "house A", "house B"}) {
    const std::size_t load = t.add_node(ids.district, {name, NodeKind::Load, 0, name[0] == 'm' ? 6'000 : 3'000});
    t.add_edge(ids.district, feed, load);
  }
  t.attach(ids.nation, town, ids.district, feed);
  grids->start();
  return grids;
}

}  // namespace

void add_world_systems(Tier0& t, const Place& place) {
  World& w = *t.world;
  auto pops = make_pops(w, *t.census, place);
  auto econ = make_economy(w, *t.census, t.owners);
  auto orders = std::make_unique<OrdersSystem>(econ.get(), pops.get());
  std::vector<PopId> audience;  // rule 5: the living founders
  for (std::size_t i = 0; i < t.census->people().size(); ++i)
    if (!is_dead(t.census->people()[i].fate)) audience.push_back(census_pop_id(static_cast<std::uint32_t>(i)));
  orders->set_audience(std::move(audience));
  auto grids = make_grids(t.owners);
  t.pops = pops.get();
  t.econ = econ.get();
  t.orders = orders.get();
  t.grids = grids.get();
  t.census->attach_pops(t.pops);
  auto where = std::make_unique<PlaceSystem>(place);  // spec 42
  t.place = where.get();
  t.power->attach_place(t.place);
  // Rule 1: cold systems after the hot ones.
  w.add_system(std::move(pops));
  w.add_system(std::move(econ));
  w.add_system(std::move(orders));
  w.add_system(std::move(grids));
  w.add_system(std::move(where));
}

}  // namespace zs::tier0
