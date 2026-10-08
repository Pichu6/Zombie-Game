// The pop creator. docs/design/content-tools.md section 4.
#include "zs/gen/pops.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <sstream>
#include <stdexcept>

#include "fields.h"
#include "toml.hpp"
#include "zs/gen/keyed.h"
#include "zs/gen/names.h"
#include "lifepath_text.h"

namespace zs::gen {
namespace {

constexpr int kMaxAge = 92;

std::uint64_t household_id(int h) { return mix64(tag::Household, static_cast<std::uint64_t>(h)); }
std::uint64_t person_id(int h, int m) { return mix64(tag::Person, (static_cast<std::uint64_t>(h) << 8) | static_cast<std::uint64_t>(m)); }

std::vector<double> shares(const Mix& mix) {
  std::vector<double> w;
  for (const Group& g : mix.groups) w.push_back(g.share);
  return w;
}

std::string spelled(int n) {
  static const char* kWords[] = {"no", "one", "two", "three", "four", "five", "six", "seven"};
  return n >= 0 && n < 8 ? kWords[n] : std::to_string(n);
}

const char* education_label(const std::string& e) {
  static const char* kLabels[] = {"no diploma", "high school", "trade school", "college degree", "master's degree", "doctorate"};
  return kLabels[education_level(e)];
}

std::string capitalised(std::string s) {
  if (!s.empty()) s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
  return s;
}

struct Builder {
  const Catalog& cat;
  const Mix& mix;
  const Keyed r;
  Town town;
  const Culture* host = nullptr;
  std::vector<std::size_t> businesses_of(const std::string& type) const {
    std::vector<std::size_t> out;
    for (std::size_t i = 0; i < town.businesses.size(); ++i)
      if (town.businesses[i].type == type || town.businesses[i].type.ends_with("_" + type)) out.push_back(i);
    return out;
  }

  void make_town() {
    host = &cat.culture(mix.town_culture);
    town.seed = r.seed();
    town.culture = host->id;
    town.name = place_name(*host, r, mix64(tag::Town, 0), PlaceKind::Town);
    std::set<std::string> seen;
    for (std::uint32_t k = 0; static_cast<int>(town.streets.size()) < mix.streets && k < 256; ++k) {
      const std::string s = street_name(*host, r, mix64(tag::Street, k));
      if (seen.insert(street_root(*host, r, mix64(tag::Street, k))).second) town.streets.push_back(s);
    }
    const auto w = shares(mix);
    std::set<std::string> names;
    for (std::size_t t = 0; t < cat.businesses.size(); ++t) {
      const BusinessType& type = cat.businesses[t];
      for (int k = 0; k < type.count; ++k) {
        Business b;
        b.type = type.id;
        for (std::uint32_t tries = 0; tries < 16; ++tries) {
          const std::uint64_t id = mix64(tag::Business, (t << 16) | (static_cast<std::uint64_t>(k) << 8) | tries);
          b.owner_culture = mix.groups[r.weighted(id, field::BizOwner, w)].culture;
          b.name = business_name(cat, type, cat.culture(b.owner_culture), *host, town.name, r, id);
          if (!names.count(b.name)) break;
        }
        names.insert(b.name);
        town.businesses.push_back(std::move(b));
      }
    }
  }

  std::string business_at(const std::string& type, std::uint64_t id, std::uint32_t f) const {
    const auto list = businesses_of(type);
    return list.empty() ? std::string{} : town.businesses[list[r.below(id, f, list.size())]].name;
  }

  const Group& group_of(const std::string& culture) const {
    for (const Group& g : mix.groups)
      if (g.culture == culture) return g;
    throw std::runtime_error("gen: culture not in mix " + culture);
  }

