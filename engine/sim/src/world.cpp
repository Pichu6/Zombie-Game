#include "zs/world.h"

#include <chrono>

#include <algorithm>

#include <stdexcept>

#include <zstd.h>

#if defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__)
#include <xmmintrin.h>
#define ZS_HAS_MXCSR 1
#else
#define ZS_HAS_MXCSR 0
#endif

namespace zs {

World::World(std::uint64_t seed) : rng(seed) {}

void World::add_system(std::unique_ptr<System> system) { systems_.push_back(std::move(system)); }

void World::close_setup() {
  ledger.close_setup();
  emit(EventType::RunStart, 0, 0);
}

void World::submit(Command command) {
  command.tick = clock.tick;
  pending_.push_back(command);
  input_log_.push_back(command);
}

void World::queue_intent(Command command) {
  command.tick = clock.tick;
  pending_.push_back(command);
}

EventId World::emit(EventType type, EntityId subject, EventId cause, std::int64_t a, std::int64_t b) {
  const EventId id = log.append(clock.tick, type, subject, cause, a, b);
  emitted_.push_back(id);
  return id;
}

namespace {
// The default SSE environment: round to nearest, no flush-to-zero, no denormals-are-zero (DET-40). Something in
// the process (a driver, a plugin) may change it; the simulation puts it back before each step.
bool restore_fp_env() {
#if ZS_HAS_MXCSR
  constexpr unsigned kRounding = 0x6000, kFlushToZero = 0x8000, kDenormalsAreZero = 0x0040;
  const unsigned csr = _mm_getcsr();
  if (csr & (kRounding | kFlushToZero | kDenormalsAreZero)) {
    _mm_setcsr(csr & ~(kRounding | kFlushToZero | kDenormalsAreZero));
    return true;
  }
#endif
  return false;
}
}  // namespace

void World::step() {
  if (restore_fp_env()) ++fp_env_restored;
  emitted_.clear();
  // 1. Input commands, in submission order.
  for (const Command& c : pending_) {
    const EventId id = emit(EventType::Intent, c.subject, 0, c.type, c.a);
    for (auto& s : systems_) s->apply_command(*this, c, id);
  }
  pending_.clear();
  // 2. Systems, in their fixed order; the probes count and, when asked, time each update.
  if (probes.size() < systems_.size()) probes.resize(systems_.size());
  for (std::size_t i = 0; i < systems_.size(); ++i) {
    if (probe_timing) {
      const auto t0 = std::chrono::steady_clock::now();
      systems_[i]->update(*this);
      probes[i].ns += static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t0).count());
    } else {
      systems_[i]->update(*this);
    }
    ++probes[i].updates;
  }
  // 3. Scheduled events due this tick.
  for (const ScheduledEvent& e : scheduler.pop_due(clock.tick))
    for (auto& s : systems_) s->on_scheduled(*this, e);
  // 4. Ledger check.
  if (check_every_tick) {
    last_violation = ledger.check();
    if (last_violation) throw std::logic_error("ledger: " + *last_violation);
  }
  ++clock.tick;
}

std::int64_t World::next_wake(std::int64_t now) const {
  if (!pending_.empty()) return now;
  std::int64_t wake = System::kNever;
  for (const auto& s : systems_) wake = std::min(wake, std::max(now, s->next_wake(*this, now)));
  if (!scheduler.pending().empty()) wake = std::min(wake, std::max(now, scheduler.pending().begin()->first.first));
  return wake;
}

void World::jump(std::int64_t to_tick) {
  while (clock.tick < to_tick) {
    const std::int64_t wake = next_wake(clock.tick);
    if (wake >= to_tick) {  // nothing has work before to_tick: skip straight there
      clock.tick = to_tick;
      if (check_every_tick) {
        last_violation = ledger.check();
        if (last_violation) throw std::logic_error("ledger: " + *last_violation);
      }
      return;
    }
    clock.tick = wake;
    step();
  }
}

void World::save_state(ArchiveWriter& w, bool with_input_log) const {
  save_core(w, with_input_log);
  w.u64(systems_.size());
  for (const auto& s : systems_) { w.str(s->name()); s->save(w); }
}

std::vector<World::SystemHash> World::system_hashes() const {
  std::vector<SystemHash> out;
  ArchiveWriter core;
  save_core(core, true);
  out.push_back({"core", hash_bytes(core.bytes())});
  for (const auto& s : systems_) {
    ArchiveWriter w;
    s->save(w);
    out.push_back({s->name(), hash_bytes(w.bytes())});
  }
  return out;
}

