#include "atlas_sim.h"

#include <imgui.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "zs/calendar.h"
#include "zs/clock.h"
#include "zs/economy_system.h"
#include "zs/grid_system.h"
#include "zs/orders_system.h"
#include "zs/pop_system.h"
#include "zs/tier0.h"
#include "zs/world.h"

namespace zs::atlas {

namespace {

struct ColdResult {
  std::int64_t ticks = 0;    // the week in ticks
  std::int64_t steps = 0;    // the ticks the jumping world actually stepped
  double step_s = 0, jump_s = 0;
  std::uint64_t hash_stepped = 0, hash_jumped = 0;
  std::size_t log_stepped = 0, log_jumped = 0;
  std::string error;
};

// The cold world of spec 40's test (engine/sim/tests/test_time.cpp): economy hourly, orders daily, grids and pops on
// commands only. Built twice: one steps a week, one jumps it.
std::unique_ptr<World> make_cold(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  auto pops = std::make_unique<PopSystem>(*w);
  pops->setup(*w).add_cohort({1, 0, 0, 50});
  auto econ = std::make_unique<EconomySystem>(*w);
  Owners& o = econ->owners();
  Goods& g = econ->goods();
  const OwnerId country = o.add(w->ledger, "country", Scope::Country);
  const OwnerId region = o.add(w->ledger, "region", Scope::Region, country);
  const OwnerId farm = o.add(w->ledger, "farm", Scope::Building, region);
  const OwnerId shop = o.add(w->ledger, "shop", Scope::Building, region);
  o.add(w->ledger, "ana", Scope::Pop, shop);
  const GoodId grain = g.add(w->ledger, "grain", "grain_g", "grain_sack", 50'000);
  const GoodId milk = g.add(w->ledger, "milk", "milk_ml", "milk_bottle", 1'000, 2.0);
  const GoodId flour = g.add(w->ledger, "flour", "flour_g", "flour_bag", 10'000);
  const RecipeId milling = econ->recipes().add(w->ledger, g, {"milling", grain, 100'000, flour, 95'000, {}, 2 * kTicksPerGameHour, 0});
  econ->establishments().add({farm, milling, 0, 4, 0});
  w->ledger.mint(farm, g.def(grain).bulk, 20'000'000);
  w->ledger.mint(farm, g.def(milk).bulk, 300'000);
  w->ledger.mint(country, econ->money().resource(), 1'000'000);
  w->ledger.mint(shop, econ->money().resource(), 50'000);
  econ->start_lots(*w);
  econ->start_establishments(*w);
  econ->start_populations(*w);
  auto orders = std::make_unique<OrdersSystem>(econ.get(), pops.get());
  orders->edicts().add_def({"milk levy 5%", EdictKind::Levy, region, country, milk, 50, 0, 0});
  auto grids = std::make_unique<GridSystem>();
  const GridId nation = grids->tree().add_grid("nation");
  grids->tree().add_node(nation, {"plant", NodeKind::Source, 1'000'000});
  const std::size_t load = grids->tree().add_node(nation, {"town", NodeKind::Load, 0, 400'000});
  grids->tree().add_edge(nation, 0, load);
  grids->start();
  w->add_system(std::move(pops));
  w->add_system(std::move(econ));
  w->add_system(std::move(orders));
  w->add_system(std::move(grids));
  w->close_setup();
  return w;
}

ColdResult run_cold(std::uint64_t seed) {
  ColdResult r;
  try {
    r.ticks = 7 * kTicksPerGameDay;
    auto stepped = make_cold(seed);
    auto t0 = std::chrono::steady_clock::now();
    stepped->run_ticks(r.ticks);
    r.step_s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    auto jumped = make_cold(seed);
    t0 = std::chrono::steady_clock::now();
    while (jumped->clock.tick < r.ticks) {
      const std::int64_t wake = std::min(jumped->next_wake(jumped->clock.tick), r.ticks);
      jumped->jump(wake);
      if (jumped->clock.tick < r.ticks) {
        jumped->step();
        ++r.steps;
      }
    }
    r.jump_s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    r.hash_stepped = stepped->state_hash();
    r.hash_jumped = jumped->state_hash();
    r.log_stepped = stepped->log.all().size();
    r.log_jumped = jumped->log.all().size();
  } catch (const std::exception& ex) {
    r.error = ex.what();
  }
  return r;
}

std::string hex(std::uint64_t h) {
  char buf[24];
  std::snprintf(buf, sizeof buf, "%016llx", static_cast<unsigned long long>(h));
  return buf;
}

std::string ticks_text(std::int64_t t) {
  if (t >= kTicksPerGameDay) return std::to_string(t / kTicksPerGameDay) + " d " + std::to_string((t % kTicksPerGameDay) / kTicksPerGameHour) + " h";
  if (t >= kTicksPerGameHour) return std::to_string(t / kTicksPerGameHour) + " h " + std::to_string((t % kTicksPerGameHour) * 30 / 60) + " min";
  return std::to_string(t) + " ticks";
}

}  // namespace

struct SimState {
  std::thread thread;
  std::atomic<bool> busy{false};
  std::string stage;
  std::mutex mutex;
  // The open world.
  std::optional<zs::tier0::Tier0> t;
  std::uint64_t seed = 0;
  gen::WorldSettings settings;
  std::string world_name;
  double open_s = 0;
  std::string error;
  // Readouts refreshed after every change (hashing a world costs a save).
  std::uint64_t hash = 0;
  std::size_t save_bytes = 0;
  double last_s = 0;
  std::string last_action;
  // What happened (spec 25): the player's event lines as the world moves, and each completed day's summary.
  std::vector<std::string> lines;
  std::size_t seen = 0;
  int days_summarised = 0;
  std::vector<std::string> days;
  // The cold check.
  std::optional<ColdResult> cold;
  bool cold_running = false;
  int auto_steps = 0;  // check shots: 2 = jump a day then step an hour

  void join() {
    if (thread.joinable()) thread.join();
  }
  ~SimState() { join(); }

  void refresh() {
    if (!t) return;
    hash = t->world->state_hash();
    save_bytes = t->world->save().size();
    const auto& all = t->world->log.all();
    for (; seen < all.size(); ++seen)
      if (auto line = zs::tier0::event_line(*t, all[seen])) lines.push_back(zs::tier0::time_text(all[seen].tick) + "  " + line->text);
    if (lines.size() > 400) lines.erase(lines.begin(), lines.end() - 400);
    const int day = static_cast<int>(t->world->clock.day());
    for (; days_summarised < day; ++days_summarised) {
      std::string s = "Day " + std::to_string(days_summarised + 1) + ": ";
      const auto notable = zs::tier0::day_summary(*t, days_summarised);
      if (notable.empty()) s += "quiet";
      for (std::size_t i = 0; i < notable.size(); ++i) s += (i ? "; " : "") + notable[i];
      days.push_back(s);
    }
  }

  void open(std::uint64_t s, const gen::WorldSettings& st, const std::string& name) {
    if (busy) return;
    join();
    t.reset();
    error.clear();
    cold.reset();
    lines.clear();
    days.clear();
    seen = 0;
    days_summarised = 0;
    seed = s;
    settings = st;
    world_name = name;
    busy = true;
    stage = "generating the planet and building the one world";
    thread = std::thread([this] {
      const auto t0 = std::chrono::steady_clock::now();
      try {
        auto built = zs::gen::make_world(seed, settings);
        built.world->check_every_tick = false;
        std::lock_guard<std::mutex> l(mutex);
        t.emplace(std::move(built));
        refresh();
      } catch (const std::exception& ex) {
        std::lock_guard<std::mutex> l(mutex);
        error = ex.what();
      }
      open_s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
      busy = false;
    });
  }

  // Steps or jumps the open world by `ticks` in the thread.
  void advance(std::int64_t ticks, bool by_jump) {
    if (busy || !t) return;
    join();
    busy = true;
    stage = by_jump ? "jumping" : "stepping";
    thread = std::thread([this, ticks, by_jump] {
      const auto t0 = std::chrono::steady_clock::now();
      {
        std::lock_guard<std::mutex> l(mutex);
        World& w = *t->world;
        if (by_jump) w.jump(w.clock.tick + ticks);
        else w.run_ticks(ticks);
        refresh();
        last_action = std::string(by_jump ? "jumped " : "stepped ") + ticks_text(ticks);
      }
      last_s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
      busy = false;
    });
  }

  void run_cold_check(std::uint64_t s) {
    if (busy) return;
    join();
    busy = true;
    cold_running = true;
    stage = "the cold world: stepping a week, then jumping it";
    thread = std::thread([this, s] {
      ColdResult r = run_cold(s);
      std::lock_guard<std::mutex> l(mutex);
      cold = r;
      cold_running = false;
      busy = false;
    });
  }
};

std::shared_ptr<SimState> make_sim() { return std::make_shared<SimState>(); }

bool sim_busy(const SimState& st) { return st.busy; }

void draw_sim(SimState& st, std::uint64_t seed, const gen::WorldSettings& settings, const std::string& world_name, bool auto_open,
              bool auto_cold, bool auto_advance) {
  ImGui::TextWrapped("The simulation card: the selected world opened here, in this process. Its systems in update order with the next "
                     "tick each has work at; its state hash and save size; step or jump by the hour or the day and watch the hash. "
                     "Below, spec 40's claim run now: a cold week jumped equals a week stepped, hash for hash.");
  ImGui::Separator();
  const bool busy = st.busy;
  if (auto_open && !st.t && !busy) st.open(seed, settings, world_name);
  if (auto_cold && !st.cold && !busy) st.run_cold_check(seed);
  if (auto_advance) st.auto_steps = 2;
  if (st.auto_steps > 0 && st.t && !busy) {
    st.advance(st.auto_steps == 2 ? kTicksPerGameDay : kTicksPerGameHour, st.auto_steps == 2);
    --st.auto_steps;
  }
  ImGui::BeginDisabled(busy);
  if (ImGui::Button(st.t ? "Open again" : ("Open " + world_name).c_str())) st.open(seed, settings, world_name);
  ImGui::EndDisabled();
  ImGui::SameLine();
  ImGui::TextDisabled("seed %llu, the world's settings; about 7 s on this PC", static_cast<unsigned long long>(seed));
  if (busy) {
    ImGui::TextColored({0.9f, 0.8f, 0.4f, 1}, "%s...", st.stage.c_str());
  }
  if (!st.error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", st.error.c_str());
  if (st.t && !busy) {
    std::lock_guard<std::mutex> l(st.mutex);
    World& w = *st.t->world;
    ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.3f);
    ImGui::TextColored({1, 0.95f, 0.8f, 1}, "%s, open (built in %.1f s)", st.world_name.c_str(), st.open_s);
    ImGui::PopFont();
    if (ImGui::BeginTable("readouts", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("k", 0, 1);
      ImGui::TableSetupColumn("v", 0, 5);
      auto row = [](const char* k, const std::string& v) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("%s", k);
        ImGui::TableSetColumnIndex(1);
        ImGui::TextWrapped("%s", v.c_str());
      };
      row("Now", format_civil(civil_now(w)) + "  (tick " + std::to_string(w.clock.tick) + ", " + ticks_text(w.clock.tick) + " since the start)");
      row("State hash", hex(st.hash) + "  (spec 06: the state part of a save; the same after save, load and replay)");
      row("Save", std::to_string(st.save_bytes) + " bytes uncompressed, format version " + std::to_string(kSaveFormatVersion));
      row("Place", st.t->place ? "a town of " + st.t->place->place().country_name + ", " + st.t->place->place().income + " income" : "the default block");
      row("Systems", std::to_string(w.systems().size()) + " in update order (below)");
      if (!st.last_action.empty()) row("Last", st.last_action + " in " + std::to_string(st.last_s).substr(0, 5) + " s");
      ImGui::EndTable();
    }
    ImGui::Spacing();
    if (ImGui::Button("Step 1 hour")) st.advance(kTicksPerGameHour, false);
    ImGui::SameLine();
    if (ImGui::Button("Jump 1 hour")) st.advance(kTicksPerGameHour, true);
    ImGui::SameLine();
    if (ImGui::Button("Step 1 day")) st.advance(kTicksPerGameDay, false);
    ImGui::SameLine();
    if (ImGui::Button("Jump 1 day")) st.advance(kTicksPerGameDay, true);
    ImGui::SameLine();
    ImGui::TextDisabled("a jump steps only the ticks some system is awake at (spec 40); with bodies in the block that is most of them");
    ImGui::Spacing();
    const std::int64_t now = w.clock.tick;
    if (ImGui::BeginTable("systems", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("System (update order)", 0, 2);
      ImGui::TableSetupColumn("Next wake", 0, 2);
      ImGui::TableSetupColumn("", 0, 1);
      ImGui::TableHeadersRow();
      for (const auto& s : w.systems()) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(s->name());
        const std::int64_t wake = s->next_wake(w, now);
        ImGui::TableSetColumnIndex(1);
        if (wake == System::kNever) ImGui::TextDisabled("never: it reacts to commands only");
        else if (wake <= now) ImGui::TextUnformatted("now: every tick");
        else ImGui::Text("in %s (tick %lld)", ticks_text(wake - now).c_str(), static_cast<long long>(wake));
        ImGui::TableSetColumnIndex(2);
        if (wake == System::kNever) ImGui::TextColored({0.6f, 0.6f, 0.9f, 1}, "cold");
        else if (wake <= now) ImGui::TextColored({0.95f, 0.6f, 0.4f, 1}, "hot");
        else ImGui::TextColored({0.55f, 0.9f, 0.55f, 1}, "sleeping");
      }
      ImGui::EndTable();
    }
    ImGui::TextDisabled("Next wake of the world as a whole: %s", [&] {
      const std::int64_t nw = w.next_wake(now);
      return nw <= now ? std::string("now") : "in " + ticks_text(nw - now);
    }().c_str());
    ImGui::Spacing();
    ImGui::TextUnformatted("What happened (spec 25: the lines a player reads; a day's five most notable events as its summary):");
    ImGui::BeginChild("chronicle", ImVec2(0, 200), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& d : st.days) ImGui::TextColored({1, 0.95f, 0.8f, 1}, "%s", d.c_str());
    if (st.lines.empty()) ImGui::TextDisabled("nothing yet: step or jump");
    for (const auto& l : st.lines) ImGui::TextUnformatted(l.c_str());
    if (!st.lines.empty()) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
  }
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.2f);
  ImGui::TextColored({1, 0.95f, 0.8f, 1}, "The cold-world check (spec 40)");
  ImGui::PopFont();
  ImGui::TextWrapped("A small world with an hourly economy, daily orders, grids and pops that only react to commands (the time test's "
                     "fixture, built here twice): one copy steps every tick of a week, the other jumps from wake to wake. The claim "
                     "is that they end in the same state, hash for hash, with the same event log, and that the jump steps a few "
                     "hundred ticks instead of 1.2 million.");
  ImGui::BeginDisabled(busy);
  if (ImGui::Button("Run the check now")) st.run_cold_check(seed);
  ImGui::EndDisabled();
  if (st.cold_running) {
    ImGui::SameLine();
    ImGui::TextColored({0.9f, 0.8f, 0.4f, 1}, "running...");
  }
  if (st.cold && !st.cold_running) {
    std::lock_guard<std::mutex> l(st.mutex);
    const ColdResult& r = *st.cold;
    if (!r.error.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", r.error.c_str());
    else {
      const bool same = r.hash_stepped == r.hash_jumped && r.log_stepped == r.log_jumped;
      if (ImGui::BeginTable("cold", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("", 0, 1);
        ImGui::TableSetupColumn("Ticks stepped", 0, 1);
        ImGui::TableSetupColumn("Time", 0, 1);
        ImGui::TableSetupColumn("State hash", 0, 2);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Stepped");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%lld", static_cast<long long>(r.ticks));
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.2f s", r.step_s);
        ImGui::TableSetColumnIndex(3);
        ImGui::TextUnformatted(hex(r.hash_stepped).c_str());
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Jumped");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%lld", static_cast<long long>(r.steps));
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.3f s", r.jump_s);
        ImGui::TableSetColumnIndex(3);
        ImGui::TextUnformatted(hex(r.hash_jumped).c_str());
        ImGui::EndTable();
      }
      if (same) ImGui::TextColored({0.55f, 0.9f, 0.55f, 1}, "Same hash, same event log (%zu events): the week jumped equals the week stepped, %lldx fewer ticks.",
                                   r.log_stepped, static_cast<long long>(r.steps > 0 ? r.ticks / r.steps : 0));
      else ImGui::TextColored({1, 0.5f, 0.5f, 1}, "DIFFERENT: hashes %s / %s, events %zu / %zu. That is a bug to report.", hex(r.hash_stepped).c_str(),
                              hex(r.hash_jumped).c_str(), r.log_stepped, r.log_jumped);
    }
  }
}

}  // namespace zs::atlas
