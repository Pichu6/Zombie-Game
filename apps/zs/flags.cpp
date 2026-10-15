#include "flags.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace zs::flags {

namespace {

constexpr const char* kIds[kPatterns] = {"plain",  "bicolour_h", "bicolour_v", "tricolour_h", "tricolour_v", "nordic_cross", "cross",
                                         "saltire", "canton",    "triangle",   "disc",        "diagonal",    "stripes"};
constexpr const char* kLabels[kPatterns] = {"Plain",          "Two horizontal bands", "Two vertical bands", "Three horizontal bands",
                                            "Three vertical bands", "Nordic cross", "Centred cross", "Saltire", "Canton",
                                            "Hoist triangle", "Disc",             "Diagonal",           "Stripes"};

std::uint64_t hash(const std::string& s) {
  std::uint64_t h = 1469598103934665603ull;
  for (char c : s) h = (h ^ static_cast<unsigned char>(c)) * 1099511628211ull;
  h ^= h >> 31;
  h *= 0x9E3779B97F4A7C15ull;
  return h ^ (h >> 29);
}

}  // namespace

const char* pattern_id(Pattern p) { return kIds[static_cast<int>(p)]; }
const char* pattern_label(Pattern p) { return kLabels[static_cast<int>(p)]; }

Pattern pattern_of(const std::string& id) {
  for (int i = 0; i < kPatterns; ++i)
    if (id == kIds[i]) return static_cast<Pattern>(i);
  return Pattern::TricolourV;
}

const std::vector<std::uint32_t>& basic_colors() {
  // The colours most flags are made of (vexillology's usual tinctures and a few common shades).
  static const std::vector<std::uint32_t> c = {0xC8102E, 0xE4002B, 0x9E1B32, 0xFFFFFF, 0x000000, 0x003DA5, 0x002868, 0x5B92E5,
                                               0x009A44, 0x006A4E, 0x00A86B, 0xFCD116, 0xFFB81C, 0xFF8200, 0x7B3F00, 0x6A1B9A};
  return c;
}

Flag default_flag(const std::string& key) {
  const std::uint64_t h = hash(key);
  const auto& c = basic_colors();
  Flag f;
  static const Pattern common[] = {Pattern::TricolourV, Pattern::TricolourH, Pattern::TricolourH, Pattern::BicolourH,
                                   Pattern::NordicCross, Pattern::Canton, Pattern::Triangle, Pattern::Disc,
                                   Pattern::Diagonal, Pattern::Stripes, Pattern::Cross, Pattern::Saltire};
  f.pattern = common[h % 12];
  std::uint32_t a = c[(h >> 8) % c.size()], b = c[(h >> 16) % c.size()], d = c[(h >> 24) % c.size()];
  if (b == a) b = a == 0xFFFFFF ? 0x000000 : 0xFFFFFF;
  if (d == a || d == b) d = (b == 0xFFFFFF || a == 0xFFFFFF) ? 0xFCD116 : 0xFFFFFF;
  f.colors = {a, b, d};
  return f;
}

Flag read_flag(const toml::table& t, const std::string& key) {
  const toml::table* ft = t["flag"].as_table();
  if (!ft) return default_flag(key);
  Flag f;
  f.pattern = pattern_of((*ft)["pattern"].value_or(std::string("tricolour_v")));
  if (const toml::array* a = (*ft)["colors"].as_array())
    for (std::size_t i = 0; i < 3 && i < a->size(); ++i) f.colors[i] = parse_hex((*a)[i].value_or(std::string{}), f.colors[i]);
  f.image = (*ft)["image"].value_or(std::string{});
  return f;
}

void write_flag(toml::table& t, const Flag& f) {
  toml::table ft;
  ft.insert_or_assign("pattern", std::string(pattern_id(f.pattern)));
  toml::array a;
  for (std::uint32_t c : f.colors) a.push_back(hex(c));
  ft.insert_or_assign("colors", std::move(a));
  if (!f.image.empty()) ft.insert_or_assign("image", f.image);
  t.insert_or_assign("flag", std::move(ft));
}

