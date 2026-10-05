#include "zs/philox.h"

namespace zs {
namespace {

constexpr std::uint32_t kMul0 = 0xD2511F53u;
constexpr std::uint32_t kMul1 = 0xCD9E8D57u;
constexpr std::uint32_t kWeyl0 = 0x9E3779B9u;
constexpr std::uint32_t kWeyl1 = 0xBB67AE85u;

void mulhilo(std::uint32_t a, std::uint32_t b, std::uint32_t& hi, std::uint32_t& lo) {
  const std::uint64_t p = static_cast<std::uint64_t>(a) * b;
  hi = static_cast<std::uint32_t>(p >> 32);
  lo = static_cast<std::uint32_t>(p);
}

}  // namespace

PhiloxCounter philox4x32_10(PhiloxCounter c, PhiloxKey k) {
  for (int round = 0; round < 10; ++round) {
    if (round > 0) { k[0] += kWeyl0; k[1] += kWeyl1; }
    std::uint32_t hi0, lo0, hi1, lo1;
    mulhilo(kMul0, c[0], hi0, lo0);
    mulhilo(kMul1, c[2], hi1, lo1);
    c = {hi1 ^ c[1] ^ k[0], lo1, hi0 ^ c[3] ^ k[1], lo0};
  }
  return c;
}

}  // namespace zs
