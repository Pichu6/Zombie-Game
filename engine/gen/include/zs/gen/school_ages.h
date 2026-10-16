// School stages by country (UNESCO UIS official entry ages and lengths, data/setup/school/ages.toml written by
// tools/data/school_ages.py; research calendars-school-and-clock-hours, CK-209). The town has elementary schools for
// primary and high schools for secondary, lower and upper together (spec 99 amendment).
#pragma once
#include <string>

namespace zs::gen {

struct SchoolAges {
  int primary_entry = 6, primary_years = 6;  // the United States' (the default)
  int lower_entry = 12, lower_years = 3;
  int upper_entry = 15, upper_years = 3;
  int secondary_end() const { return upper_entry + upper_years; }  // the first age past school
};

// The profile's table (an ISO3 or a base country id such as "base_high"); else the income group's base; else the default.
SchoolAges load_school_ages(const std::string& data_dir, const std::string& profile, const std::string& income = "");

// "elementary_school", "high_school", or "" (before school, or past it).
const char* school_stage(const SchoolAges& a, int age);
// Primary's share of the school years (the split of the town's teachers between its two kinds of school).
double primary_share(const SchoolAges& a);

}  // namespace zs::gen
