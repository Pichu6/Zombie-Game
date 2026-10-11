// The simulation card (atlas design §5, spec 55 slice 6): a world opened in-process, its systems in update order with
// their next wake tick, its hash and save size, stepping and jumping by the hour or the day, and the cold-world check
// of spec 40 (a week jumped equals a week stepped, hash for hash) run here and now.
#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "zs/gen/place.h"

namespace zs::atlas {

struct SimState;
std::shared_ptr<SimState> make_sim();
// Draws the card for the selected world; Open builds that world in a thread. `auto_open` opens it without a click
// (for check shots), `auto_cold` runs the cold-world check without a click.
void draw_sim(SimState& st, std::uint64_t seed, const gen::WorldSettings& settings, const std::string& world_name,
              bool auto_open = false, bool auto_cold = false, bool auto_advance = false);
bool sim_busy(const SimState& st);

}  // namespace zs::atlas
