// Name generators. docs/design/content-tools.md section 2.
#include "zs/gen/names.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>

#include "fields.h"

namespace zs::gen {
namespace {

const std::string& pick(const RankedList& l, const Keyed& r, std::uint64_t id, std::uint32_t f, std::uint32_t k = 0) {
  return l.items[r.weighted(id, f, l.weights, k)];
}

const std::string& pick_flat(const std::vector<std::string>& v, const Keyed& r, std::uint64_t id, std::uint32_t f,
                             std::uint32_t k = 0) {
  return v[r.below(id, f, v.size(), k)];
}

void replace_all(std::string& s, const std::string& from, const std::string& to) {
  for (std::size_t at = s.find(from); at != std::string::npos; at = s.find(from, at + to.size()))
    s.replace(at, from.size(), to);
}

bool ends_with(const std::string& s, const std::string& end) {
  return s.size() >= end.size() && s.compare(s.size() - end.size(), end.size(), end) == 0;
}

// Order-2 letter chain over the culture's place names ('^' start, '$' end).
using Chain = std::map<std::string, std::map<char, int>>;

Chain train(const std::vector<std::string>& words) {
  Chain chain;
  for (const std::string& w : words) {
    std::string s = "^^";
    for (const char ch : w)
      if (std::isalpha(static_cast<unsigned char>(ch))) s += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    s += '$';
    for (std::size_t i = 2; i < s.size(); ++i) ++chain[s.substr(i - 2, 2)][s[i]];
  }
  return chain;
}

std::string markov_root(const Culture& c, const Keyed& r, std::uint64_t id) {
  const Chain chain = train(c.places);
  std::set<std::string> known;
  for (const auto& p : c.places) {
    std::string low;
    for (const char ch : p)
      if (std::isalpha(static_cast<unsigned char>(ch))) low += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    known.insert(low);
  }
  for (std::uint32_t attempt = 0; attempt < 64; ++attempt) {
    std::string s = "^^";
    for (std::uint32_t step = 0; step < 14; ++step) {
      const auto it = chain.find(s.substr(s.size() - 2));
      if (it == chain.end()) break;
      std::vector<char> next;
      std::vector<double> w;
      for (const auto& [ch, n] : it->second) {
        next.push_back(ch);
        w.push_back(n);
      }
      const char ch = next[r.weighted(id, field::Markov, w, attempt * 16 + step)];
      if (ch == '$') break;
      s += ch;
    }
    std::string root = s.substr(2);
    if (root.size() < 4 || root.size() > 8 || known.count(root)) continue;
    root[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(root[0])));
    return root;
  }
  return c.places[r.below(id, field::Markov, c.places.size(), 9999)];
}

}  // namespace

std::string draw_given(const Culture& c, const Keyed& r, std::uint64_t id, char sex) {
  return pick(sex == 'f' ? c.given_f : c.given_m, r, id, field::Given);
}

std::string draw_family(const Culture& c, const Keyed& r, std::uint64_t id, std::uint32_t k) {
  return pick(c.family, r, id, field::Family, k);
}

PersonName person_name(const Culture& c, const Keyed& r, std::uint64_t id, char sex) {
  PersonName n;
  n.given = draw_given(c, r, id, sex);
  n.family = draw_family(c, r, id);
  if (c.surnames == Surnames::PaternalMaternal) {
    for (std::uint32_t k = 0; k < 8 && (n.family2.empty() || n.family2 == n.family); ++k)
      n.family2 = pick(c.family, r, id, field::Family2, k);
  }
  return n;
}

std::string family_form(const Culture& c, const std::string& family, char sex) {
  if (c.surnames != Surnames::GenderedSki || sex != 'f') return family;
  if (ends_with(family, "ski") || ends_with(family, "cki") || ends_with(family, "dzki"))
    return family.substr(0, family.size() - 1) + "a";
  return family;
}

std::string full_name(const Culture& c, const PersonName& n, char sex) {
  const std::string fam = family_form(c, n.family, sex);
  if (c.family_first) return fam + " " + n.given;
  if (!n.family2.empty()) return n.given + " " + fam + " " + n.family2;
  return n.given + " " + fam;
}

std::string street_root(const Culture& c, const Keyed& r, std::uint64_t id) {
  return pick(c.street_names, r, id, field::StreetRoot);
}

std::string street_name(const Culture& c, const Keyed& r, std::uint64_t id) {
  std::string s = c.street_format;
  replace_all(s, "{name}", street_root(c, r, id));
  replace_all(s, "{kind}", pick(c.street_kinds, r, id, field::StreetKind));
  return s;
}

std::string place_name(const Culture& c, const Keyed& r, std::uint64_t id, PlaceKind kind) {
  const auto& forms = kind == PlaceKind::Town ? c.town_forms : kind == PlaceKind::Province ? c.province_forms : c.country_forms;
  std::string s = pick_flat(forms, r, id, field::PlaceForm);
  replace_all(s, "{root}", markov_root(c, r, id));
  return s;
}

std::string business_name(const Catalog& cat, const BusinessType& type, const Culture& owner,
                          const Culture& town_culture, const std::string& town, const Keyed& r,
                          std::uint64_t id) {
  const auto extra = type.culture_forms.find(owner.id);
  const bool own = extra != type.culture_forms.end() && r.chance(id, field::BizCultureForm, 0.6);
  std::string s = pick_flat(own ? extra->second : type.forms, r, id, field::BizForm);
  const PersonName n = person_name(owner, r, id, r.chance(id, field::Sex, 0.5) ? 'f' : 'm');
  std::string fam2 = n.family;
  for (std::uint32_t k = 1; k < 8 && fam2 == n.family; ++k) fam2 = draw_family(owner, r, id, k);
  replace_all(s, "{family2}", fam2);
  replace_all(s, "{family}", n.family);
  replace_all(s, "{given}", n.given);
  replace_all(s, "{street}", street_root(town_culture, r, id));
  replace_all(s, "{word}", pick_flat(cat.words, r, id, field::BizWord));
  replace_all(s, "{town}", town);
  return s;
}

}  // namespace zs::gen
