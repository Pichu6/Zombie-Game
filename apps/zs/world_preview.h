// Spec 84 rule 3: a preview of a world's draft settings before Create or Apply: the draft generated at coarse cells on a
// worker thread, drawn in spec 83's layers, with its key numbers beside Earth's. Nothing is written.
#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "planet_raster.h"
#include "world_numbers.h"
#include "zs/gen/place.h"

struct SDL_Renderer;
struct SDL_Texture;

namespace zs::atlas {

class WorldPreview {
 public:
  ~WorldPreview();
  // Starts generating `settings` for `seed` at the preview's grid; a running preview finishes first.
  void start(std::uint64_t seed, const gen::WorldSettings& settings);
  bool busy() const { return busy_; }
  bool ready() const { return ready_; }
  const gen::WorldSettings& settings() const { return settings_; }
  // Draws the layer row, the map and the numbers (ImGui); the texture is made on the renderer when needed.
  void draw(SDL_Renderer* renderer, void (*image)(SDL_Texture*, const char*, float, float));

 private:
  void render();
  std::thread thread_;
  std::atomic<bool> busy_{false}, ready_{false};
  std::mutex mutex_;
  std::string stage_;
  std::uint64_t seed_ = 0;
  gen::WorldSettings settings_;
  std::shared_ptr<const gen::GeneratedWorld> world_;
  std::vector<PreviewNumber> numbers_;
  double seconds_ = 0;
  MapLayer layer_ = MapLayer::Physical;
  PlanetRaster raster_;
  SDL_Texture* texture_ = nullptr;
  bool dirty_ = false;
};

}  // namespace zs::atlas
