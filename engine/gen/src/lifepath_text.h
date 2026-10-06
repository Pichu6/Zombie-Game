// Lifepath as text lines for the dossier.
#pragma once
#include <string>

#include "zs/gen/pops.h"

namespace zs::gen {
std::string timeline(const Pop& p, int year);
}  // namespace zs::gen
