// Components and assemblies: one data model for everything built. Spec: docs/tier0/specs/08-assembly.md
#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "zs/condition.h"
#include "zs/world.h"

namespace zs {

using DefId = std::uint16_t;

enum class Joint : std::uint8_t { Loose, Screwed, Nailed, Welded, Glued };

struct SlotDef {
  std::string name;
  std::string accepts;           // family of part that fits
  bool required = true;          // missing or failed -> the assembly does not work
  Joint joint = Joint::Screwed;  // sets which salvage process applies
  std::string default_part;      // def id built into new instances ("" = empty)
};

// Shared data for one kind of thing. Loaded from data/tier0/items.toml.
struct ItemDef {
  std::string id;
  std::string name;
  std::string family;
  std::int64_t mass_g = 0;                    // this part alone, without mounted parts
  std::vector<SlotDef> slots;
  std::int64_t failure_threshold = 100'000;   // condition (ppm) below which it stops working
  std::vector<std::string> failure_chain;     // slot names, weakest first (fortification)
  double hold_force_n = 0;                    // structural parts: force held at Q50, full condition
  double sound_loss_db = 0;                   // walls, doors, windows: transmission loss when closed
  std::string source;                         // where the numbers come from
};

struct Instance {
  EntityId id = 0;
  DefId def = 0;
  int quality = 50;  // 0-100, fixed except through a real process
  Condition condition;
  std::vector<EntityId> slots;  // one per slot of the def; 0 = empty
};

// Caps on the quality of something made or repaired (decision 0014).
struct QualityCaps {
  double skill = 100;
  double tool = 100;
  double method_ceiling = 100;
  double station = 100;  // no stations in Tier 0
};

// Weighted mean of the governing parts' Q (weights = function), capped by min(skill, tool, method, station).
double made_quality(std::span<const std::pair<int, double>> parts_q_weight, const QualityCaps& caps);

class Assemblies : public System {
 public:
  const char* name() const override { return "assemblies"; }
  void update(World&) override {}
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  // Definitions (setup).
  DefId add_def(ItemDef def);
  const ItemDef& def(DefId id) const { return defs_.at(id); }
  std::optional<DefId> find_def(const std::string& id) const;
  const std::vector<ItemDef>& defs() const { return defs_; }

  // Creates an instance with its default parts, all at `quality`, placed under `parent`.
  EntityId create(World& w, DefId def, int quality, EntityId parent);

  const Instance& get(EntityId id) const { return instances_.at(id); }
  Instance& get(EntityId id) { return instances_.at(id); }
  bool exists(EntityId id) const { return instances_.contains(id); }
  const std::map<EntityId, Instance>& all() const { return instances_; }

  // Mounts a loose part into an empty slot. Returns false if it does not fit.
  bool mount(World& w, EntityId parent, std::size_t slot, EntityId part);
  // Salvage: removes the part, damages it by `condition_loss`, places it under `destination`.
  // Mass is conserved exactly. Returns the part id, or 0 if the slot was empty.
  EntityId unmount(World& w, EntityId parent, std::size_t slot, EntityId destination, std::int64_t condition_loss);

  std::int64_t mass_g(EntityId id) const;  // itself plus mounted parts
  bool works(EntityId id) const;           // not failed, every required part present and working
  std::optional<std::size_t> slot_index(EntityId id, const std::string& slot_name) const;

 private:
  std::vector<ItemDef> defs_;
  std::map<EntityId, Instance> instances_;
};

}  // namespace zs
