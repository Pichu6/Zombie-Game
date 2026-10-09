// Spec 52 slice 3: deposits. docs/tier0/specs/52-land-water-capacity.md rules 14-18.
#include "zs/gen/world_deposits.h"

#include <algorithm>

#include "zs/detmath.h"
#include "zs/gen/keyed.h"

namespace zs::gen {
namespace {

enum Field : std::uint32_t { kHas = 1, kSize = 2, kGrade = 3 };

// Rule 15 (estimates): odds of a deposit, the highest of the cell's settings.
double odds(Resource r, std::uint8_t s) {
  double p = 0;
  auto take = [&](std::uint8_t flag, double v) {
    if (s & flag) p = std::max(p, v);
  };
  switch (r) {
    case Resource::Coal: take(kForeland, 0.5); take(kRift, 0.2); take(kBasin, 0.15); break;
    case Resource::Oil:
    case Resource::Gas: take(kRift, 0.3); take(kForeland, 0.2); take(kBasin, 0.12); break;
    case Resource::IronOre: take(kCraton, 0.25); break;
    case Resource::CopperOre: take(kArc, 0.4); break;
    case Resource::Bauxite: take(kLaterite, 0.35); break;
  }
  return p;
}

// About normal: Keyed::bell is a mean of four uniforms in [-1, 1] with sd 0.2887.
double normal(const Keyed& r, std::uint64_t id, std::uint32_t field, std::uint32_t k) { return r.bell(id, field, k) / 0.28867513459481287; }

}  // namespace

const char* resource_name(Resource r) {
  switch (r) {
    case Resource::Coal: return "coal";
    case Resource::Oil: return "oil";
    case Resource::Gas: return "gas";
    case Resource::IronOre: return "iron ore";
    case Resource::CopperOre: return "copper ore";
    case Resource::Bauxite: return "bauxite";
  }
  return "?";
}

const char* resource_unit(Resource r) { return r == Resource::Oil ? "bbl" : r == Resource::Gas ? "m3" : "t"; }

std::string setting_names(std::uint8_t s) {
  std::string out;
  const std::pair<std::uint8_t, const char*> names[] = {{kArc, "arc"},       {kForeland, "foreland"}, {kRift, "rift"},
                                                        {kCraton, "craton"}, {kBasin, "basin"},       {kLaterite, "laterite"}};
  for (const auto& [flag, name] : names)
    if (s & flag) out += (out.empty() ? "" : ", ") + std::string(name);
  return out;
}

// Rule 14.
std::uint8_t cell_settings(const WorldFields& f, CellId i) {
  const bool land = f.land(i), shelf = !land && f.elevation_m[i] >= -1500;
  const BoundaryKind k = f.boundary_kind[i];
  const std::uint32_t d = f.boundary_cells[i];
  std::uint8_t s = 0;
  if (land && (k == BoundaryKind::Subduction || k == BoundaryKind::Trench || k == BoundaryKind::IslandArc) && d <= 3) s |= kArc;
  if (land && k == BoundaryKind::Collision && d >= 2 && d <= 6) s |= kForeland;
  if ((land || shelf) && k == BoundaryKind::Rift && d <= 3) s |= kRift;
  if (land && d >= 8) s |= kCraton;
  if ((land && f.elevation_m[i] < 600 && d >= 3) || shelf) s |= kBasin;
  if (land && f.temp_mean_c[i] >= 20 && f.rain_mm[i] >= 1200 && f.elevation_m[i] < 800) s |= kLaterite;
  return s;
}

double WorldDeposits::total(Resource r) const {
  double t = 0;
  for (const Deposit& d : deposits)
    if (d.kind == r) t += d.amount;
  return t;
}

double WorldDeposits::metal(Resource r) const {
  double t = 0;
  for (const Deposit& d : deposits)
    if (d.kind == r) t += d.amount * d.grade;
  return t;
}

WorldDeposits generate_deposits(const WorldGrid& grid, const WorldFields& f, const DepositParams& p) {
  const Keyed r(mix64(p.seed, hash_str("deposits")));
  WorldDeposits w;
  w.settings.assign(grid.size(), 0);
  for (CellId i = 0; i < grid.size(); ++i) {
    const std::uint8_t s = cell_settings(f, i);
    w.settings[i] = s;
    if (!s) continue;
    for (int k = 0; k < resource_count; ++k) {
      const auto res = static_cast<Resource>(k);
      // Coal and the ores are worked on land; oil and gas also offshore, on the shelf (the North Sea).
      if (!f.land(i) && res != Resource::Oil && res != Resource::Gas) continue;
      const double p_has = odds(res, s);
      if (p_has <= 0 || !r.chance(i, kHas, p_has, static_cast<std::uint32_t>(k))) continue;
      Deposit d;
      d.cell = i;
      d.kind = res;
      d.settings = s;
      d.boundary_cells = f.boundary_cells[i];
      const double median = p.median[k] * grid.area_km2(i) / 88500.0;  // per area of a frequency-24 cell
      if (res == Resource::Oil || res == Resource::Gas) {
        // Pareto with exponent 1, capped (rule 16, WSR-30): median = 2 x the smallest.
        const double u = std::max(r.u01(i, kSize, static_cast<std::uint32_t>(k)), 1.0 / p.pareto_cap);
        d.amount = (median / 2.0) / u;
      } else {
        d.amount = median * detmath::exp(p.sigma * normal(r, i, kSize, static_cast<std::uint32_t>(k)));
      }
      switch (res) {  // grades, independent of size (WSR-6)
        case Resource::CopperOre: d.grade = 0.0039 * detmath::exp(0.4 * normal(r, i, kGrade, static_cast<std::uint32_t>(k))); break;
        case Resource::IronOre: d.grade = 0.30 + 0.34 * r.u01(i, kGrade, static_cast<std::uint32_t>(k)); break;
        case Resource::Bauxite: d.grade = 0.40 + 0.15 * r.u01(i, kGrade, static_cast<std::uint32_t>(k)); break;
        default: d.grade = 0; break;
      }
      w.deposits.push_back(d);
    }
  }
  return w;
}

}  // namespace zs::gen
