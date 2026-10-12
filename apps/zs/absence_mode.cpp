// zs --absence (spec 59): a pop of the one world goes missing and the household reacts on the ladder of decision 0070:
// phone, ask around, report missing, search; then the pop comes back. Prints every step with the game time.
//   zs --absence [--seed N] [--pop SLOT] [--back-after-hours H] [--phoned-in]
//   zs --absence --real [--seed N] [--days D] [--rate-scale S]   the world's own disappearances (spec 60)
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "zs/clock.h"
#include "zs/expectations.h"
#include "zs/gen/place.h"
#include "zs/missing_persons.h"
#include "zs/tier0.h"
#include "zs/world.h"
#include "absence_mode.h"

namespace zs::atlas {

namespace {

std::string name_of(const tier0::Tier0& t, PopId id) {
  if (!is_born(id) && founder_slot_of(id) < t.census->people().size()) return t.census->people()[founder_slot_of(id)].name;
  return "someone";
}

// One step of the ladder in words; empty for events that are not part of it.
std::string line_of(const tier0::Tier0& t, const Event& e) {
  const auto n = [&](std::uint64_t id) { return name_of(t, id); };
  switch (e.type) {
    case EventType::AbsenceBegan: return n(e.subject) + (e.b ? " called in: away for a while (nothing will be noticed)" : " goes away");
    case EventType::CommitmentBroken: return n(e.subject) + " did not keep " + commitment_name(static_cast<CommitmentKind>(e.a));
    case EventType::HolderPhoned: return n(e.subject) + " phones " + n(e.a) + ": no answer";
    case EventType::HolderAskedAround: return n(e.subject) + " asks around about " + n(e.a) + " (" + std::to_string(e.b) + " people)";
    case EventType::MissingReported: return n(e.subject) + " is reported missing, by " + n(e.a);
    case EventType::SearchStarted: return "a search opens for " + n(e.subject);
    case EventType::AbsenceEnded: return n(e.subject) + " is back, after " + std::to_string(e.b / kTicksPerGameHour) + " h";
    case EventType::CaseClosed: return "the missing-person case for " + n(e.subject) + " is closed";
    case EventType::ShiftMissed: return n(e.subject) + " misses a shift without calling (" + std::to_string(e.a) + " in a row)";
    case EventType::Separated: return "the employer records " + n(e.subject) + " as having left the job (" + std::to_string(e.a) + " days without contact)";
    case EventType::DisciplinaryOpened: return "the employer opens a disciplinary process for " + n(e.subject);
    case EventType::ConductCaseOpened: return "a conduct case opens for " + n(e.subject) + " (" + std::to_string(e.a) + " shifts missed)";
    case EventType::ReturnedToWork:
      return n(e.subject) + " meets the employer: " + (e.a == 0 ? "forgiven" : e.a == 1 ? "a warning" : "a disciplinary process");
    case EventType::ExpectationRefused: return "refused";
    default: return "";
  }
}

}  // namespace

int absence_main(int argc, char** argv) {
  std::uint64_t seed = 1;
  long slot = -1;
  double back_after_h = 48;
  bool phoned_in = false, real = false;
  double days = 365, rate_scale = 1;
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--seed") && i + 1 < argc) seed = std::strtoull(argv[++i], nullptr, 10);
    else if (!std::strcmp(argv[i], "--pop") && i + 1 < argc) slot = std::strtol(argv[++i], nullptr, 10);
    else if (!std::strcmp(argv[i], "--back-after-hours") && i + 1 < argc) back_after_h = std::atof(argv[++i]);
    else if (!std::strcmp(argv[i], "--phoned-in")) phoned_in = true;
    else if (!std::strcmp(argv[i], "--real")) real = true;
    else if (!std::strcmp(argv[i], "--days") && i + 1 < argc) days = std::atof(argv[++i]);
    else if (!std::strcmp(argv[i], "--rate-scale") && i + 1 < argc) rate_scale = std::atof(argv[++i]);
  }
  try {
    auto t = zs::gen::make_world(seed);
    t.world->check_every_tick = false;
    const auto& people = t.census->people();
    if (real) {  // spec 60: nobody is sent away; the world's own disappearances, at the table's rates times the scale
      t.missing->set_rate_scale(rate_scale);
      t.missing->redraw(*t.world, 0);
      std::printf("the one world, seed %llu: real disappearances for %.0f days at %.0f times the table's rate (%zu pops tracked)\n",
                  static_cast<unsigned long long>(seed), days, rate_scale, t.missing->entries().size());
      t.world->jump(static_cast<std::int64_t>(days * static_cast<double>(kTicksPerGameDay)));
      int began = 0, ended = 0, broken = 0, reported = 0;
      for (const Event& e : t.world->log.all()) {
        const std::string s = line_of(t, e);
        if (s.empty()) continue;
        began += e.type == EventType::AbsenceBegan;
        ended += e.type == EventType::AbsenceEnded;
        broken += e.type == EventType::CommitmentBroken;
        reported += e.type == EventType::MissingReported;
        std::printf("  %s  %s\n", tier0::time_text(e.tick).c_str(), s.c_str());
      }
      std::printf("%d went missing, %d came back, %d commitments broken, %d reported missing\n", began, ended, broken, reported);
      return 0;
    }
    if (slot < 0) {  // the first living adult with another living adult in the household
      for (std::size_t i = 0; i < people.size() && slot < 0; ++i) {
        if (is_dead(people[i].fate) || people[i].age < 18 || people[i].household.empty()) continue;
        for (std::size_t j = 0; j < people.size(); ++j)
          if (j != i && people[j].household == people[i].household && !is_dead(people[j].fate) && people[j].age >= 16) slot = static_cast<long>(i);
      }
    }
    if (slot < 0 || static_cast<std::size_t>(slot) >= people.size()) {
      std::printf("no pop to send away (give --pop SLOT)\n");
      return 1;
    }
    const PopId pop = census_pop_id(static_cast<std::uint32_t>(slot));
    std::printf("the one world, seed %llu: %s (slot %ld) goes away at %s%s; back after %.0f h\n", static_cast<unsigned long long>(seed),
                people[static_cast<std::size_t>(slot)].name.c_str(), slot, tier0::time_text(0).c_str(), phoned_in ? ", having phoned in" : "", back_after_h);
    t.world->submit({0, AbsenceBegin, pop, static_cast<std::int64_t>(AbsenceCause::Possessed), phoned_in ? 1 : 0});
    std::size_t shown = 0;
    const auto show = [&] {
      for (; shown < t.world->log.all().size(); ++shown) {
        const Event& e = t.world->log.all()[shown];
        const std::string s = line_of(t, e);
        if (!s.empty()) std::printf("  %s  %s\n", tier0::time_text(e.tick).c_str(), s.c_str());
      }
    };
    const std::int64_t back = static_cast<std::int64_t>(back_after_h * static_cast<double>(kTicksPerGameHour));
    t.world->jump(back);
    show();
    t.world->submit({0, AbsenceEnd, pop, 0, 0});
    t.world->run_ticks(1);
    show();
    return 0;
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "%s\n", ex.what());
    return 1;
  }
}

}  // namespace zs::atlas
