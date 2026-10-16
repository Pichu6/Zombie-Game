// Draw fields for the generators. Append only: renumbering changes every generated world.
#pragma once
#include <cstdint>

namespace zs::gen::field {
enum : std::uint32_t {
  Given = 1, Family = 2, Family2 = 3, StreetRoot = 4, StreetKind = 5, PlaceForm = 6, Markov = 7,
  BizForm = 8, BizWord = 9, BizOwner = 10, BizCultureForm = 11,
  Group = 20, Size = 21, Couple = 22, PartnerSex = 23, PartnerCulture = 24, Age = 25, Sex = 26,
  Abroad = 27, MovedAt = 28, Naturalized = 29, HostGiven = 30, TakesName = 31, Birthplace = 32,
  BornInTown = 33, Years = 34, Address = 35, AddressStreet = 36,
  Skin = 40, Hair = 41, Eyes = 42, Gray = 43, EyesFromParent = 44, Body = 45,
  Work = 50, Job = 51, Education = 52, YearsInJob = 53, PastJob = 54, Workplace = 55, School = 56,
  TownName = 60, Streets = 61, Survivor = 62,
  LifeCity = 70, LifeTerm = 71, LifeJob = 72, LifeGap = 73, LifeService = 74, LifeEvent = 75, LifeMarried = 76,
  LifeRetire = 77, LifeEmployer = 78,
  // Spec 47: draws from a country's tables.
  CountrySize = 80, Attainment = 81, LabourForce = 82, Unemployed = 83, OccupationGroup = 84,
  Development = 85,  // spec 48: a country's luck
  // Spec 52 slice 4: the country history, keyed by cell id and period.
  Conflict = 86, Opponent = 87, Win = 88, Secede = 89, CountryLuck = 90,
  // World fields (spec 31): keyed by plate index or cell id.
  PlateSeed = 100, PlateType = 101, PlateOmega = 102, PlateWeight = 103, PlateOffset = 104,
  PlateAge = 105, CellCost = 106, Rough = 107,
  Primacy = 113,  // spec 78 rule 2: a country's primacy, keyed by country index
  // Spec 31 rules 2-4 revised (2026-10-09): keyed points and noise channels, resolution-independent.
  PlatePoint = 108, NoiseCost = 109, NoiseElev = 110, NoiseWarp = 111, Hotspot = 112,
  // Spec 89: continental crust from cratons, island arcs as chains.
  CratonPoint = 114, CratonSize = 115, NoiseCraton = 116, NoiseArc = 117, NoiseIce = 118,
  // Spec 91: the founding and the recorded history, keyed by polity or cell and period.
  Divide = 119, Unite = 120, AllianceJoin = 121, AllianceSeed = 122,
  // Spec 96: the outage, keyed by plant or town.
  Outage = 123,
};
}  // namespace zs::gen::field

namespace zs::gen::tag {
enum : std::uint64_t {
  Household = 1, Person = 2, Business = 3, Street = 4, Town = 5,
};
}  // namespace zs::gen::tag