  // Founding adult: their own culture, abroad or not, appearance from the culture.
  void founder(Pop& p, const Group& g, bool abroad) {
    const Culture& c = cat.culture(g.culture);
    p.culture = c.id;
    p.culture_label = c.country == host->country ? c.label : c.label + " heritage";
    p.born_abroad = abroad && c.country != host->country;
    p.skin_tone = 1 + static_cast<int>(r.weighted(p.id, field::Skin, c.skin));
    p.hair = c.hair[r.weighted(p.id, field::Hair, c.hair_w)];
    p.eyes = c.eyes[r.weighted(p.id, field::Eyes, c.eyes_w)];
  }

  void inherit(Pop& child, const std::vector<const Pop*>& parents) {
    const Culture& c = cat.culture(child.culture);
    double tone = 0;
    for (const Pop* q : parents) tone += q->skin_tone;
    tone = tone / static_cast<double>(parents.size()) + r.bell(child.id, field::Skin) * 1.2;
    child.skin_tone = std::clamp(static_cast<int>(std::lround(tone)), 1, 10);
    const Pop& from = *parents[r.below(child.id, field::EyesFromParent, parents.size())];
    child.hair = from.hair == "gray" || from.hair == "white" ? c.hair[r.weighted(child.id, field::Hair, c.hair_w)] : from.hair;
    child.eyes = r.chance(child.id, field::Eyes, 0.8) ? from.eyes : c.eyes[r.weighted(child.id, field::Eyes, c.eyes_w)];
  }

  void name(Pop& p, const Pop* name_giver, const std::set<std::string>& taken) {
    const Culture& c = cat.culture(p.culture);
    const Group& g = group_of(p.culture);
    const bool host_given = !p.born_abroad && c.id != host->id && r.chance(p.id, field::HostGiven, g.host_given);
    const PersonName n = person_name(c, r, p.id, p.sex);
    const Culture& given_from = host_given ? *host : c;
    p.given = host_given ? draw_given(*host, r, p.id, p.sex) : n.given;
    for (std::uint32_t k = 1; k < 16 && taken.count(p.given); ++k)  // no two in a household share a given name
      p.given = (p.sex == 'f' ? given_from.given_f : given_from.given_m).items[r.below(p.id, field::Given, 20, k)];
    p.family = name_giver ? name_giver->family : n.family;
    if (c.surnames == Surnames::PaternalMaternal) p.family2 = n.family2;
    // In the town everyone is written in its order; the maternal surname stays with those born abroad.
    PersonName shown{p.given, p.family, p.born_abroad ? p.family2 : std::string{}};
    Culture order = c;
    order.family_first = host->family_first;
    p.full_name = full_name(order, shown, p.sex);
  }

  void origin(Pop& p, int household_years, const Pop* parent, const std::string& before_city) {
    const Culture& c = cat.culture(p.culture);
    if (p.born_abroad) {
      p.birth_country = c.country;
      p.birthplace = c.places[r.below(p.id, field::Birthplace, c.places.size())];
      if (parent && parent->moved_at >= 0) p.moved_at = std::max(0, p.age - (parent->age - parent->moved_at));
      const int years_here = p.age - p.moved_at;
      const bool citizen = r.chance(p.id, field::Naturalized, std::min(0.9, years_here / 20.0));
      p.nationality = citizen ? host->nationality + " (naturalized)" : c.nationality;
    } else {
      p.birth_country = host->country;
      // Born in the town only if they have lived here all their life; children born before the
      // household moved here were born in the city it came from.
      const bool in_town = p.age <= household_years;
      const Culture& place_culture = c.country == host->country ? c : *host;
      p.birthplace = in_town ? town.name
                     : p.role == "child" ? before_city
                                         : place_culture.places[r.below(p.id, field::Birthplace, place_culture.places.size())];
      p.nationality = host->nationality;
    }
    p.years_in_town = std::min(p.age, household_years);
  }

  int years_cap(const Pop& p) const { return p.moved_at >= 0 ? p.age - p.moved_at : p.age; }

