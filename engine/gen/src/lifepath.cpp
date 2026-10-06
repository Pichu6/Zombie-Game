// Lifepaths: each pop's past as terms, built last and back from who they are today.
// docs/design/content-tools.md section 4.1.
#include <algorithm>
#include <map>
#include <set>
#include <sstream>

#include "fields.h"
#include "lifepath_text.h"
#include "zs/gen/keyed.h"
#include "zs/gen/pops.h"

namespace zs::gen {
namespace {

// Age schooling or study ends, by education level (none leaves at 16).
int study_end(int level) {
  static const int kEnd[] = {16, 18, 20, 22, 24, 28};
  return kEnd[std::clamp(level, 0, 5)];
}

struct Life {
  const Catalog& cat;
  Town& t;
  Keyed r;
  const Culture& host;

  std::string town_business(const std::string& type, std::uint64_t id, std::uint32_t k) const {
    std::vector<const Business*> list;
    for (const Business& b : t.businesses)
      if (b.type == type || b.type.ends_with("_" + type)) list.push_back(&b);
    return list.empty() ? std::string{} : list[r.below(id, field::LifeEmployer, list.size(), k)]->name;
  }

  // A real city in the town's country (never the town), for years lived elsewhere at home.
  std::string host_city(const Pop& p, std::uint32_t k) const {
    const Culture& c = cat.culture(p.culture).country == host.country ? cat.culture(p.culture) : host;
    return "in " + c.places[r.below(p.id, field::LifeCity, c.places.size(), k)] + ", " + host.country;
  }

  int arrival(const Pop& p) const { return p.age - p.years_in_town; }

  // Where the pop lived at age a: empty means in the town. Abroad until they emigrated; adults
  // grew up where they were born; otherwise the household's city before it moved here.
  std::string lived(const Pop& p, int a, const std::string& before_town) const {
    if (a >= arrival(p)) return {};
    if (p.born_abroad && a < p.moved_at) return "in " + p.birthplace + ", " + p.birth_country;
    if (!p.born_abroad && p.role != "child" && a < 18) return "in " + p.birthplace + ", " + p.birth_country;
    return before_town;
  }

  const Job* job_by_label(const std::string& label) const {
    for (const Job& j : cat.jobs)
      if (j.label == label) return &j;
    return nullptr;
  }

  std::string employer(const Pop& p, const Job& j, int a, const std::string& before_town, std::uint32_t k) const {
    const std::string away = lived(p, a, before_town);
    if (!away.empty()) return away;
    if (j.workplace.empty()) return "in " + t.name;
    return "at " + town_business(j.workplace, p.id, k);
  }

  void learn(Pop& p, const std::vector<std::string>& skills) const {
    for (const auto& s : skills)
      if (std::find(p.skills.begin(), p.skills.end(), s) == p.skills.end()) p.skills.push_back(s);
  }

  // Appends [from, to), split where the pop moved, so each piece names one place.
  void school(Pop& p, std::vector<Term>& out, int from, int to, const std::string& before_town) const {
    std::set<int> cuts{from, to, 14};
    if (p.moved_at > from && p.moved_at < to) cuts.insert(p.moved_at);
    if (arrival(p) > from && arrival(p) < to) cuts.insert(arrival(p));
    std::vector<int> c(cuts.begin(), cuts.end());
    for (std::size_t i = 0; i + 1 < c.size(); ++i) {
      if (c[i] < from || c[i + 1] > to) continue;
      const std::string away = lived(p, c[i], before_town);
      const bool high = c[i] >= 14;
      Term term{c[i], c[i + 1], high ? "high school student" : "school student",
                away.empty() ? "at " + town_business(high ? "high_school" : "elementary_school", p.id, 0) : away, {}};
      out.push_back(std::move(term));
    }
  }

