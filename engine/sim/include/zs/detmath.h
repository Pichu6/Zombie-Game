// Deterministic transcendental functions built only from + - * / and sqrt, which IEEE 754
// rounds identically on every compiler. The simulation never calls <cmath> exp/log/sin/cos.
// Spec: docs/tier0/specs/01-quantity.md rule 5.
#pragma once

namespace zs::detmath {

double exp(double x);
double expm1(double x);   // exp(x) - 1, accurate near 0
double log(double x);     // natural log, x > 0
double log10(double x);
double log1p(double x);   // log(1 + x), accurate near 0
double sin(double x);
double cos(double x);
double sqrt(double x);    // forwards to the IEEE-exact square root
double pow(double base, double exponent);  // base > 0

inline constexpr double pi = 3.14159265358979323846;

}  // namespace zs::detmath
