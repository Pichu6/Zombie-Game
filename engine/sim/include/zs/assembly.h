// Components and assemblies: one data model for everything built. Spec: docs/tier0/specs/08-assembly.md
#pragma once
#include <array>
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

// Numeric properties that processes test (spec 15 model). One fixed list: append only.
enum class Prop : std::uint8_t {
  Edge,         // cutting edge, 0-1
  Blunt,        // striking mass and hardness, 0-1 (hammer ~1)
  Lever,        // prying, 0-1 (crowbar 1, screwdriver ~0.3)
  Grip,         // fine driving and turning (screwdriver, wrench), 0-1
  ContainerMl,  // holds this many mL of liquid
  ClothAreaM2,  // usable cloth
  Absorbent,    // soaks blood, 0-1
  FlatBoard,    // usable as a board for barricades and splints, 0-1
  Fastener,     // nails or screws, 0-1
  Rigid,        // stiff enough for a splint, 0-1
  Count
};
inline constexpr std::size_t kPropCount = static_cast<std::size_t>(Prop::Count);
const char* to_string(Prop p);
std::optional<Prop> prop_from(const std::string& name);

// Physical numbers live here, once per material (spec 15 model).
struct MaterialDef {
  std::string id;
  double density_kg_m3 = 0;
  double hardness = 0;  // 0-1, relative (steel 1, pine 0.3, cotton 0)
  std::string source;
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
  std::string material;                       // id in the materials table ("" = mixed or none)
  std::array<double, kPropCount> props{};     // properties at Q50 and full condition
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
  void add_material(MaterialDef m) { materials_.push_back(std::move(m)); }
  const MaterialDef* find_material(const std::string& id) const;

  // A property of an instance: the definition's value scaled by Q and condition
  // (x (0.5 + Q/100) x (0.5 + 0.5 condition)); 0 when the item is failed.
  double prop(EntityId id, Prop p) const;

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
  std::vector<MaterialDef> materials_;
  std::map<EntityId, Instance> instances_;
};

}  // namespace zs