  void build(Pop& p, const std::map<int, int>& married_years_ago) {
    std::vector<Term> life;
    if (p.age < 5) {
      p.lifepath = {Term{0, p.age, "small child", "at home", {}}};
      return;
    }
    const std::string before_town = "in " + t.households[static_cast<std::size_t>(p.household)].before_city + ", " + host.country;
    const int level = education_level(p.education);
    const bool student = p.occupation.ends_with("student");
    const bool worker = !p.job_id.empty();
    const int now = p.age;

    // School, then study.
    const int limit = worker ? now - 1 : now;  // a worker's present job lasts at least a year
    const int school_end = std::min(limit, level == 0 && !student ? 16 : 18);
    const bool in_school = p.age < 18;
    school(p, life, 5, in_school ? now + 1 : school_end, before_town);  // a child's last term reaches the present
    int at = school_end;
    if (p.occupation == "college student") {
      life.push_back(Term{at, now, "college student", "at " + town_business("college", p.id, 0), {}});
      at = now;
    } else if (level >= 2 && at < now) {
      int end = std::min(study_end(level), limit);
      if (end > at) {
        const std::string away = lived(p, at, before_town);
        const std::string where = level == 2 && away.empty() ? "at " + town_business("college", p.id, 0)
                                  : away.empty() ? host_city(p, 1) : away;
        life.push_back(Term{at, end, level == 2 ? "trade school" : "college", where, {}});
        at = end;
      }
    }
    // Military service, for some adults who did not go to university.
    if (!student && level <= 2 && now - at >= 6 &&
        r.chance(p.id, field::LifeService, cat.life.service_share * (p.sex == 'm' ? 1.7 : 0.3))) {
      const int len = std::min(limit - at, r.range(p.id, field::LifeService, cat.life.service_min, cat.life.service_max, 1));
      life.push_back(Term{at, at + len, "served in the Army", {}, {}});
      learn(p, {"firearms", "first_aid", "fitness"});
      at += len;
    }

    if (!student && at < now) {
      // The present: the current job, or retirement or a spell out of work after the last job.
      std::vector<Term> back;  // newest first
      int end = now;
      const Job* last = worker ? cat.job(p.job_id) : cat.job(p.last_job_id);
      if (!worker) {
        const bool retired = p.occupation.starts_with("retired");
        const int since = retired ? r.range(p.id, field::LifeRetire, std::max(at + 1, std::min(now, 60)), std::max(at + 1, std::min(now, 67)))
                                  : std::max(at + 1, now - r.range(p.id, field::LifeRetire, 1, 3));
        if (since < now) back.push_back(Term{since, now, retired ? "retired" : "out of work", {}, {}});
        end = since;
      }
      if (last && end > at) {
        int start = std::max(at, worker ? now - p.years_in_job : end - r.range(p.id, field::LifeTerm, cat.life.term_min, 20));
        if (worker && start < arrival(p) && end > arrival(p)) start = arrival(p);  // today's job started in town
        if (worker) p.years_in_job = now - start;
        const std::string where = worker && start >= arrival(p)
                                      ? (p.workplace.empty() ? "in " + t.name : "at " + p.workplace)
                                      : employer(p, *last, start, before_town, 0);
        back.push_back(Term{start, end, last->label, where, {}});
        learn(p, last->skills);
        end = start;
      }
      // Earlier jobs and gaps back to the end of study, cut where they moved.
      bool past_used = false;
      const Job* later = last;  // careers stick: an earlier term often has the same job
      for (std::uint32_t k = 1; end > at && k < 32; ++k) {
        int start = std::max(at, end - r.range(p.id, field::LifeTerm, cat.life.term_min, cat.life.term_max, k));
        for (const int cut : {p.moved_at, arrival(p)})
          if (cut > start && cut < end) start = cut;
        if (end - start < 1) break;
        const Job* j = nullptr;
        if (!past_used && !p.past_job.empty()) j = job_by_label(p.past_job);
        past_used = true;
        if (!j && later && r.chance(p.id, field::LifeJob, cat.life.same_job, 1000 + k)) j = later;
        if (!j && r.chance(p.id, field::LifeGap, cat.life.gap_chance, k)) {
          back.push_back(Term{start, end, "out of work", lived(p, start, before_town), {}});
        } else {
          if (!j) {
            std::vector<double> w;
            for (const Job& x : cat.jobs) w.push_back(education_level(x.education) <= level ? x.share : 0.0);
            j = &cat.jobs[r.weighted(p.id, field::LifeJob, w, k)];
          }
          back.push_back(Term{start, end, j->label, employer(p, *j, start, before_town, k), {}});
          if (end - start >= 3) learn(p, j->skills);  // a short stint leaves no lasting know-how
          later = j;
        }
        end = start;
      }
      if (end > at) back.back().from = at;  // ran out of draws: stretch the oldest term
      for (auto it = back.rbegin(); it != back.rend(); ++it) {
        if (!life.empty() && life.back().what == it->what && life.back().where == it->where) life.back().to = it->to;
        else life.push_back(*it);
      }
    }
    if (!life.empty()) life.back().to = now;
    if (worker && !life.empty()) p.years_in_job = life.back().to - life.back().from;

    // Events: moves, marriage, children, then drawn ones. Each goes in the term holding its age.
    auto add = [&](int a, std::string text) {
      for (Term& term : life)
        if (a >= term.from && (a < term.to || (&term == &life.back() && a <= term.to))) {
          term.events.emplace_back(a, std::move(text));
          return;
        }
    };
    if (p.moved_at > 0) add(p.moved_at, "came to " + host.country + " from " + p.birth_country);
    if (arrival(p) > 0) add(arrival(p), "moved to " + t.name);
    const Household& hh = t.households[static_cast<std::size_t>(p.household)];
    if (const auto it = married_years_ago.find(p.household); it != married_years_ago.end() && (p.role == "head" || p.role == "partner")) {
      for (const std::size_t i : hh.members) {
        const Pop& q = t.pops[i];
        if (q.id != p.id && (q.role == "head" || q.role == "partner")) add(now - it->second, "married " + q.given);
      }
    }
    if (p.role == "head" || p.role == "partner")
      for (const std::size_t i : hh.members) {
        const Pop& c = t.pops[i];
        if (c.role == "child") add(now - c.age, std::string(c.sex == 'f' ? "daughter " : "son ") + c.given + " born");
      }
    for (std::uint32_t ti = 0; ti < life.size(); ++ti) {
      Term& term = life[ti];
      const bool school_term = term.what.find("student") != std::string::npos;
      const bool work_term = !school_term && term.what != "out of work" && term.what != "retired" &&
                             term.what != "small child" && ti + 1 < life.size();  // past jobs only
      for (std::uint32_t e = 0; e < cat.life.events.size(); ++e) {
        const LifeEvent& ev = cat.life.events[e];
        if ((ev.kind == "work" && !work_term) || (ev.kind == "school" && !school_term)) continue;
        const int a = term.from + static_cast<int>(r.below(p.id, field::LifeEvent, static_cast<std::uint64_t>(std::max(1, term.to - term.from)), ti * 64 + e));
        if (a < ev.min_age || !r.chance(p.id, field::LifeEvent, ev.chance, 4096 + ti * 64 + e)) continue;
        add(a, ev.text);
        if (!ev.skill.empty()) learn(p, {ev.skill});
      }
    }
    for (Term& term : life) std::stable_sort(term.events.begin(), term.events.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    p.lifepath = std::move(life);
  }
};

}  // namespace

void add_lifepaths(const Catalog& cat, Town& t) {
  Life life{cat, t, Keyed(mix64(t.seed, tag::Person)), cat.culture(t.culture)};
  // Marriage: one date per couple, before their oldest child and after both turned 18.
  std::map<int, int> married_years_ago;
  for (const Household& hh : t.households) {
    std::vector<const Pop*> couple;
    int oldest_child = -1;
    for (const std::size_t i : hh.members) {
      const Pop& q = t.pops[i];
      if (q.role == "head" || q.role == "partner") couple.push_back(&q);
      if (q.role == "child") oldest_child = std::max(oldest_child, q.age);
    }
    if (couple.size() != 2) continue;
    int max_ago = std::min(couple[0]->age, couple[1]->age) - 18;
    // A couple of an immigrant and someone born here married after the immigrant came.
    if (couple[0]->born_abroad != couple[1]->born_abroad)
      for (const Pop* q : couple)
        if (q->born_abroad) max_ago = std::min(max_ago, q->age - q->moved_at);
    const int min_ago = std::min(max_ago, std::max(1, oldest_child + 1));
    const int top = oldest_child >= 0 ? std::min(max_ago, min_ago + 6) : max_ago;
    married_years_ago[hh.index] =
        life.r.range(mix64(tag::Household, static_cast<std::uint64_t>(hh.index)), field::LifeMarried, min_ago, std::max(min_ago, top));
  }
  for (Pop& p : t.pops) life.build(p, married_years_ago);
}

std::string timeline(const Pop& p, int year) {
  std::ostringstream o;
  for (const Term& term : p.lifepath) {
    const bool present = &term == &p.lifepath.back();
    o << "    " << year - p.age + term.from << "-" << (present ? std::string("now ") : std::to_string(year - p.age + term.to))
      << "  " << term.what;
    if (!term.where.empty()) o << " " << term.where;
    for (const auto& [a, text] : term.events) o << "; " << year - p.age + a << " " << text;
    o << "\n";
  }
  return o.str();
}

}  // namespace zs::gen
