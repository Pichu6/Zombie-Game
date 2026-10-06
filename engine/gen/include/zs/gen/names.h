// Name generators: people, streets, businesses, towns, provinces, countries.
// docs/design/content-tools.md section 2. All pure functions of (seed, id).
#pragma once
#include <cstdint>
#include <string>

#include "zs/gen/catalog.h"
#include "zs/gen/keyed.h"

namespace zs::gen {

enum class PlaceKind { Town, Province, Country };

struct PersonName {
  std::string given, family, family2;  // family2: maternal surname where the culture has one
};

std::string draw_given(const Culture& c, const Keyed& r, std::uint64_t id, char sex);
std::string draw_family(const Culture& c, const Keyed& r, std::uint64_t id, std::uint32_t k = 0);
PersonName person_name(const Culture& c, const Keyed& r, std::uint64_t id, char sex);
// Surname as written for this sex ("Kowalski" -> "Kowalska").
std::string family_form(const Culture& c, const std::string& family, char sex);
// "Given Family", "Family Given" or "Given Paternal Maternal", per the culture's own rules.
std::string full_name(const Culture& c, const PersonName& n, char sex);

std::string street_root(const Culture& c, const Keyed& r, std::uint64_t id);
std::string street_name(const Culture& c, const Keyed& r, std::uint64_t id);

// A new place name: an order-2 letter chain trained on the culture's real places, wrapped in one
// of the culture's forms for that kind. Never returns a training place verbatim (unless it gives up).
std::string place_name(const Culture& c, const Keyed& r, std::uint64_t id, PlaceKind kind);

// A business of `type` owned by someone of culture `owner`, in `town`.
std::string business_name(const Catalog& cat, const BusinessType& type, const Culture& owner,
                          const Culture& town_culture, const std::string& town, const Keyed& r,
                          std::uint64_t id);

}  // namespace zs::gen
