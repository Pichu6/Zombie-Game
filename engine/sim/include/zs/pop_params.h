// The numbers behind pop lives: mortality, fertility, starting ages. Spec: docs/tier0/specs/28-pop-levels.md
// They are data, not code: loaded from a preset (data/setup/demography/), recorded in the save of every
// registry, and never read from compiled-in defaults after a world exists. So retuning a default later
// never changes the lives of pops in an old save ("a preset update never changes an old save").
// All values are placeholders until fitted to real tables (spec 28, Calibration).
#pragma once
#include <string>

#include "zs/archive.h"

namespace zs {

// Two independent hazards, so the death age is closed-form: a constant one (accidents) and a Gompertz one (ageing).
struct Mortality {
  double constant_hazard = 3.0e-4;  // per year
  double gompertz_b = 1.5e-5;       // per year at age 0
  double gompertz_c = 0.10;         // per year of age
  bool operator==(const Mortality&) const = default;
};

// About 1.8 births per woman who lives to the end age: some childless; first birth at a minimum age plus an
// exponential delay; then a minimum gap plus an exponential one; none from the end age; at most 15 (the id format).
struct Fertility {
  double childless_share = 0.20;
  double first_min_age = 18.0, first_mean_delay = 10.0;
  double gap_min = 3.0, gap_mean_extra = 9.0;
  double end_age = 45.0;
  bool operator==(const Fertility&) const = default;
};

struct PopParams {
  Mortality female{3.0e-4, 1.5e-5, 0.10};
  Mortality male{3.0e-4, 3.0e-5, 0.10};
  double male_share_at_birth = 0.512;       // about 105 boys per 100 girls; to be cited in the calibration pass
  double founder_min_age = 18.0;            // census people start between this age ...
  double founder_age_span = 47.0;           // ... and this many years above it (a placeholder for the census pyramid)
  Fertility fertility;
  const Mortality& mortality(bool male_pop) const { return male_pop ? male : female; }
  bool operator==(const PopParams&) const = default;
};

const PopParams& default_pop_params();                 // the compiled-in preset, used only to make new worlds
PopParams load_pop_params(const std::string& path);    // a TOML preset; unknown keys are an error, missing keys keep the default
void write_pop_params(ArchiveWriter& w, const PopParams& p);
PopParams read_pop_params(ArchiveReader& r);

}  // namespace zs
