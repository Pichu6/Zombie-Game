// The one world (spec 41): the run's Tier 0 systems plus the pop registry, the economy, orders and the grid tree,
// with the census rows as the registry's founders. Spec: docs/tier0/specs/41-one-world.md
#include <memory>

#include "zs/calendar.h"
#include "zs/demographics.h"
#include "zs/economy_system.h"
#include "zs/expectations.h"
#include "zs/missing_persons.h"
#include "zs/grid_system.h"
#include "zs/orders_system.h"
#include "zs/pop_system.h"
#include "zs/schedule.h"
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
// Spec 48: the block's country is its place's income group's base country; spec 49: or its real profile.
std::unique_ptr<PopSystem> make_pops(World& w, const Census& census, const Place& place) {
  const std::string data = data_dir();
  auto pops = std::make_unique<PopSystem>(w, make_pop_params(load_nation(data + (place.profile.empty() ? "/setup/nations/base_" + place.income : "/setup/nations/real/" + place.profile) + ".toml"), data));
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

// Spec 59: the schedule's commitments, as holders see them. A pop of the census whose household has another living
// adult is expected somewhere: at work or school at the segment's start (a shift start), and home at the end of the
// way back (arriving home). The holders are the household's other living adults, in census order: the first phones,
// the last reports. Born pops and people nobody lives with are expected nowhere (nobody notices their absence).
NextCommitment make_commitments(const Census& census, const CalendarSystem& calendar, const Place& place, std::uint64_t world_seed) {
  auto table = std::make_shared<TimeUseTable>(timeuse_for(data_dir(), place.profile, place.income));
  const Census* people = &census;
  const CalendarSystem* cal = &calendar;
  return [table, people, cal, world_seed](PopId pop, std::int64_t from) -> std::optional<Commitment> {
    const auto& all = people->people();
    if (is_born(pop) || founder_slot_of(pop) >= all.size()) return std::nullopt;
    const Person& p = all[founder_slot_of(pop)];
    if (is_dead(p.fate) || p.household.empty()) return std::nullopt;
    Commitment out;
    for (std::size_t i = 0; i < all.size(); ++i)
      if (i != founder_slot_of(pop) && all[i].household == p.household && !is_dead(all[i].fate) && all[i].age >= 16)
        out.holders.push_back(census_pop_id(static_cast<std::uint32_t>(i)));
    if (out.holders.empty()) return std::nullopt;
    SchedulePerson sp;
    sp.key = keyed_u64(world_seed, Purpose::PopLife, pop, 0x5C4ED);  // the schedule's own draw of the pop's life
    sp.age = p.age;
    sp.sex = p.sex == Sex::Female ? 'f' : 'm';
    sp.employment = p.age < 5 ? Employment::Child : p.age < 18 ? Employment::Student : p.age < 65 ? Employment::FullTime : Employment::Retired;
    sp.has_workplace = false;  // the block has no workplaces placed: work is done at home with no commute (spec 54 rule 4)
    const std::int64_t start_day = cal->start_day();
    const std::int64_t first_day = civil_time(start_day, from).day_number;
    bool found = false;
    for (std::int64_t k = 0; k < 4 && !found; ++k) {
      const std::int64_t dn = first_day + k;
      const std::int64_t midnight = (dn - start_day) * kTicksPerGameDay - kStartTimeOfDayTicks;
      const DayPlan plan = plan_for_tick(*table, cal->calendar(), start_day, sp, midnight + 12 * kTicksPerGameHour);
      for (const Segment& s : plan.segments) {
        if (!s.commitment) continue;
        const bool back_home = s.activity == DayActivity::Commute;
        const std::int64_t due = midnight + static_cast<std::int64_t>(back_home ? s.end_min : s.start_min) * (kTicksPerGameHour / 60);
        if (due < from || (found && due >= out.due_tick)) continue;
        out.due_tick = due;
        out.kind = back_home ? CommitmentKind::ArriveHome : CommitmentKind::ShiftStart;
        found = true;
      }
    }
    if (!found) return std::nullopt;
    return out;
  };
}

}  // namespace

void add_world_systems(Tier0& t, const Place& place, std::int64_t start_day) {
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
  // Spec 53 rule 10: the calendar of the place's country, from the world's start date (decision 0072; the default
  // unless a world's settings say otherwise, spec 55).
  auto calendar = std::make_unique<CalendarSystem>(
      start_day, load_country_calendar(calendar_path(data_dir(), calendar_id(place.profile, place.income))));
  t.calendar = calendar.get();
  // Spec 59: broken commitments are noticed. The schedule's commitments and the household's holders (decision 0070).
  auto expectations = std::make_unique<Expectations>(
      load_expectation_params(data_dir() + "/life/expectations.toml"), make_commitments(*t.census, *calendar, place, w.rng.seed()));
  t.expectations = expectations.get();
  // Spec 60: real people go missing without any player (decision 0070 rule 6): every living founder is a Poisson
  // process of absences at the country's rates by age class; the ladder above reacts to them.
  auto missing = std::make_unique<MissingPersons>(
      load_missing_params(data_dir() + "/life/missing.toml", place.profile, place.income), expectations.get(),
      [people = t.census](PopId id) {
        return !is_born(id) && founder_slot_of(id) < people->people().size() && !is_dead(people->people()[founder_slot_of(id)].fate);
      });
  for (std::size_t i = 0; i < t.census->people().size(); ++i)
    if (!is_dead(t.census->people()[i].fate)) missing->track(w, census_pop_id(static_cast<std::uint32_t>(i)), t.census->people()[i].age);
  t.missing = missing.get();
  // Rule 1: cold systems after the hot ones.
  w.add_system(std::move(pops));
  w.add_system(std::move(econ));
  w.add_system(std::move(orders));
  w.add_system(std::move(grids));
  w.add_system(std::move(where));
  w.add_system(std::move(calendar));
  w.add_system(std::move(expectations));
  w.add_system(std::move(missing));
}

}  // namespace zs::tier0
