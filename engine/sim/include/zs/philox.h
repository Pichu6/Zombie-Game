// Philox-4x32-10 counter-based generator (Salmon et al. 2011, Random123).
#pragma once
#include <array>
#include <cstdint>

namespace zs {

using PhiloxCounter = std::array<std::uint32_t, 4>;
using PhiloxKey = std::array<std::uint32_t, 2>;

PhiloxCounter philox4x32_10(PhiloxCounter counter, PhiloxKey key);

}  // namespace zs
