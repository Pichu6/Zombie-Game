// Spec 90: country packs. A user pack of included default countries and custom ones loads through the same matching;
// a custom country's tables load through the resolver and its ISO-keyed tables follow its template.
#include <doctest.h>

#include <filesystem>
#include <fstream>
#include <set>

#include <sstream>
#include <string>
#include "zs/demographics.h"
#include "zs/gen/place.h"
#include "zs/gen/world_people.h"
#include "zs/nations.h"
#include "zs/tier0.h"

using namespace zs::gen;
namespace fs = std::filesystem;

namespace {

// A pack in a temporary folder: two default countries included and one custom country made from Argentina's profile.
fs::path make_pack(const std::string& name, bool include_all = false) {
  const fs::path dir = fs::temp_directory_path() / ("zs_pack_test_" + name);
  fs::remove_all(dir);
  fs::create_directories(dir);
  {
    std::ofstream o(dir / "pack.toml");
    o << "name = \"" << name << "\"\ninclude = [" << (include_all ? "\"all\"" : "\"ARG\", \"NOR\"") << "]\n";
  }
  // Argentina's profile as text, made a custom country: its id, name, status, population; custom, template, colour.
  std::ifstream in(zs::tier0::data_dir() + "/setup/nations/real/ARG.toml");
  std::stringstream ss;
  ss << in.rdbuf();
  std::string t = ss.str();
  auto line = [&](const std::string& key, const std::string& value) {
    const std::size_t at = t.find("\n" + key + " = ");
    REQUIRE(at != std::string::npos);
    const std::size_t end = t.find('\n', at + 1);
    t.replace(at + 1, end - at - 1, key + " = " + value);
  };
  line("id", "\"gondwana\"");
  line("name", "\"Gondwana\"\ncustom = true\ntemplate = \"ARG\"\ncolor = \"#a05030\"");
  line("status", "\"custom\"");
  line("population", "900000000");  // the most populous
  std::ofstream o(dir / "gondwana.toml");
  o << t;
  return dir;
}

}  // namespace

TEST_CASE("country packs: a user pack's profiles are its included and custom countries") {
  const std::string data = zs::tier0::data_dir();
  CHECK(load_pack_profiles(data, "").size() == load_real_profiles(data).size());  // no pack: the default one
  const fs::path dir = make_pack("small");
  const std::vector<RealProfile> p = load_pack_profiles(data, dir.string());
  REQUIRE(p.size() == 3);
  std::set<std::string> ids;
  for (const RealProfile& r : p) ids.insert(r.iso);
  CHECK(ids == std::set<std::string>{"ARG", "NOR", "custom/gondwana"});
  for (const RealProfile& r : p)
    if (r.iso == "custom/gondwana") CHECK(r.name == "Gondwana");
  CHECK(load_pack_profiles(data, make_pack("all", true).string()).size() == load_real_profiles(data).size() + 1);
}

TEST_CASE("country packs: a custom country's file and its template's tables") {
  const fs::path dir = make_pack("resolver");
  zs::set_custom_nations_dir(dir.string());
  const std::string data = zs::tier0::data_dir();
  CHECK(zs::nation_file(data, "custom/gondwana", "high") == dir.string() + "/gondwana.toml");
  CHECK(zs::nation_file(data, "ARG", "high") == data + "/setup/nations/real/ARG.toml");
  CHECK(zs::nation_file(data, "", "low") == data + "/setup/nations/base_low.toml");
  CHECK(zs::table_profile("custom/gondwana") == "ARG");
  CHECK(zs::table_profile("NOR") == "NOR");
  const zs::NationDemographics n = zs::load_nation(zs::nation_file(data, "custom/gondwana", "high"));  // a custom file loads
  CHECK(!n.sourced);
  zs::set_custom_nations_dir("");
}

TEST_CASE("country packs: a world made with a pack takes its countries" * doctest::skip(zs::World::kDebugBuild)) {
  const fs::path dir = make_pack("world", true);
  WorldSettings s;
  s.pack = "world";
  s.pack_dir = dir.string();
  const GeneratedWorld w = generate_world(1, s, 24);
  bool custom = false;
  for (const Country& c : w.people.countries) custom |= c.profile == "custom/gondwana" && c.name.find("Gondwana") != std::string::npos;
  CHECK(custom);
}