  void gray(Pop& p) {
    if (p.age < 40) return;
    const double chance = std::min(0.95, (p.age - 40) / 30.0);
    if (r.chance(p.id, field::Gray, chance)) p.hair = p.age >= 75 ? "white" : "gray";
  }

  const Job& draw_job(std::uint64_t id, std::uint32_t f, int age, std::uint32_t k = 0) const {
    std::vector<double> w;
    for (const Job& j : cat.jobs) {
      const int min_age = 16 + 2 * education_level(j.education) + (j.education == "doctorate" ? 4 : 0);
      w.push_back(age >= std::max(18, min_age) ? j.share : 0.0);
    }
    return cat.jobs[r.weighted(id, f, w, k)];
  }

  // ---- Spec 47: a country's tables. ----

  // Highest job education a country level allows: none and primary take unschooled jobs; secondary includes
  // vocational post-secondary (WIC folds ISCED 4 into upper secondary), so trade jobs; tertiary takes any job.
  static int max_job_level(int attainment) { return attainment <= 1 ? 0 : attainment == 2 ? 2 : 5; }

  bool eligible(const Job& j, int age, int attainment) const {
    const int level = education_level(j.education);
    const int min_age = 16 + 2 * level + (j.education == "doctorate" ? 4 : 0);
    return age >= std::max(18, min_age) && level <= max_job_level(attainment);
  }

  // Occupations depend on schooling: a group is open to a level if one of its jobs is. Each group's weight gets a
  // factor, fitted once (iterative proportional fitting) so that, over the workers' schooling (the 25-34 and 35-64
  // shares, 35:65), the groups come out in the country's shares.
  std::array<double, zs::kOccupationGroups> group_factor{};
  bool group_open(std::size_t g, int attainment) const {
    for (const Job& j : cat.jobs)
      if (static_cast<std::size_t>(j.group) == g && education_level(j.education) <= max_job_level(attainment)) return true;
    return false;
  }
  void fit_group_factors() {
    const zs::NationDemographics& n = *mix.nation;
    group_factor.fill(1.0);
    std::array<double, zs::kEducationLevels> workers{};
    for (std::size_t e = 0; e < workers.size(); ++e) workers[e] = 0.35 * n.education[0][e] + 0.65 * n.education[1][e];
    for (int it = 0; it < 200; ++it) {
      std::array<double, zs::kOccupationGroups> got{};
      for (std::size_t e = 0; e < workers.size(); ++e) {
        double total = 0;
        for (std::size_t g = 0; g < got.size(); ++g)
          if (group_open(g, static_cast<int>(e))) total += group_factor[g] * n.occupations[g];
        if (total <= 0) continue;
        for (std::size_t g = 0; g < got.size(); ++g)
          if (group_open(g, static_cast<int>(e))) got[g] += workers[e] * group_factor[g] * n.occupations[g] / total;
      }
      for (std::size_t g = 0; g < got.size(); ++g)
        if (got[g] > 0) group_factor[g] *= n.occupations[g] / got[g];
    }
  }

  // The occupation group by the fitted weights among the groups the pop can work in, then a job of that group by the
  // jobs' own shares.
  const Job& draw_country_job(std::uint64_t id, std::uint32_t f, int age, int attainment, std::uint32_t k = 0) const {
    const auto& occ = mix.nation->occupations;
    std::vector<double> gw(occ.size(), 0.0);
    for (const Job& j : cat.jobs)
      if (eligible(j, age, attainment)) {
        const auto g = static_cast<std::size_t>(j.group);
        gw[g] = group_factor[g] * occ[g];
      }
    const int group = static_cast<int>(r.weighted(id, field::OccupationGroup, gw, k));
    std::vector<double> w;
    for (const Job& j : cat.jobs) w.push_back(j.group == group && eligible(j, age, attainment) ? j.share : 0.0);
    return cat.jobs[r.weighted(id, f, w, k)];
  }