void World::save_core(ArchiveWriter& w, bool with_input_log) const {
  w.i64(clock.tick);
  for (std::uint64_t c : creator_counters_) w.u64(c);  // v5: one counter per creator (decision 0065)
  w.u64(rng.seed());
  w.u64(rng.counters().size());
  for (const auto& [key, value] : rng.counters()) { w.u64(key.first); w.u32(key.second); w.u64(value); }
  w.boolean(ledger.setup_open());
  w.u64(ledger.expected_totals().size());
  for (std::int64_t v : ledger.expected_totals()) w.i64(v);
  w.u64(ledger.balances().size());
  for (const auto& [key, value] : ledger.balances()) { w.u32(key.first); w.u16(key.second); w.i64(value); }
  w.u64(locations.all().size());
  for (const auto& [entity, parent] : locations.all()) { w.u64(entity); w.u64(parent); }
  w.u64(log.all().size());
  for (const Event& e : log.all()) {
    w.u64(e.id); w.i64(e.tick); w.u16(static_cast<std::uint16_t>(e.type));
    w.u64(e.subject); w.u64(e.cause); w.i64(e.a); w.i64(e.b);
  }
  w.u64(scheduler.next_sequence());
  w.u64(scheduler.pending().size());
  for (const auto& [key, e] : scheduler.pending()) {
    w.i64(key.first); w.u64(key.second);
    w.i64(e.tick); w.u16(static_cast<std::uint16_t>(e.type)); w.u64(e.subject); w.u64(e.cause); w.i64(e.a); w.i64(e.b);
  }
  if (with_input_log) {
    w.u64(input_log_.size());
    for (const Command& c : input_log_) { w.i64(c.tick); w.u16(c.type); w.u64(c.subject); w.i64(c.a); w.i64(c.b); }
  }
  w.u64(pending_.size());  // v3: submitted, applied on the next step
  for (const Command& c : pending_) { w.i64(c.tick); w.u16(c.type); w.u64(c.subject); w.i64(c.a); w.i64(c.b); }
}

void World::load_state(ArchiveReader& r, std::uint32_t version) {
  clock.tick = r.i64();
  if (version >= 5) {
    for (std::uint64_t& c : creator_counters_) c = r.u64();
  } else {
    // v2 to v4: one global counter; its ids keep the Legacy creator (0), and the new creators start from nothing.
    creator_counters_[0] = version >= 2 ? r.u64() : kFirstEntity;
    for (std::size_t k = 1; k < static_cast<std::size_t>(Creator::kCount); ++k) creator_counters_[k] = 0;
  }
  rng = Rng(r.u64());
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    const EntityId e = r.u64(); const std::uint32_t p = r.u32(); rng.set_counter(e, p, r.u64());
  }
  const bool setup_open = r.boolean();
  std::vector<std::int64_t> expected(r.u64());
  if (expected.size() != ledger.resource_names().size()) throw std::runtime_error("save: resource list differs");
  for (auto& v : expected) v = r.i64();
  std::map<std::pair<OwnerId, ResourceId>, std::int64_t> balances;
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    const OwnerId o = r.u32(); const ResourceId res = r.u16(); balances[{o, res}] = r.i64();
  }
  ledger.restore(std::move(balances), std::move(expected), setup_open);
  std::map<EntityId, EntityId> parents;
  for (std::uint64_t n = r.u64(); n > 0; --n) { const EntityId e = r.u64(); parents[e] = r.u64(); }
  locations.restore(std::move(parents));
  std::vector<Event> events(r.u64());
  for (Event& e : events) {
    e.id = r.u64(); e.tick = r.i64(); e.type = static_cast<EventType>(r.u16());
    e.subject = r.u64(); e.cause = r.u64(); e.a = r.i64(); e.b = r.i64();
  }
  log.restore(std::move(events));
  const std::uint64_t seq = r.u64();
  std::multimap<std::pair<std::int64_t, std::uint64_t>, ScheduledEvent> queue;
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    const std::int64_t t = r.i64(); const std::uint64_t s = r.u64();
    ScheduledEvent e{};
    e.tick = r.i64(); e.type = static_cast<EventType>(r.u16()); e.subject = r.u64(); e.cause = r.u64();
    e.a = r.i64(); e.b = r.i64();
    queue.emplace(std::pair{t, s}, e);
  }
  scheduler.restore(std::move(queue), seq);
  input_log_.resize(r.u64());
  for (Command& c : input_log_) { c.tick = r.i64(); c.type = r.u16(); c.subject = r.u64(); c.a = r.i64(); c.b = r.i64(); }
  pending_.clear();
  if (version >= 3) {
    pending_.resize(r.u64());
    for (Command& c : pending_) { c.tick = r.i64(); c.type = r.u16(); c.subject = r.u64(); c.a = r.i64(); c.b = r.i64(); }
  }
  emitted_.clear();
  if (r.u64() != systems_.size()) throw std::runtime_error("save: system list differs");
  for (auto& s : systems_) {
    if (r.str() != s->name()) throw std::runtime_error("save: system order differs");
    s->load(r);
  }
}

