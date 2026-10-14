// The one world (spec 41): the run's Tier 0 systems plus the pop registry, the economy, orders and the grid tree,
// with the census rows as the registry's founders. Spec: docs/tier0/specs/41-one-world.md
#include <functional>
#include <memory>
#include <stdexcept>

#include "zs/actions.h"
#include "zs/body.h"
#include "zs/bonds.h"
#include "zs/calendar.h"
#include "zs/demographics.h"
#include "zs/economy_system.h"
#include "zs/expectations.h"
#include "zs/missing_persons.h"
#include "zs/grid_system.h"
#include "zs/orders_system.h"
#include "zs/dead.h"
#include "zs/payroll.h"
#include "zs/police.h"
#include "zs/police_agency.h"
#include "zs/places.h"
#include "zs/players.h"
#include "zs/police_staff.h"
#include "zs/pop_ai.h"
#include "zs/pop_system.h"
#include "zs/run.h"
#include "zs/schedule.h"
#include "zs/site_map.h"
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
std::unique_ptr<PopSystem> make_pops(World& w, const Census& census, const Place& place, const AgencyPlan& agency) {
  const int officers = agency.sworn;
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
  for (int i = 0; i < officers; ++i) force.push_back({static_cast<std::uint16_t>(25 + (i * 7) % 31), officer_is_woman(agency, i) ? PopSex::Female : PopSex::Male});  // spec 69: SV-12
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
        // Spec 59 (data review part 2): the household expects every member home by bedtime, not only those who commute; a
        // child, a student or a retired pop is missed when bed time comes and they are not there (DRL-6: two thirds of
        // the people reported missing in the US are children).
        const bool bedtime = s.activity == DayActivity::Sleep && s.start_min >= 18 * 60 && !out.holders.empty();
        if (!s.commitment && !bedtime) continue;
        const bool back_home = s.activity == DayActivity::Commute || bedtime;
        const std::int64_t due = midnight + static_cast<std::int64_t>(back_home && !bedtime ? s.end_min : s.start_min) * (kTicksPerGameHour / 60);
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

// Spec 67: how the players reach bodies in the one world. A census row with a living body on the block is controlled
// there (the pop AI leaves it to the player); anyone else (an officer, a row with no body) gets a pool body.
class WorldHands : public Hands {
 public:
  WorldHands(const Census* census, Bodies* bodies, Actors* actors, PopAI* ai, const Run* run, PopSystem* pops,
             std::function<Cell(PopId)> arrival)
      : census_(census), bodies_(bodies), actors_(actors), ai_(ai), run_(run), pops_(pops), arrival_(std::move(arrival)) {}
  Found body_of(const World&, PopId pop) const override {
    const EntityId body = map_body(pop);
    if (body == 0) return {Where::Nowhere, 0};
    const Body& b = bodies_->get(body);
    if (!b.alive || b.spec.walker || b.away) return {Where::Unavailable, body};
    return {Where::OnMap, body};
  }
  void take(World&, EntityId body) override { ai_->possess(body); }
  void give_back(World&, EntityId body) override {
    if (ai_->possessed() == body) ai_->release();
  }
  EntityId embody(World& w, PopId pop, EventId cause) override { return pops_->embody_pop(w, pop, cause); }
  bool disembody(World& w, PopId pop, EventId cause) override { return pops_->disembody_pop(w, pop, cause); }
  std::int64_t dwell_ticks() const override { return pops_->dwell_ticks(); }
  // Spec 70: an officer comes in at his own station's door when the town's stations are loaded beside the block.
  // Spec 68: a pool body comes onto the block where the player starts (inside the main building, by the group), idle, and
  // leaves the block when the player leaves it: what it carries stays on the floor where it stood, and it is off the map
  // (away). Not the yard gate: during the outbreak the street there has walkers from the first minute (measured: dead in 47).
  bool pool_on_map() const override { return true; }
  int level_of(const World&, EntityId body) const override { return actors_->actor(body).pos.z; }  // spec 75
  void arrive(World& w, EntityId body, EventId cause) override {
    Actor& a = actors_->actor(body);
    const Cell at = arrival_(bodies_->get(body).pop);
    a.pos = at;
    a.action = Action{};
    a.queue.clear();
    a.heading = -1;
    a.noise_db = 0;
    Body& b = bodies_->get(body);
    b.away = false;
    w.emit(EventType::ArrivedOnMap, body, cause, static_cast<std::int64_t>(b.pop), pack_cell(at));
  }
  void leave(World& w, EntityId body, EventId cause) override {
    actors_->apply_command(w, {w.clock.tick, static_cast<std::uint16_t>(CommandType::Cancel), body, 0, 0}, cause);  // drops what is in the hands
    Actor& a = actors_->actor(body);
    const std::vector<EntityId> carried = actors_->carried(w, body);
    for (EntityId item : carried) w.locations.place(item, actors_->floor_at(a.pos));
    a.carried_kg = 0;
    a.noise_db = 0;
    Body& b = bodies_->get(body);
    b.away = true;
    w.emit(EventType::LeftMap, body, cause, static_cast<std::int64_t>(b.pop), static_cast<std::int64_t>(carried.size()));
  }
  // Spec 26: when the controlled member dies the run hands control to the next one; the player follows it.
  PopId successor(const World&, PopId dead) const override {
    const EntityId next = run_->controlled();
    if (next == 0 || next == map_body(dead)) return kNoPop;
    const auto person = census_->person_of(next);
    return person ? census_pop_id(static_cast<std::uint32_t>(*person)) : kNoPop;
  }

 private:
  EntityId map_body(PopId pop) const {
    if (is_born(pop) || founder_slot_of(pop) >= census_->people().size()) return 0;
    return census_->people()[founder_slot_of(pop)].body;
  }
  const Census* census_;
  Bodies* bodies_;
  Actors* actors_;
  PopAI* ai_;
  const Run* run_;
  PopSystem* pops_;
  std::function<Cell(PopId)> arrival_;
};

}  // namespace

