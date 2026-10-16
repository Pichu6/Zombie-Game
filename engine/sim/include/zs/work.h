// Spec 99: where the one world's census works or studies: the job and workplace each row holds in the town scaled by
// spec 98, and its commute by the town's streets. Set by zs::gen::make_world after the run is built (setup-derived from
// the seed, never saved); empty: nobody's workplace is placed (spec 54 rule 4, work done at home). docs/tier0/specs/99-*.
#pragma once
#include <string>
#include <vector>

namespace zs {

struct PopWork {
  std::string job;          // the job's label ("" for a pupil)
  std::string workplace;    // the workplace's name, "" for none
  std::string address;      // its address in the town, "" off the town's plots
  int commute_min = -1;     // one way by the town's streets; -1: drawn from the time-use survey (spec 54)
  bool placed = false;      // the pop goes somewhere to work or study
  bool school = false;      // a pupil
};
using WorkTable = std::vector<PopWork>;  // by census row

}  // namespace zs