std::vector<unsigned char> draw(const Flag& f, int w, int h) {
  std::vector<unsigned char> px(static_cast<std::size_t>(w) * h * 4, 255);
  auto put = [&](int x, int y, std::uint32_t c) {
    unsigned char* p = &px[(static_cast<std::size_t>(y) * w + x) * 4];
    p[0] = static_cast<unsigned char>(c >> 16), p[1] = static_cast<unsigned char>(c >> 8), p[2] = static_cast<unsigned char>(c), p[3] = 255;
  };
  const auto [a, b, c] = f.colors;
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x) {
      const double u = (x + 0.5) / w, v = (y + 0.5) / h;  // 0..1 across and down
      std::uint32_t col = a;
      switch (f.pattern) {
        case Pattern::Plain: col = a; break;
        case Pattern::BicolourH: col = v < 0.5 ? a : b; break;
        case Pattern::BicolourV: col = u < 0.5 ? a : b; break;
        case Pattern::TricolourH: col = v < 1.0 / 3 ? a : v < 2.0 / 3 ? b : c; break;
        case Pattern::TricolourV: col = u < 1.0 / 3 ? a : u < 2.0 / 3 ? b : c; break;
        case Pattern::NordicCross: {
          const double cx = 0.375, t = 0.12, t2 = 0.06;  // the cross set toward the hoist, with an inner cross
          const bool outer = std::abs(u - cx) < t / 2 * h / w * 1.5 || std::abs(v - 0.5) < t / 2 * 1.5;
          const bool inner = std::abs(u - cx) < t2 / 2 * h / w * 1.5 || std::abs(v - 0.5) < t2 / 2 * 1.5;
          col = inner ? c : outer ? b : a;
          break;
        }
        case Pattern::Cross: col = (std::abs(u - 0.5) < 0.1 * h / w || std::abs(v - 0.5) < 0.1) ? b : a; break;
        case Pattern::Saltire: {
          const double d1 = std::abs(v - u), d2 = std::abs(v - (1 - u));
          col = (d1 < 0.09 || d2 < 0.09) ? b : a;
          break;
        }
        case Pattern::Canton: col = (u < 0.45 && v < 0.5) ? b : ((static_cast<int>(v * 7)) % 2 ? c : a); break;
        case Pattern::Triangle: col = u < 0.45 * (1 - std::abs(2 * v - 1)) ? c : (v < 0.5 ? a : b); break;
        case Pattern::Disc: {
          const double dx = (u - 0.5) * w / h, dy = v - 0.5;
          col = dx * dx + dy * dy < 0.09 ? b : a;
          break;
        }
        case Pattern::Diagonal: col = v < 1 - u - 0.08 ? a : v > 1 - u + 0.08 ? c : b; break;
        case Pattern::Stripes: col = u < 0.4 && v < 0.55 ? c : ((static_cast<int>(v * 9)) % 2 ? b : a); break;
      }
      put(x, y, col);
    }
  // A thin dark edge, so a white flag reads on a white card.
  for (int x = 0; x < w; ++x) put(x, 0, 0x404040), put(x, h - 1, 0x404040);
  for (int y = 0; y < h; ++y) put(0, y, 0x404040), put(w - 1, y, 0x404040);
  return px;
}

std::string hex(std::uint32_t rgb) {
  char buf[8];
  std::snprintf(buf, sizeof buf, "#%06x", rgb & 0xFFFFFF);
  return buf;
}

std::uint32_t parse_hex(const std::string& s, std::uint32_t fallback) {
  unsigned v = 0;
  if (s.size() == 7 && s[0] == '#' && std::sscanf(s.c_str() + 1, "%x", &v) == 1) return v & 0xFFFFFF;
  return fallback;
}

}  // namespace zs::flags
