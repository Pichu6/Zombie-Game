// The one world (spec 41): the run's Tier 0 systems plus the pop registry, the economy, orders and the grid tree,
// with the census rows as the registry's founders. Spec: docs/tier0/specs/41-one-world.md
#include <memory>

#include "zs/bonds.h"
#include "zs/calendar.h"
#include "zs/demographics.h"
#include "zs/economy_system.h"
#include "zs/expectations.h"
#include "zs/missing_persons.h"
#include "zs/grid_system.h"
#include "zs/orders_system.h"
#include "zs/payroll.h"
#include "zs/police.h"
#include "zs/police_staff.h"
#include "zs/pop_system.h"
#include "zs/schedule.h"
#include "zs/tier0.h"

namespace zs::tier0 {

namespace {

constexpr std::uint64_t kBlockCohort = 1;
constexpr std::uint32_t kBlockRegion = 1;
constexpr std::uint64_t kHouseholdRoster = std::uint64_t{1} << 32;  // the generator's household roster ids (registry_bridge)
constexpr std::uint64_t kEmployerRoster = std::uint64_t{1} << 33;   // spec 62: the placeholder employer's workplace roster
constexpr std::int64_t kPlaceholderPayrollCents = 10'000'000'000;       // what the placeholder employer holds to pay wages (until establishments are placed)
constexpr std::uint64_t kForceCohort = 2;                                // spec 65: the police force serving the town
constexpr std::uint64_t kPoliceRoster = std::uint64_t{1} << 34;          // its workplace roster

// Placeholder money until the data spec (decision 0052 item 4) gives real wallets and budgets.
constexpr std::int64_t kPlaceholderAdultWalletCents = 50'000;
constexpr std::int64_t kPlaceholderRegionBudgetCents = 10'000'000;
constexpr std::int64_t kPlaceholderCountryBudgetCents = 1'000'000'000;

// The household roster id of each census row (0 for none), by first appearance of the household name: the same ids make_pops
// gives the rosters it adds (spec 63: the roster a holder leaves).
std::vector<std::uint64_t> household_rosters(const Census& census) {
  const auto& people = census.people();
  std::vector<std::string> households;
  std::vector<std::uint64_t> out(people.size(), 0);
  for (std::size_t i = 0; i < people.size(); ++i) {
    if (people[i].household.empty()) continue;
    std::size_t h = 0;
    while (h < households.size() && households[h] != people[i].household) ++h;
    if (h == households.size()) households.push_back(people[i].household);
    out[i] = kHouseholdRoster | h;
  }
  return out;
}

// Spec 62: a living pop of working age (18 to 64, the schedule's full-time workers) has a job with the placeholder employer.
bool has_job(const Person& p) { return !is_dead(p.fate) && p.age >= 18 && p.age < 65; }

// Rule 2: one founder per census row, slot = row, the row's age and sex; households as rosters.
// Spec 48: the block's country is its place's income group's base country; spec 49: or its real profile.
std::unique_ptr<PopSystem> make_pops(World& w, const Census& census, const Place& place, int officers) {
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
  reg.add_roster(kEmployerRoster, kBlockRegion, RosterKind::Workplace);  // spec 62
  for (std::size_t i = 0; i < people.size(); ++i)
    if (has_job(people[i])) reg.roster_join(kEmployerRoster, census_pop_id(static_cast<std::uint32_t>(i)), 0.0);
  // Spec 65: the police force that serves the town: more founders after the census's, on their own roster (a workplace).
  reg.add_cohort({kForceCohort, kBlockRegion, static_cast<std::uint32_t>(people.size()), static_cast<std::uint64_t>(officers)});
  std::vector<FounderSpec> force;
  for (int i = 0; i < officers; ++i) force.push_back({static_cast<std::uint16_t>(25 + (i * 7) % 31), i % 4 == 0 ? PopSex::Female : PopSex::Male});
  reg.set_founder_specs(kForceCohort, std::move(force));
  reg.add_roster(kPoliceRoster, kBlockRegion, RosterKind::Workplace);
  for (int i = 0; i < officers; ++i) reg.roster_join(kPoliceRoster, census_pop_id(static_cast<std::uint32_t>(people.size()) + static_cast<std::uint32_t>(i)), 0.0);
  return pops;
}

// Rule 3: country, region, the map's three buildings, one owner per person; placeholder money.
std::unique_ptr<EconomySystem> make_economy(World& w, const Census& census, WorldOwners& ids, int officers) {
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
  for (int i = 0; i < officers; ++i) {  // spec 65: the officers' wallets, by registry slot after the census's
    const OwnerId pop = o.add(w.ledger, "officer " + std::to_string(i + 1), Scope::Pop, ids.region);
    ids.pops.push_back(pop);
    w.ledger.mint(pop, money, kPlaceholderAdultWalletCents);
  }
  w.ledger.mint(ids.country, money, kPlaceholderCountryBudgetCents);
  w.ledger.mint(ids.region, money, kPlaceholderRegionBudgetCents);
  ids.employers = o.add(w.ledger, "block employers", Scope::Building, ids.region);  // spec 62
  w.ledger.mint(ids.employers, money, kPlaceholderPayrollCents);
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
NextCommitment make_commitments(const Census& census, const CalendarSystem& calendar, const Place& place, std::uint64_t world_seed,
                               std::shared_ptr<const Expectations*> lost_jobs, std::shared_ptr<const Bonds*> bonds) {
  auto table = std::make_shared<TimeUseTable>(timeuse_for(data_dir(), place.profile, place.income));
  const Census* people = &census;
  const CalendarSystem* cal = &calendar;
  return [table, people, cal, world_seed, lost_jobs, bonds](PopId pop, std::int64_t from) -> std::optional<Commitment> {
    const auto& all = people->people();
    if (is_born(pop) || founder_slot_of(pop) >= all.size()) return std::nullopt;
    const Person& p = all[founder_slot_of(pop)];
    if (is_dead(p.fate)) return std::nullopt;
    Commitment out;
    // A pop that left its household because of a strained bond lives apart (nobody at home expects it), and a holder that left no longer holds.
    const bool apart = *bonds && (*bonds)->left_household(pop);
    for (std::size_t i = 0; i < all.size() && !p.household.empty() && !apart; ++i)
      if (i != founder_slot_of(pop) && all[i].household == p.household && !is_dead(all[i].fate) && all[i].age >= 16 &&
          !(*bonds && (*bonds)->left_household(census_pop_id(static_cast<std::uint32_t>(i)))))
        out.holders.push_back(census_pop_id(static_cast<std::uint32_t>(i)));
    SchedulePerson sp;
    sp.key = keyed_u64(world_seed, Purpose::PopLife, pop, 0x5C4ED);  // the schedule's own draw of the pop's life
    sp.age = p.age;
    sp.sex = p.sex == Sex::Female ? 'f' : 'm';
    sp.employment = p.age < 5 ? Employment::Child : p.age < 18 ? Employment::Student : p.age < 65 ? Employment::FullTime : Employment::Retired;
    if (sp.employment == Employment::FullTime && *lost_jobs && (*lost_jobs)->lost_job(pop)) sp.employment = Employment::NotEmployed;  // separated: no more shifts
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
        out.employer = s.activity == DayActivity::Work ? EmployerKind::Private : EmployerKind::None;  // the block has no police employers placed
        out.minutes = s.end_min - s.start_min;
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
  // Spec 65: the police force serving the town, sized by its population: officers are pops after the census's, with shifts.
  const ForceParams force_params = load_force_params(data_dir() + "/institutions/police/force.toml", place.profile, place.income);
  const int officers = force_size(force_params, place.town_people);
  auto pops = make_pops(w, *t.census, place, officers);
  auto econ = make_economy(w, *t.census, t.owners, officers);
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
  // Spec 61: the employer's rule (decision 0070 rule 3); a separated pop's schedule has no more shifts, so the provider
  // reads the system it is given to.
  auto lost_jobs = std::make_shared<const Expectations*>(nullptr);
  auto bonds_link = std::make_shared<const Bonds*>(nullptr);
  // Spec 65: the officers as pops with shifts; their commitments come from the force, everyone else's from the census.
  auto staff = std::make_shared<PoliceStaff>(force_params, std::make_shared<TimeUseTable>(timeuse_for(data_dir(), place.profile, place.income)),
                                             calendar.get(), w.rng.seed());
  const std::size_t census_size = t.census->people().size();
  for (int i = 0; i < officers; ++i) staff->add_officer(census_pop_id(static_cast<std::uint32_t>(census_size) + static_cast<std::uint32_t>(i)), 25 + (i * 7) % 31, i % 4 == 0 ? 'f' : 'm');
  t.force = staff;
  const NextCommitment census_commitments = make_commitments(*t.census, *calendar, place, w.rng.seed(), lost_jobs, bonds_link);
  const NextCommitment commitments = [census_commitments, staff](PopId pop, std::int64_t from) -> std::optional<Commitment> {
    return staff->is_officer(pop) ? staff->next_commitment(pop, from) : census_commitments(pop, from);
  };
  auto expectations = std::make_unique<Expectations>(
      load_expectation_params(data_dir() + "/life/expectations.toml"),
      commitments,
      load_employer_policy(data_dir() + "/employment/absence_policy.toml", place.profile, place.income));
  *lost_jobs = expectations.get();
  t.expectations = expectations.get();
  // Spec 60: real people go missing without any player (decision 0070 rule 6): every living founder is a Poisson
  // process of absences at the country's rates by age class; the ladder above reacts to them.
  auto missing = std::make_unique<MissingPersons>(
      load_missing_params(data_dir() + "/life/missing.toml", place.profile, place.income), expectations.get(),
      [people = t.census, census_size, officers](PopId id) {
        if (!is_born(id) && founder_slot_of(id) >= census_size && founder_slot_of(id) < census_size + static_cast<std::size_t>(officers)) return true;  // an officer
        return !is_born(id) && founder_slot_of(id) < people->people().size() && !is_dead(people->people()[founder_slot_of(id)].fate);
      });
  for (std::size_t i = 0; i < t.census->people().size(); ++i)
    if (!is_dead(t.census->people()[i].fate)) missing->track(w, census_pop_id(static_cast<std::uint32_t>(i)), t.census->people()[i].age);
  for (int i = 0; i < officers; ++i) missing->track(w, census_pop_id(static_cast<std::uint32_t>(census_size) + static_cast<std::uint32_t>(i)), 25 + (i * 7) % 31);  // spec 65
  t.missing = missing.get();
  // Spec 62: money and labour: the placeholder employer pays each shift worked and nothing for a shift missed; a separation takes
  // the pop off its workplace roster.
  auto payroll = std::make_unique<Payroll>(
      load_wage_params(data_dir() + "/employment/wages.toml", place.profile, place.income), expectations.get(), commitments,
      t.owners.employers, kEmployerRoster, [owners = t.owners.pops](PopId id) { return owners[founder_slot_of(id)]; });
  for (std::size_t i = 0; i < t.census->people().size(); ++i)
    if (has_job(t.census->people()[i])) payroll->track(census_pop_id(static_cast<std::uint32_t>(i)));
  for (int i = 0; i < officers; ++i) payroll->track(census_pop_id(static_cast<std::uint32_t>(census_size) + static_cast<std::uint32_t>(i)));  // spec 65: the officers are paid too
  t.payroll = payroll.get();
  // Spec 63: the household after an absence or a lost job: each holder carries a strain toward the pop, easing over time; past
  // a limit the holder leaves the household's roster. The ladder's system tells it.
  auto bonds = std::make_unique<Bonds>(load_bond_params(data_dir() + "/life/household.toml"),
                                       [rosters = household_rosters(*t.census)](PopId id) -> std::uint64_t {
                                         return !is_born(id) && founder_slot_of(id) < rosters.size() ? rosters[founder_slot_of(id)] : 0;
                                       });
  expectations->add_listener(bonds.get());
  *bonds_link = bonds.get();
  t.bonds = bonds.get();
  // Spec 64: the police's missing-person case: a risk tier, a queue for free searches, the national record. The police own the
  // search (the ladder no longer opens one by itself).
  auto police = std::make_unique<PoliceCases>(
      load_police_params(data_dir() + "/institutions/police/missing.toml", place.profile, place.income), expectations.get(),
      [people = t.census](PopId id) { return !is_born(id) && founder_slot_of(id) < people->people().size() && people->people()[founder_slot_of(id)].age <= 17; });
  expectations->set_external_search(true);
  expectations->add_listener(police.get());
  // Spec 65: the force's searches are the officers on duty and present over the officers one takes; a shift change is a wake.
  police->set_staffing([staff, exp = expectations.get()](const World& world) { return staff->capacity(world.clock.tick, exp); },
                       [staff](const World&, std::int64_t now) { return staff->next_change(now); });
  t.police = police.get();
  // Rule 1: cold systems after the hot ones.
  w.add_system(std::move(pops));
  w.add_system(std::move(econ));
  w.add_system(std::move(orders));
  w.add_system(std::move(grids));
  w.add_system(std::move(where));
  w.add_system(std::move(calendar));
  w.add_system(std::move(expectations));
  w.add_system(std::move(missing));
  w.add_system(std::move(payroll));
  w.add_system(std::move(bonds));
  w.add_system(std::move(police));
}

}  // namespace zs::tier0
