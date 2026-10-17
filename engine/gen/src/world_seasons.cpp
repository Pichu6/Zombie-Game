// Spec 100: monthly normals and rainy seasons.
#include "zs/gen/world_seasons.h"

#include <algorithm>
#include <cmath>

#include "zs/detmath.h"

namespace zs::gen {

namespace {
constexpr double kPi = 3.14159265358979323846;
// A ramp from 0 at a to 1 at b (a < b), or from 1 at a to 0 at b (a > b).
double ramp(double x, double a, double b) { return std::clamp((x - a) / (b - a), 0.0, 1.0); }
double band(double x, double a, double b, double c, double d) { return std::min(ramp(x, a, b), ramp(x, d, c)); }
// The month (0-based, at mid-month 0.5 ... 11.5) of the local midsummer rain peak; winter is half a year away.
constexpr double kSummerPeakNorth = 6.8;  // late July: the rain belt lags the sun (estimate)
}  // namespace

void fill_rain_seasons(const WorldGrid& g, WorldFields& f, const SeasonParams& p_in) {
  const SeasonParams& p = p_in;
  const std::size_t n = g.size();
  f.rain_kappa.assign(n, 0.0);
  f.rain_peak.assign(n, 0.0);
  const double spacing = std::sqrt(4 * kPi * g.radius_km() * g.radius_km() / static_cast<double>(n));
  for (CellId i = 0; i < n; ++i) {
    if (!f.land(i)) continue;
    const double s = g.sin_lat(i);
    // Latitudes compared through their sines (detmath, so every compiler draws the same seasons).
    const double lat = std::abs(s);
    auto L = [](double deg) { return detmath::sin(deg * kPi / 180); };
    const double e = east_index(f, i);  // 0 the west side, 1 the east side
    const double inland = static_cast<double>(f.sea_cells[i]) * spacing;
    // Rule 2: the drivers, positive for a summer peak.
    double c = 0;
    c += (p.tropic_equator + (p.tropic - p.tropic_equator) * ramp(lat, L(2), L(14))) * ramp(lat, L(28), L(20));  // the rain belt
    c -= p.mediterranean * band(lat, L(28), L(33), L(42), L(48)) * (1 - e);                                    // dry summers, west
    // A monsoon needs a wide continent heating against the sea (Asia's): its strength grows with the land's east-west width.
    const double width = f.west_km[i] + f.east_km[i];
    c += p.monsoon * band(lat, L(16), L(22), L(46), L(54)) * e * ramp(width, p.monsoon_width_km / 2, p.monsoon_width_km);                                                // wet summers, east (the East Asian monsoon reaches 50 degrees)
    c += p.continental * band(lat, L(38), L(45), L(70), L(78)) * (1 - detmath::exp(-inland / p.continental_km));
    c -= p.maritime * band(lat, L(38), L(45), L(62), L(68)) * (1 - e) * detmath::exp(-inland / p.maritime_km);
    c = std::clamp(c, -1.0, 1.0);
    f.rain_kappa[i] = p.kappa_max * std::abs(c);
    const double summer = s >= 0 ? kSummerPeakNorth : std::fmod(kSummerPeakNorth + 6, 12.0);
    f.rain_peak[i] = c >= 0 ? summer : std::fmod(summer + 6, 12.0);
  }
}

void fill_winds(const WorldGrid& g, WorldFields& f) {
  // DW-6: WorldClim's mean speed over land by 10-degree band from -60 (m/s).
  static const double land[15] = {7.30, 5.82, 3.51, 2.95, 2.19, 1.63, 1.73, 2.59, 2.92, 3.02, 3.27, 3.12, 3.27, 4.72, 3.86};
  const std::size_t n = g.size();
  f.wind_ms.assign(n, 0.0);
  const double spacing = std::sqrt(4 * kPi * g.radius_km() * g.radius_km() / static_cast<double>(n));
  for (CellId i = 0; i < n; ++i) {
    const double s = g.sin_lat(i);
    // The band by the sine's thresholds (detmath), interpolated between band centres.
    double b = 0;
    for (int k = 0; k < 15; ++k)
      if (s >= detmath::sin((-60.0 + 10 * k + 5) * kPi / 180)) b = k;
    const int k0 = static_cast<int>(b);
    const int k1 = std::min(14, k0 + 1);
    const double lo = detmath::sin((-60.0 + 10 * k0 + 5) * kPi / 180), hi = detmath::sin((-60.0 + 10 * k1 + 5) * kPi / 180);
    const double t = k1 == k0 ? 0.0 : std::clamp((s - lo) / (hi - lo), 0.0, 1.0);
    const double band = s < detmath::sin(-55.0 * kPi / 180) ? land[0] : land[k0] + t * (land[k1] - land[k0]);
    if (f.land(i)) {
      // Coasts and heights windier, interiors and lowlands calmer (estimates, about neutral on average).
      const double coast = detmath::exp(-static_cast<double>(f.sea_cells[i]) * spacing / 150.0);
      const double height = std::min(f.elevation_m[i], 4000.0) / 4000.0;
      f.wind_ms[i] = band * (0.85 + 0.35 * coast + 0.3 * height);
    } else {
      f.wind_ms[i] = std::max(5.0, 2.2 * band);  // estimate: the sea about twice the land's wind, never under 5 m/s
    }
  }
}

std::array<double, 12> rain_months_mm(const WorldFields& f, CellId i) {
  std::array<double, 12> out{};
  const double k = f.rain_kappa.empty() ? 0.0 : f.rain_kappa[i], peak = f.rain_peak.empty() ? 0.0 : f.rain_peak[i];
  double sum = 0;
  for (int m = 0; m < 12; ++m) sum += out[static_cast<std::size_t>(m)] = detmath::exp(k * detmath::cos(2 * kPi * (m + 0.5 - peak) / 12));
  for (double& x : out) x *= f.rain_mm[i] / sum;
  return out;
}

std::array<double, 12> temp_months_c(const WorldGrid& g, const WorldFields& f, CellId i) {
  std::array<double, 12> out{};
  const double coldest = g.sin_lat(i) >= 0 ? 0.65 : 6.65;  // spec 36: near 20 January in the north
  for (int m = 0; m < 12; ++m)
    out[static_cast<std::size_t>(m)] = f.temp_mean_c[i] - f.temp_range_c[i] / 2 * detmath::cos(2 * kPi * (m + 0.5 - coldest) / 12);
  return out;
}

std::string koppen_class(const std::array<double, 12>& P, const std::array<double, 12>& T, bool south) {
  double map = 0, mat = 0, thot = -1e9, tcold = 1e9, pdry = 1e9, psum = 0;
  double psdry = 1e9, pwdry = 1e9, pswet = 0, pwwet = 0;
  for (std::size_t m = 0; m < 12; ++m) {
    const bool summer = south ? (m <= 2 || m >= 9) : (m >= 3 && m <= 8);
    map += P[m], mat += T[m] / 12;
    thot = std::max(thot, T[m]), tcold = std::min(tcold, T[m]), pdry = std::min(pdry, P[m]);
    if (summer) psum += P[m], psdry = std::min(psdry, P[m]), pswet = std::max(pswet, P[m]);
    else pwdry = std::min(pwdry, P[m]), pwwet = std::max(pwwet, P[m]);
  }
  const double pwin = map - psum;
  if (thot < 10) return thot > 0 ? "ET" : "EF";
  const double pth = pwin >= 0.7 * map ? 2 * mat : psum >= 0.7 * map ? 2 * mat + 28 : 2 * mat + 14;
  if (map < 10 * pth) return map < 5 * pth ? "BW" : "BS";
  if (tcold >= 18) return pdry >= 60 ? "Af" : pdry >= 100 - map / 25 ? "Am" : "Aw";
  bool s = psdry < 40 && psdry < pwwet / 3, w = pwdry < pswet / 10;
  if (s && w) s = !(w = psum > pwin);
  return std::string(tcold > 0 ? "C" : "D") + (s ? "s" : w ? "w" : "f");
}

}  // namespace zs::gen