namespace {
constexpr std::uint32_t kSaveMagic = 0x5641535a;  // "ZSAV"
// Header of version 6: magic, version, codec, uncompressed size, hash of the uncompressed state.
constexpr std::size_t kHeaderV6 = 4 + 4 + 4 + 8 + 8;
constexpr std::uint64_t kMaxStateBytes = 1ull << 30;  // a header claiming more is refused before any allocation
}  // namespace

std::vector<std::uint8_t> World::save(SaveCodec codec, int level) const {
  ArchiveWriter state;
  save_state(state);
  const std::vector<std::uint8_t>& raw = state.bytes();
  ArchiveWriter head;
  head.u32(kSaveMagic);
  head.u32(kSaveFormatVersion);
  head.u32(static_cast<std::uint32_t>(codec));
  head.u64(raw.size());
  head.u64(hash_bytes(raw));
  std::vector<std::uint8_t> bytes = head.bytes();
  if (codec == SaveCodec::None) {
    bytes.insert(bytes.end(), raw.begin(), raw.end());
    return bytes;
  }
  // One frame, single-threaded, content size and checksum on (spec 06 rule 17): the same state gives the same bytes.
  ZSTD_CCtx* ctx = ZSTD_createCCtx();
  ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, level);
  ZSTD_CCtx_setParameter(ctx, ZSTD_c_checksumFlag, 1);
  ZSTD_CCtx_setParameter(ctx, ZSTD_c_contentSizeFlag, 1);
  const std::size_t room = ZSTD_compressBound(raw.size());
  bytes.resize(kHeaderV6 + room);
  const std::size_t n = ZSTD_compress2(ctx, bytes.data() + kHeaderV6, room, raw.data(), raw.size());
  ZSTD_freeCCtx(ctx);
  if (ZSTD_isError(n)) throw std::runtime_error(std::string("save: compression failed: ") + ZSTD_getErrorName(n));
  bytes.resize(kHeaderV6 + n);
  return bytes;
}

void World::load(const std::vector<std::uint8_t>& bytes) {
  ArchiveReader r(bytes);
  if (r.u32() != kSaveMagic) throw std::runtime_error("not a save file");
  const std::uint32_t version = r.u32();
  // Older versions are upgraded field by field in load_state. Unknown versions refuse to load.
  if (version < 1 || version > kSaveFormatVersion) throw std::runtime_error("unsupported save version " + std::to_string(version));
  if (version <= 5) {  // magic, version, hash, then the state as it is
    const std::uint64_t expected_hash = r.u64();
    const std::vector<std::uint8_t> state(bytes.begin() + 16, bytes.end());
    if (hash_bytes(state) != expected_hash) throw std::runtime_error("save file is corrupt (hash mismatch)");
    r.format = version;
    load_state(r, version);
    if (!r.at_end()) throw std::runtime_error("save file has trailing data");
    return;
  }
  const std::uint32_t codec = r.u32();
  const std::uint64_t size = r.u64();
  const std::uint64_t expected_hash = r.u64();
  if (size > kMaxStateBytes) throw std::runtime_error("save file claims an absurd size");
  std::vector<std::uint8_t> state;
  if (codec == static_cast<std::uint32_t>(SaveCodec::None)) {
    state.assign(bytes.begin() + kHeaderV6, bytes.end());
    if (state.size() != size) throw std::runtime_error("save file is truncated or has trailing data");
  } else if (codec == static_cast<std::uint32_t>(SaveCodec::Zstd)) {
    const std::uint8_t* frame = bytes.data() + kHeaderV6;
    const std::size_t frame_size = bytes.size() - kHeaderV6;
    if (ZSTD_getFrameContentSize(frame, frame_size) != size) throw std::runtime_error("save file: frame size differs from the header");
    state.resize(size);
    const std::size_t n = ZSTD_decompress(state.data(), state.size(), frame, frame_size);
    if (ZSTD_isError(n) || n != size) throw std::runtime_error("save file is corrupt (decompression failed)");
  } else {
    throw std::runtime_error("unsupported save codec " + std::to_string(codec));
  }
  if (hash_bytes(state) != expected_hash) throw std::runtime_error("save file is corrupt (hash mismatch)");
  ArchiveReader sr(state);
  sr.format = version;
  load_state(sr, version);
  if (!sr.at_end()) throw std::runtime_error("save file has trailing data");
}

std::uint64_t World::state_hash() const {
  ArchiveWriter state;
  save_state(state);
  return hash_bytes(state.bytes());
}

std::uint64_t World::world_hash() const {
  ArchiveWriter state;
  save_state(state, false);
  return hash_bytes(state.bytes());
}

}  // namespace zs