  static std::string country_education(int attainment, const Job* job) {
    if (attainment <= 1) return "none";
    if (attainment == 2) return job && job->education == "trade" ? "trade" : "high_school";
    return job && education_level(job->education) > 3 ? job->education : "bachelor";
  }

  void work_in_country(Pop& p) {
    const zs::NationDemographics& n = *mix.nation;
    const std::size_t edu_group = p.age < 35 ? 0 : p.age < 65 ? 1 : 2;  // 18-24 take the 25-34 shares
    const auto& edu = n.education[edu_group];
    p.attainment = static_cast<int>(r.weighted(p.id, field::Attainment, std::span<const double>(edu.data(), edu.size())));
    const std::size_t lab_group = p.age < 25 ? 0 : p.age < 55 ? 1 : p.age < 65 ? 2 : 3;
    const double participation = n.participation[lab_group][p.sex == 'f' ? 1 : 0];
    if (!r.chance(p.id, field::LabourForce, participation)) {
      if (p.age < 25 && p.attainment >= 2) {
        p.occupation = "college student";
        p.education = "high_school";
        p.workplace = business_at("college", p.id, field::School);
      } else if (p.age >= 55) {
        const Job& job = draw_country_job(p.id, field::Job, std::min(p.age, 60), p.attainment);
        p.occupation = "retired " + job.label;
        p.last_job_id = job.id;
        p.skills = job.skills;
        p.education = country_education(p.attainment, &job);
      } else {
        p.occupation = "keeps house";
        p.education = country_education(p.attainment, nullptr);
      }
      return;
    }
    const Job& job = draw_country_job(p.id, field::Job, p.age, p.attainment);
    p.education = country_education(p.attainment, &job);
    if (r.chance(p.id, field::Unemployed, n.unemployment_rate)) {
      p.occupation = "out of work";
      p.last_job_id = job.id;
      p.past_job = job.label;
      return;
    }
    employ(p, job, education_level(p.education));
  }

  void employ(Pop& p, const Job& job, int level) {
    p.job_id = job.id;
    p.occupation = job.label;
    p.skills = job.skills;
    p.workplace = job.workplace.empty() ? std::string{} : business_at(job.workplace, p.id, field::Workplace);
    const int start = std::max(18, 16 + 2 * level);
    p.years_in_job = r.range(p.id, field::YearsInJob, 1, std::max(1, std::min(p.age - start, 30)));
    if (p.age - p.years_in_job >= 25 && r.chance(p.id, field::PastJob, 0.5)) {
      const Job& past = draw_job(p.id, field::PastJob, p.age - p.years_in_job, 1);
      if (past.id != job.id) p.past_job = past.label;
    }
  }

  void work(Pop& p) {
    if (p.age < 5) {
      p.occupation = "small child";
      p.education = "none";
      return;
    }
    if (p.age < 18) {
      p.occupation = "student";
      p.education = "none";
      p.workplace = business_at(p.age < 14 ? "elementary_school" : "high_school", p.id, field::School);
      return;
    }
    if (mix.nation) return work_in_country(p);
    if (p.age <= 22 && r.chance(p.id, field::Work, 0.4)) {
      p.occupation = "college student";
      p.education = "high_school";
      p.workplace = business_at("college", p.id, field::School);
      return;
    }
    const Job& job = draw_job(p.id, field::Job, p.age);
    const int level = education_level(job.education);
    p.education = education_name(level + (r.chance(p.id, field::Education, 0.2) && level < 5 ? 1 : 0));
    const double retire = p.age >= 67 ? 0.85 : p.age >= 62 ? 0.35 : 0.0;
    const double u = r.u01(p.id, field::Work, 1);
    if (u < retire) {
      p.occupation = "retired " + job.label;
      p.last_job_id = job.id;
      p.skills = job.skills;
      return;
    }
    if (u < retire + 0.05) {
      p.occupation = "out of work";
      p.last_job_id = job.id;
      p.past_job = job.label;
      return;
    }
    employ(p, job, level);
  }

