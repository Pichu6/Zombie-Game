// Spec 73: places opened in play (decision 0077 rule 2: the street is made when someone looks). A town's plots are records
// (spec 71); asking for one opens it: its site map (spec 72) is made from the record and added to the one world's map as
// levels of its own (spec 70), its walls and doors minted then. Opening a place shows it; it changes nothing that happens.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "zs/world.h"

namespace zs {

class SiteMap;
class Assemblies;
class Actors;

// Where the places come from: the town's plots (set by the setup from the generator; never saved).
struct PlotSource {
  std::function<int()> count;                  // how many plots the town has
  std::function<std::string(int)> address;     // a plot's address
  std::function<std::string(int)> site_toml;   // a plot's site map; empty when it has its own place (a station, the block)
};

enum PlaceCommand : std::uint16_t {
  PlaceOpen = 0x1301,  // a = the plot (0-based)
};

// Why a place was not opened (PlaceRefused's `b`).
enum class PlaceRefusal : std::int64_t { NoSuchPlot = 1, AlreadyOpen = 2, OwnPlace = 3, DoesNotFit = 4, NoTown = 5 };

class Places : public System {
 public:
  Places(SiteMap& map, Assemblies& items, Actors& actors) : map_(map), items_(items), actors_(actors) {}
  const char* name() const override { return "places"; }
  void set_source(PlotSource source) { source_ = std::move(source); }
  const PlotSource& source() const { return source_; }
  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World&) override {}
  std::int64_t next_wake(const World&, std::int64_t) const override { return kNever; }
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  // Rule 2: a place opened in play takes levels from 1000 up, ten to a place.
  static constexpr int kFirstLevel = 1000;
  struct Open {
    int plot = 0;
    int z = 0;     // its ground floor's level
    int site = 0;  // its site number in the map
  };
  const std::vector<Open>& opened() const { return opened_; }
  const Open* find(int plot) const;

 private:
  SiteMap& map_;
  Assemblies& items_;
  Actors& actors_;
  PlotSource source_;  // never saved
  std::vector<Open> opened_;
};

}  // namespace zs
