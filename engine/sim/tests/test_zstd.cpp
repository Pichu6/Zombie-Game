// The vendored zstd (spec 06 rule 17): a save round-trips at the levels the spec names, one frame with its checksum,
// and the same input gives the same bytes. The save format that uses it (version 5) is research fixes package 4.
#include <doctest.h>

#include <vector>

#include <zstd.h>

#include "zs/demo.h"

namespace {

std::vector<std::uint8_t> compress(const std::vector<std::uint8_t>& in, int level) {
  ZSTD_CCtx* c = ZSTD_createCCtx();
  ZSTD_CCtx_setParameter(c, ZSTD_c_compressionLevel, level);
  ZSTD_CCtx_setParameter(c, ZSTD_c_checksumFlag, 1);
  ZSTD_CCtx_setParameter(c, ZSTD_c_contentSizeFlag, 1);
  std::vector<std::uint8_t> out(ZSTD_compressBound(in.size()));
  const std::size_t n = ZSTD_compress2(c, out.data(), out.size(), in.data(), in.size());
  ZSTD_freeCCtx(c);
  REQUIRE(!ZSTD_isError(n));
  out.resize(n);
  return out;
}

}  // namespace

TEST_CASE("zstd: a save round-trips at levels 1, 3 and 19, smaller, with its content size, deterministic (A1)") {
  auto w = zs::demo::make_world(1);
  for (int i = 0; i < 2000; ++i) w->step();
  const std::vector<std::uint8_t> save = w->save();
  for (int level : {1, 3, 19}) {
    CAPTURE(level);
    const auto packed = compress(save, level);
    CHECK(packed.size() < save.size());
    CHECK(packed == compress(save, level));  // same library, same level, single-threaded: same bytes
    CHECK(ZSTD_getFrameContentSize(packed.data(), packed.size()) == save.size());
    std::vector<std::uint8_t> back(save.size());
    const std::size_t n = ZSTD_decompress(back.data(), back.size(), packed.data(), packed.size());
    REQUIRE(!ZSTD_isError(n));
    CHECK(n == save.size());
    CHECK(back == save);
  }
  // A corrupted frame is refused by its checksum.
  auto packed = compress(save, 3);
  packed[packed.size() / 2] ^= 0xFF;
  std::vector<std::uint8_t> back(save.size());
  CHECK(ZSTD_isError(ZSTD_decompress(back.data(), back.size(), packed.data(), packed.size())));
}