void add_world_systems(Tier0& t, const Place& place, std::int64_t start_day) {
  World& w = *t.world;
  // Spec 65: the police force serving the town, sized by its population: officers are pops after the census's, with shifts.
  const ForceParams force_params = load_force_params(data_dir() + "/institutions/police/force.toml", place.profile, place.income);
  // Spec 69: its size from the BJS curve for the town's people, at the country's level; women by the class's share.
  const AgencyPlan agency = plan_agency(load_agency_params(data_dir() + "/services/police.toml", place.profile, place.income), place.town_people,
                                        force_params.min_officers);
  const int officers = agency.sworn;
  t.agency = agency;
  auto pops = make_pops(w, *t.census, place, agency);
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
  // Spec 82: each building of the block is a leaf of spec 35's tree: its node's demand starts at the building's load and
  // follows it by command; the block's grid failure is its substation's fault.
  {
    GridTree& tree = t.grids->tree();
    const GridId district = t.owners.district;
    const Grid& g = tree.grid(district);
    std::vector<NodeId> node_of(t.power->buildings().size(), 0);
    NodeId substation = 0;
    for (std::size_t i = 0; i < g.nodes.size(); ++i) {
      if (g.nodes[i].name == "substation") substation = GridTree::node_id(district, i);
      for (std::size_t b = 0; b < node_of.size(); ++b)
        if (g.nodes[i].name == t.power->buildings()[b].grid_node) node_of[b] = GridTree::node_id(district, i);
    }
    for (std::size_t b = 0; b < node_of.size(); ++b)
      if (node_of[b]) tree.node(node_of[b]).demand_w = std::llround(t.power->building_load_w(b));
    t.grids->start();
    t.power->set_grid_link(
        [node_of](World& world, std::size_t b, double load) {
          if (b < node_of.size() && node_of[b]) world.submit({world.clock.tick, GridSetDemand, node_of[b], std::llround(load), 0});
        },
        [substation](World& world, EventId) {
          if (substation) world.submit({world.clock.tick, GridFault, substation, 0, 0});
        });
    // Rule 2: a building's mains are alive while its node is fed (connected to a running source, and given its demand).
    Power* power = t.power;
    GridSystem* grids = t.grids;
    grids->on_solved = [power, grids, node_of](World& world, EventId cause) {
      for (std::size_t b = 0; b < node_of.size(); ++b) {
        if (!node_of[b]) continue;
        const GridNode& n = grids->tree().node(node_of[b]);
        power->set_mains(world, b, n.sound && n.connected && n.served_w >= n.demand_w, cause);
      }
    };
  }
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
  for (int i = 0; i < officers; ++i) staff->add_officer(census_pop_id(static_cast<std::uint32_t>(census_size) + static_cast<std::uint32_t>(i)), 25 + (i * 7) % 31, officer_is_woman(agency, i) ? 'f' : 'm');
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
  // Spec 66: players and possession (decisions 0066, 0068): a possession is an absence like any other. Nobody plays until a
  // player joins; until then the system has nothing to look at and never wakes.
  const PossessionParams possession = load_possession_params(data_dir() + "/life/possession.toml");
  // Spec 67: the body pool (spec 29) for pops a player takes that have no body on the block.
  t.bodies->reserve_slots(w, static_cast<std::size_t>(possession.body_slots));
  t.pops->attach_bodies(w, *t.bodies);
  // Spec 68: each slot has an actor, so a body taken from the pool can walk and act on the block. A vacant slot is not alive:
  // no rule runs on it and nothing sees it. Bodies come in where the player starts.
  const std::optional<Cell> gate = t.map->find_object("pop_start", "player");
  if (!gate) throw std::runtime_error("one world: the map has no pop_start for the player (where pool bodies come in)");
  for (EntityId slot : t.bodies->slots()) {
    t.actors->add_actor(w, slot, *gate);
    t.bodies->get(slot).away = true;  // off the map until a player takes it
  }
  auto players = std::make_unique<Players>(
      possession, expectations.get(),
      [reg = &t.pops->registry()](PopId id, std::int64_t tick) {
        PopSnapshot s;
        const double year = year_of_tick(tick);
        return reg->snapshot(id, year, s) && s.base.birth_year <= year && year < s.base.death_year;
      });
  t.players = players.get();
  // Spec 70: an officer of the town's force arrives at his station's door (stations take the officers in agency order).
  const Cell player_start = *gate;
  const std::vector<Site> sites = t.sites;
  t.hands = std::make_shared<WorldHands>(t.census, t.bodies, t.actors, t.ai, t.run, t.pops,
                                         [player_start, sites, census_size, officers](PopId pop) {
                                           if (is_born(pop) || founder_slot_of(pop) < census_size) return player_start;
                                           const int k = static_cast<int>(founder_slot_of(pop) - census_size);
                                           if (k >= officers) return player_start;
                                           for (const Site& s : sites)
                                             if (s.kind == "police_station" && k >= s.first_officer && k < s.first_officer + s.officers) return s.door;
                                           return player_start;
                                         });
  players->set_hands(t.hands.get());
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
  w.add_system(std::move(players));
  // Spec 73: places opened in play. Last, so a world that never opens one saves its other systems as before.
  auto places = std::make_unique<Places>(*t.map, *t.items, *t.actors);
  t.places = places.get();
  w.add_system(std::move(places));
}

}  // namespace zs::tier0
