#include "zs/pop_params.h"

#include <set>
#include <stdexcept>

#include "toml.hpp"

namespace zs {

const PopParams& default_pop_params() {
  static const PopParams p;
  return p;
}

namespace {
void read_double(const toml::table& t, const char* key, double& out, std::set<std::string>& seen) {
  if (const auto* node = t.get(key)) {
    const auto v = node->value<double>();
    if (!v) throw std::runtime_error(std::string("pop params: '") + key + "' must be a number");
    out = *v;
    seen.insert(key);
  }
}
void check_keys(const toml::table& t, const std::set<std::string>& known, const std::string& where) {
  for (const auto& [k, v] : t)
    if (!known.count(std::string(k.str()))) throw std::runtime_error("pop params: unknown key '" + std::string(k.str()) + "' in " + where);
}
void read_mortality(const toml::table* t, Mortality& m, const std::string& where) {
  if (!t) return;
  std::set<std::string> seen;
  read_double(*t, "constant_hazard", m.constant_hazard, seen);
  read_double(*t, "gompertz_b", m.gompertz_b, seen);
  read_double(*t, "gompertz_c", m.gompertz_c, seen);
  check_keys(*t, {"constant_hazard", "gompertz_b", "gompertz_c"}, where);
}
}  // namespace

PopParams load_pop_params(const std::string& path) {
  const toml::table t = toml::parse_file(path);
  PopParams p;
  if (const toml::array* a = t["age_pyramid"].as_array()) {
    for (const auto& v : *a) {
      const auto x = v.value<double>();
      if (!x || *x < 0) throw std::runtime_error("pop params: age_pyramid must be non-negative numbers");
      p.age_pyramid.push_back(*x);
    }
    double sum = 0;
    for (double x : p.age_pyramid) sum += x;
    if (static_cast<int>(p.age_pyramid.size()) != kAgeBands) throw std::runtime_error("pop params: age_pyramid needs 20 five-year bands");
    if (sum < 0.99 || sum > 1.01) throw std::runtime_error("pop params: age_pyramid shares must add up to 1");
    for (double& x : p.age_pyramid) x /= sum;
  }
  std::set<std::string> seen;
  read_double(t, "male_share_at_birth", p.male_share_at_birth, seen);
  read_double(t, "founder_min_age", p.founder_min_age, seen);
  read_double(t, "founder_age_span", p.founder_age_span, seen);
  read_mortality(t["mortality_female"].as_table(), p.female, "[mortality_female]");
  read_mortality(t["mortality_male"].as_table(), p.male, "[mortality_male]");
  if (const toml::table* f = t["fertility"].as_table()) {
    std::set<std::string> s2;
    Fertility& x = p.fertility;
    read_double(*f, "childless_share", x.childless_share, s2);
    read_double(*f, "first_min_age", x.first_min_age, s2);
    read_double(*f, "first_mean_delay", x.first_mean_delay, s2);
    read_double(*f, "gap_min", x.gap_min, s2);
    read_double(*f, "gap_mean_extra", x.gap_mean_extra, s2);
    read_double(*f, "end_age", x.end_age, s2);
    check_keys(*f, {"childless_share", "first_min_age", "first_mean_delay", "gap_min", "gap_mean_extra", "end_age"}, "[fertility]");
  }
  check_keys(t, {"male_share_at_birth", "founder_min_age", "founder_age_span", "mortality_female", "mortality_male", "fertility", "age_pyramid"}, "the file");
  if (p.male_share_at_birth < 0 || p.male_share_at_birth > 1 || p.fertility.childless_share < 0 || p.fertility.childless_share > 1)
    throw std::runtime_error("pop params: a share is outside 0..1");
  for (const Mortality* m : {&p.female, &p.male})
    if (m->constant_hazard <= 0 || m->gompertz_b <= 0 || m->gompertz_c <= 0) throw std::runtime_error("pop params: hazards must be positive");
  if (p.fertility.first_mean_delay <= 0 || p.fertility.gap_mean_extra <= 0 || p.founder_age_span <= 0)
    throw std::runtime_error("pop params: means and spans must be positive");
  return p;
}

void write_pop_params(ArchiveWriter& w, const PopParams& p) {
  for (const Mortality* m : {&p.female, &p.male}) { w.f64(m->constant_hazard); w.f64(m->gompertz_b); w.f64(m->gompertz_c); }
  w.f64(p.male_share_at_birth); w.f64(p.founder_min_age); w.f64(p.founder_age_span);
  const Fertility& f = p.fertility;
  w.f64(f.childless_share); w.f64(f.first_min_age); w.f64(f.first_mean_delay); w.f64(f.gap_min); w.f64(f.gap_mean_extra); w.f64(f.end_age);
  w.u64(p.age_pyramid.size());  // registry version 4
  for (double x : p.age_pyramid) w.f64(x);
}

PopParams read_pop_params(ArchiveReader& r, std::uint32_t registry_version) {
  PopParams p;
  for (Mortality* m : {&p.female, &p.male}) { m->constant_hazard = r.f64(); m->gompertz_b = r.f64(); m->gompertz_c = r.f64(); }
  p.male_share_at_birth = r.f64(); p.founder_min_age = r.f64(); p.founder_age_span = r.f64();
  Fertility& f = p.fertility;
  f.childless_share = r.f64(); f.first_min_age = r.f64(); f.first_mean_delay = r.f64(); f.gap_min = r.f64(); f.gap_mean_extra = r.f64(); f.end_age = r.f64();
  if (registry_version >= 4) {  // older saves drew founders by the placeholder rule: an empty pyramid keeps that
    p.age_pyramid.resize(r.u64());
    for (double& x : p.age_pyramid) x = r.f64();
  }
  return p;
}

}  // namespace zs
