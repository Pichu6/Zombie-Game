// Spec 84 rule 3: the key numbers of a generated world beside Earth's, for the setup's preview and `zs gen world --preview`.
#pragma once
#include <string>
#include <vector>

#include "zs/gen/place.h"

namespace zs::atlas {

struct PreviewNumber {
  std::string what;
  double value = 0, earth = 0;
  std::string unit;
  bool realistic = true;  // within its band around Earth's value
};

std::vector<PreviewNumber> key_numbers(const gen::GeneratedWorld& w);

}  // namespace zs::atlas
