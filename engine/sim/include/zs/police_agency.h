// A town's police agency by its size (spec 69, decision 0077): sworn officers from the BJS size curve times the country,
// investigators, civilians, women among the officers, stations and holding cells. A pure function of the town's people and
// the data (data/services/police.toml); research services-by-settlement-size (SV).
#pragma once
#include <string>
#include <vector>

namespace zs {

struct SizeClass {
  double min_people = 0, max_people = 0;  // max 0: open
  double value = 0;
};

struct AgencyParams {
  std::vector<SizeClass> per_1000;  // sworn officers per 1,000 residents by size class (SV-5)
  std::vector<SizeClass> female;    // share of sworn who are women by size class (SV-12)
  double reference_per_100k = 217;  // the curve's own level (SV-21)
  double country_per_100k = 217;    // the country's police per 100,000; the reference where none is sourced
  double investigator_share = 0.15;
  int min_sworn_for_investigator = 7;
  double civilian_share = 0.22;
  int shifts = 3;
  double people_per_station = 100000;
  int officers_per_cell = 15;
};
// data/services/police.toml; `country_profile` (ISO3) picks the country's level when it has one.
// The country's police per 100,000 is its [country] entry, else its income group's [group] entry (data review part 2,
// DRL-4), else the reference.
AgencyParams load_agency_params(const std::string& path, const std::string& country_profile, const std::string& income = "");

// The value of the class `people` falls in (the last class if none matches).
double size_class_value(const std::vector<SizeClass>& classes, double people);

struct AgencyPlan {
  int sworn = 0;          // full-time sworn officers
  int investigators = 0;  // of the sworn
  int patrol = 0;         // the sworn who are not investigators
  int civilians = 0;
  int women = 0;          // of the sworn
  int stations = 1;
  int cells = 1;          // holding cells over all stations
  double per_1000 = 0;    // the class's rate after the country's level
};
// Rules 1-4. `min_sworn` is the smallest force that keeps every shift (spec 65).
AgencyPlan plan_agency(const AgencyParams& p, double people, int min_sworn = 1);

// Rule 5: whether the i-th sworn officer (0-based) of an agency is a woman: spread evenly so that any first n officers
// hold round(n x share) women, with no draw.
bool officer_is_woman(const AgencyPlan& plan, int index);

}  // namespace zs
