// Spec 92: country flags and colours. A flag is a pattern in up to three colours, or an image file kept beside the
// country (a user's custom country, in its pack). A default country, read-only, gets a keyed parody flag: not its real
// one. Drawn to RGBA for the Atlas; the map colour is separate (a flag's colours are too alike for neighbours on a map).
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "toml.hpp"

namespace zs::flags {

enum class Pattern : std::uint8_t {
  Plain, BicolourH, BicolourV, TricolourH, TricolourV, NordicCross, Cross, Saltire, Canton, Triangle, Disc, Diagonal,
  Stripes
};
inline constexpr int kPatterns = 13;
const char* pattern_id(Pattern p);     // "tricolour_v"
const char* pattern_label(Pattern p);  // "Three vertical bands"
Pattern pattern_of(const std::string& id);

struct Flag {
  Pattern pattern = Pattern::TricolourV;
  std::array<std::uint32_t, 3> colors{0xC8102E, 0xFFFFFF, 0x003DA5};  // 0xRRGGBB
  std::string image;  // a file name beside the country's file; empty: the pattern
};

// The basic colours of flags (the swatches the editor offers before its full picker).
const std::vector<std::uint32_t>& basic_colors();

// A keyed parody flag for a default country (by its ISO code).
Flag default_flag(const std::string& key);
// A country file's [flag] table (spec 90's custom countries); absent: a keyed flag from its id.
Flag read_flag(const toml::table& country, const std::string& key);
void write_flag(toml::table& country, const Flag& f);

// The flag drawn at w x h (3:2 is usual), RGBA rows top first. An image flag is drawn by the caller.
std::vector<unsigned char> draw(const Flag& f, int w, int h);

std::string hex(std::uint32_t rgb);              // "#c8102e"
std::uint32_t parse_hex(const std::string& s, std::uint32_t fallback);

}  // namespace zs::flags
