// Spec 101: regional anomalies and wind. A month's rain and temperature depart from the climate by fields that are
// coherent in space (neighbours share a drought or a hard winter) and, for temperature, persist a little from month to
// month; the wind blows from the prevailing side at the cell's mean speed with daily gusts and turns. All are pure
// functions of (seed, place, time), like spec 36's weather: nothing to save. Calibrated to WorldClim 2.1 / CRU TS on
// Earth (research droughts-and-wind, DW). docs/tier0/specs/101-droughts-and-wind.md.
#pragma once
#include <cstdint>

#include "zs/fields.h"

namespace zs::fields {

// Rule 1: a coherent standard normal field on the sphere: smooth value noise on a lattice of `scale_km` (the corners
// keyed normals of (seed, field, lattice point, index)), normalised to about unit variance; `index` gives an independent
// field (a month).
double coherent_normal(std::uint64_t seed, std::uint32_t field, double lat_deg, double lon_deg, double scale_km, std::int64_t index);

// Rule 2 (DW-1, DW-2): the coefficient of variation of annual rain Earth shows at this mean (0.329 - 0.0278 ln mm).
double rain_cv_target(double rain_mm);
// The month's rain factor (mean 1, lognormal); its log spread chosen so the year's total has rain_cv_target.
double rain_month_factor(std::uint64_t seed, const Climate& c, std::int64_t month_abs);

// Rule 3 (DW-4, DW-5): the monthly temperature anomaly's standard deviation by latitude and month (winter larger), and
// the month's anomaly (coherent over about 1,600 km, lag-one persistence about 0.3).
double temp_anomaly_sd(double lat_deg, int month);
double temp_month_anomaly_c(std::uint64_t seed, const Climate& c, std::int64_t month_abs);

// Rule 4 (DW-6, DW-7): the prevailing wind's bearing (where it blows from, degrees clockwise from north) by latitude.
double prevailing_from_deg(double lat_deg);
// The wind at an hour: speed in m/s around the cell's mean (Weibull k = 2 a day, windier in winter and at midday over
// land) and the bearing it blows from (the prevailing one turned by a daily draw).
double wind_ms(std::uint64_t seed, std::uint64_t cell, const Climate& c, const TimeOfYear& t);
double wind_from_deg(std::uint64_t seed, std::uint64_t cell, const Climate& c, const TimeOfYear& t);

}  // namespace zs::fields
