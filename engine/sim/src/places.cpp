// Places opened in play. Spec: docs/tier0/specs/73-a-plot-opened-in-play.md (decision 0077 rule 2)
#include "zs/places.h"

#include <stdexcept>

#include "zs/actions.h"
#include "zs/site_map.h"

namespace zs {

const Places::Open* Places::find(int plot) const {
  for (const Open& o : opened_)
    if (o.plot == plot) return &o;
  return nullptr;
}

// Rule 1: a plot opens once; its site map is made from the town's record, added to the map from the next free level, its
// walls and doors minted now and its furniture given entities, so it can be walked, seen and used like the block.
void Places::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type != PlaceOpen) return;
  const int plot = static_cast<int>(c.a);
  const auto refuse = [&](PlaceRefusal why) { w.emit(EventType::PlaceRefused, static_cast<EntityId>(plot), cause, c.type, static_cast<std::int64_t>(why)); };
  if (!source_.count || !source_.site_toml) return refuse(PlaceRefusal::NoTown);
  if (plot < 0 || plot >= source_.count()) return refuse(PlaceRefusal::NoSuchPlot);
  if (find(plot)) return refuse(PlaceRefusal::AlreadyOpen);
  const std::string toml = source_.site_toml(plot);
  if (toml.empty()) return refuse(PlaceRefusal::OwnPlace);
  const int z = kFirstLevel + 10 * static_cast<int>(opened_.size());
  int site = 0;
  try {
    site = map_.add_site(w, items_, toml, z, true);
  } catch (const std::runtime_error&) {  // larger than the map's frame: nothing was changed
    return refuse(PlaceRefusal::DoesNotFit);
  }
  actors_.sync_with_map(w);
  opened_.push_back({plot, z, site});
  w.emit(EventType::PlaceOpened, static_cast<EntityId>(plot), cause, z, site);
}

void Places::save(ArchiveWriter& w) const {
  w.u64(opened_.size());
  for (const Open& o : opened_) {
    w.i64(o.plot);
    w.i64(o.z);
    w.i64(o.site);
  }
}

void Places::load(ArchiveReader& r) {
  opened_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    Open o;
    o.plot = static_cast<int>(r.i64());
    o.z = static_cast<int>(r.i64());
    o.site = static_cast<int>(r.i64());
    opened_.push_back(o);
  }
}

}  // namespace zs