  // Spec 47: a size from the country's groups (1, 2-3, 4-5, 6+). Within 2-3 and 4-5 the upper size is taken with one
  // share q, set so the mean equals the published mean; 6+ is 6 to 9 evenly (mean 7.5, an estimate).
  int country_household_size(std::uint64_t hid) const {
    const auto& s = mix.nation->household_sizes;
    const double base = s[0] + 2 * s[1] + 4 * s[2] + 7.5 * s[3];
    const double mean = zs::mean_household_size(*mix.nation);
    const double q = s[1] + s[2] > 0 ? std::clamp((mean - base) / (s[1] + s[2]), 0.0, 1.0) : 0.0;
    const std::size_t g = r.weighted(hid, field::CountrySize, std::span<const double>(s.data(), s.size()));
    if (g == 0) return 1;
    if (g == 3) return 6 + static_cast<int>(r.below(hid, field::CountrySize, 4, 1));
    return (g == 1 ? 2 : 4) + (r.chance(hid, field::CountrySize, q, 2) ? 1 : 0);
  }

  void make_household(int h, int size) {
    const std::uint64_t hid = household_id(h);
    Household hh;
    hh.index = h;
    const int on_map = static_cast<int>(mix.on_map_homes.size());
    hh.home = h < on_map ? mix.on_map_homes[h] : "street_" + std::to_string(h - on_map + 1);
    hh.address = std::to_string(r.range(hid, field::Address, 1, 149) * 2 + (r.chance(hid, field::Address, 0.5, 1) ? 1 : 0) + 100) +
                 " " + town.streets[r.below(hid, field::AddressStreet, town.streets.size())];
    const Group& g = mix.groups[r.weighted(hid, field::Group, shares(mix))];
    hh.culture = g.culture;

    std::vector<Pop> people;
    people.reserve(static_cast<std::size_t>(size));
    Pop head;
    head.id = person_id(h, 0);
    head.household = h;
    head.role = "head";
    head.sex = r.chance(head.id, field::Sex, 0.5) ? 'f' : 'm';
    head.age = size == 1 ? r.range(head.id, field::Age, 19, 88)
               : size == 2 ? r.range(head.id, field::Age, 22, 85)
                           : r.range(head.id, field::Age, 25, 62);
    founder(head, g, r.chance(head.id, field::Abroad, g.born_abroad));
    people.push_back(head);

    const bool couple = size >= 2 && r.chance(hid, field::Couple, size == 2 ? 0.75 : 0.85);
    if (couple) {
      Pop partner;
      partner.id = person_id(h, 1);
      partner.household = h;
      partner.role = "partner";
      const bool same = r.chance(partner.id, field::PartnerSex, mix.same_sex_couples);
      partner.sex = same ? head.sex : (head.sex == 'm' ? 'f' : 'm');
      partner.age = std::clamp(head.age + static_cast<int>(std::lround(r.bell(partner.id, field::Age) * 8)), 19, kMaxAge);
      const bool mixed = r.chance(partner.id, field::PartnerCulture, mix.intermarriage);
      const Group& pg = mixed ? mix.groups[r.weighted(partner.id, field::Group, shares(mix))] : g;
      const bool abroad = pg.culture == g.culture && people[0].born_abroad ? r.chance(partner.id, field::Abroad, 0.85)
                                                                           : r.chance(partner.id, field::Abroad, pg.born_abroad);
      founder(partner, pg, abroad);
      people.push_back(partner);
    }
    // Moves: first-generation adults came at some age; the household has lived here for `years`.
    for (Pop& p : people)
      if (p.born_abroad) p.moved_at = r.range(p.id, field::MovedAt, std::min(p.age, 16), std::min(p.age - 1, 45));
    // An immigrant couple of one culture came together, or the partner followed within a few years.
    if (couple && people[0].born_abroad && people[1].born_abroad && people[0].culture == people[1].culture) {
      const int head_years = people[0].age - people[0].moved_at;
      const int years = head_years - r.range(people[1].id, field::MovedAt, 0, std::min(3, head_years - 1), 1);
      people[1].moved_at = std::clamp(people[1].age - years, 0, people[1].age - 1);
    }
    int cap = 40;
    for (const Pop& p : people) cap = std::min(cap, std::max(1, years_cap(p) - (p.moved_at >= 0 ? 0 : 18)));
    hh.years = r.range(hid, field::Years, 1, std::max(1, cap));
    {
      const Culture& hc = cat.culture(g.culture).country == host->country ? cat.culture(g.culture) : *host;
      hh.before_city = hc.places[r.below(hid, field::LifeCity, hc.places.size())];
    }

    // Children and (often) a partner take the family name of the father, or else of the head.
    const bool hetero = couple && people[0].sex != people[1].sex;
    const std::size_t giver_i = hetero && people[1].sex == 'm' ? 1 : 0;
    const int youngest_parent = couple ? std::min(people[0].age, people[1].age) : people[0].age;
    for (int m = static_cast<int>(people.size()); m < size; ++m) {
      Pop c;
      c.id = person_id(h, m);
      c.household = h;
      c.sex = r.chance(c.id, field::Sex, 0.5) ? 'f' : 'm';
      // Children at home are mostly young; some adult children stay or come back.
      const int lo = std::max(0, youngest_parent - 45);
      const int hi = std::max(lo, std::min(youngest_parent - 18, r.chance(c.id, field::Age, 0.15, 1) ? 99 : 24));
      if (youngest_parent - 18 >= 0) {
        c.role = "child";
        c.age = r.range(c.id, field::Age, lo, hi);
        c.culture = people[giver_i].culture;
        c.culture_label = people[giver_i].culture_label;
        const Pop& elder = people[0];
        c.born_abroad = elder.born_abroad && elder.moved_at >= 0 && c.age > elder.age - elder.moved_at;
        std::vector<const Pop*> parents{&people[0]};
        if (couple) parents.push_back(&people[1]);
        inherit(c, parents);
      } else {
        c.role = "relative";
        c.age = r.range(c.id, field::Age, 18, kMaxAge);
        founder(c, g, people[0].born_abroad && r.chance(c.id, field::Abroad, 0.8));
        if (c.born_abroad) c.moved_at = r.range(c.id, field::MovedAt, std::min(c.age, 16), std::min(c.age - 1, 45));
      }
      people.push_back(c);
    }

    const Pop* father = hetero ? &people[giver_i] : nullptr;
    const Pop& giver = people[giver_i];
    std::set<std::string> taken;
    name(people[giver_i], nullptr, taken);
    taken.insert(people[giver_i].given);
    for (Pop& p : people) {
      if (&p == &giver) continue;
      const bool takes = p.role == "child" || (p.role == "relative" && r.chance(p.id, field::TakesName, 0.5)) || (p.role == "partner" && father && p.sex == 'f' &&
                                               r.chance(p.id, field::TakesName,
                                                        cat.culture(p.culture).surnames == Surnames::PaternalMaternal ? 0.2 : 0.65));
      name(p, takes ? &giver : nullptr, taken);
      taken.insert(p.given);
      if (p.role == "child" && cat.culture(p.culture).surnames == Surnames::PaternalMaternal) {
        const Pop* mother = hetero ? &people[1 - giver_i] : nullptr;
        if (mother) p.family2 = mother->family;
        if (p.born_abroad) p.full_name = p.given + " " + p.family + (p.family2.empty() ? "" : " " + p.family2);
      }
    }
    for (Pop& p : people) {
      origin(p, hh.years, p.role == "child" ? &people[0] : nullptr, hh.before_city);
      work(p);
      gray(p);
      p.body_seed = r.bits(p.id, field::Body);
    }
    for (Pop& p : people) {
      hh.members.push_back(town.pops.size());
      town.pops.push_back(std::move(p));
    }
    town.households.push_back(std::move(hh));
  }
};

std::string with_whom(const Town& t, const Pop& p) {
  const Household& hh = t.households[static_cast<std::size_t>(p.household)];
  int sons = 0, daughters = 0, others = 0;
  const Pop* partner = nullptr;
  for (const std::size_t i : hh.members) {
    const Pop& q = t.pops[i];
    if (q.id == p.id) continue;
    if ((p.role == "head" && q.role == "partner") || (p.role == "partner" && q.role == "head")) partner = &q;
    else if (q.role == "child" && (p.role == "head" || p.role == "partner")) (q.sex == 'f' ? daughters : sons)++;
    else if (p.role != "child" || q.role == "child" || q.role == "relative") ++others;
  }
  std::vector<std::string> parts;
  if (partner) parts.push_back(std::string(p.sex == partner->sex ? "partner" : partner->sex == 'f' ? "wife" : "husband") + " " + partner->given);
  if (sons + daughters > 0) {
    const int n = sons + daughters;
    parts.push_back(spelled(n) + (n == 1 ? (sons ? " son" : " daughter") : " children"));
  }
  if (p.role == "child") {
    int parents = 0;
    for (const std::size_t i : hh.members) parents += t.pops[i].role == "head" || t.pops[i].role == "partner";
    parts.push_back(parents > 1 ? "parents" : "a parent");
    if (others > 0) parts.push_back(spelled(others) + (others == 1 ? " sibling" : " siblings"));
  } else if (others > 0) {
    parts.push_back(spelled(others) + " other " + (others == 1 ? "relative" : "relatives"));
  }
  if (parts.empty()) return "Lives alone";
  std::string s = "Lives with ";
  for (std::size_t i = 0; i < parts.size(); ++i) s += (i == 0 ? "" : i + 1 == parts.size() ? " and " : ", ") + parts[i];
  return s;
}

}  // namespace

Mix load_mix(const std::string& path) {
  const toml::table t = toml::parse_file(path);
  Mix m;
  m.town_culture = t["town_culture"].value<std::string>().value();
  m.people = t["people"].value_or(60);
  if (const auto* a = t["on_map_homes"].as_array())
    for (const auto& e : *a) m.on_map_homes.push_back(e.value<std::string>().value());
  m.streets = t["streets"].value_or(4);
  if (const auto* a = t["household_sizes"].as_array())
    for (const auto& e : *a) m.household_sizes.push_back(e.value<double>().value());
  m.intermarriage = t["intermarriage"].value_or(0.0);
  m.same_sex_couples = t["same_sex_couples"].value_or(0.0);
  for (const auto& n : *t["group"].as_array()) {
    const auto& x = *n.as_table();
    m.groups.push_back({x["culture"].value<std::string>().value(), x["share"].value<double>().value(),
                        x["born_abroad"].value_or(0.0), x["host_given"].value_or(1.0)});
  }
  if (m.groups.empty() || m.household_sizes.empty()) throw std::runtime_error("gen: mix needs groups and household_sizes");
  if (const auto c = t["country"].value<std::string>()) set_country(m, *c, ZS_DATA_DIR);
  return m;
}

void set_country(Mix& mix, const std::string& country, const std::string& data_dir) {
  mix.country = country;
  mix.nation = std::make_shared<const zs::NationDemographics>(zs::load_nation(data_dir + "/setup/nations/" + country + ".toml"));
}

Town generate_town(const Catalog& cat, const Mix& mix, std::uint64_t seed) {
  Builder b{cat, mix, Keyed(seed), {}};
  if (mix.nation) b.fit_group_factors();  // spec 47
  b.make_town();
  for (int h = 0; static_cast<int>(b.town.pops.size()) < mix.people; ++h) {
    const std::uint64_t hid = household_id(h);
    int size = 0;
    if (mix.nation) {
      size = b.country_household_size(hid);
    } else {
      size = 1 + static_cast<int>(b.r.weighted(hid, field::Size, mix.household_sizes));
      if (size >= 5) size += static_cast<int>(b.r.below(hid, field::Size, 3, 1));
    }
    size = std::min(size, mix.people - static_cast<int>(b.town.pops.size()));
    b.make_household(h, size);
  }
  add_lifepaths(cat, b.town);  // last: backgrounds only name what already exists
  return b.town;
}

std::string dossier(const Town& t, const Pop& p, int year) {
  const Household& hh = t.households[static_cast<std::size_t>(p.household)];
  std::ostringstream o;
  o << p.full_name << ", " << p.age << ", " << (p.sex == 'f' ? "female" : "male") << " | " << p.nationality;
  if (p.culture != t.culture && !p.born_abroad) o << " (" << p.culture_label << ")";
  o << "\n  " << capitalised(p.occupation);
  if (!p.workplace.empty()) o << " at " << p.workplace;
  if (p.years_in_job > 0) o << " for " << p.years_in_job << (p.years_in_job == 1 ? " year" : " years");
  if (!p.past_job.empty()) o << (p.occupation == "out of work" ? "; last worked as " : "; before that, ") << p.past_job;
  o << ".";
  if (p.age >= 18) o << " Education: " << education_label(p.education) << ".";
  o << "\n  Born in " << p.birthplace << ", " << p.birth_country;
  if (p.moved_at >= 0) o << "; moved here at " << p.moved_at;
  o << ". " << with_whom(t, p) << " at " << hh.address << " (" << hh.home << "), for " << p.years_in_town
    << (p.years_in_town == 1 ? " year." : " years.");
  o << "\n  Skin " << p.skin_tone << "/10, " << p.hair << " hair, " << p.eyes << " eyes";
  if (!p.skills.empty()) {
    o << " | skills: ";
    for (std::size_t i = 0; i < p.skills.size(); ++i) o << (i ? ", " : "") << p.skills[i];
  }
  o << "\n";
  if (year > 0) o << timeline(p, year);
  return o.str();
}

std::string census_toml(const Town& t) {
  // Three survivor slots: adults 18-65, picked by seed (spec 22 rule 5).
  std::vector<std::size_t> adults;
  for (std::size_t i = 0; i < t.pops.size(); ++i)
    if (t.pops[i].age >= 18 && t.pops[i].age <= 65) adults.push_back(i);
  std::set<std::size_t> survivors;
  const Keyed r(t.seed);
  for (std::uint32_t k = 0; survivors.size() < 3 && k < 64 && !adults.empty(); ++k)
    survivors.insert(adults[r.below(mix64(tag::Town, 1), field::Survivor, adults.size(), k)]);

  std::ostringstream o;
  o << "# The block's neighbours (spec 22). Generated by zs_gen census --seed " << t.seed << " (town " << t.name << ").\n";
  o << "# home: on-map homes are named on the map; street_N are off-map homes.\n";
  o << "# survivor = true marks the found-survivor slots (spec 26).\n\n";
  for (std::size_t i = 0; i < t.pops.size(); ++i) {
    const Pop& p = t.pops[i];
    const Household& hh = t.households[static_cast<std::size_t>(p.household)];
    o << "[[person]]\nname = \"" << p.full_name << "\"\nage = " << p.age << "\nsex = \"" << p.sex << "\"\nhousehold = \"h"
      << hh.index + 1 << "\"\nhome = \"" << hh.home << "\"\n";
    if (!p.job_id.empty()) o << "job = \"" << p.job_id << "\"\n";
    o << "culture = \"" << p.culture << "\"\n";
    if (survivors.count(i)) o << "survivor = true\n";
    o << "\n";
  }
  return o.str();
}

}  // namespace zs::gen
