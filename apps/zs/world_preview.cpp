#include "world_preview.h"

#include <SDL3/SDL.h>
#include <imgui.h>

#include <chrono>
#include <exception>

namespace zs::atlas {

WorldPreview::~WorldPreview() {
  if (thread_.joinable()) thread_.join();
  if (texture_) SDL_DestroyTexture(texture_);
}

void WorldPreview::start(std::uint64_t seed, const gen::WorldSettings& settings) {
  if (busy_) return;
  if (thread_.joinable()) thread_.join();
  seed_ = seed;
  settings_ = settings;
  ready_ = false;
  busy_ = true;
  thread_ = std::thread([this] {
    const auto t0 = std::chrono::steady_clock::now();
    try {
      auto stage = [this](const char* what) {
        std::lock_guard<std::mutex> l(mutex_);
        stage_ = what;
      };
      auto w = std::make_shared<gen::GeneratedWorld>(
          gen::generate_world(seed_, settings_, gen::world_frequency(settings_.radius_km, gen::kPreviewCellKm2), stage));
      stage("the numbers");
      auto numbers = key_numbers(*w);
      std::lock_guard<std::mutex> l(mutex_);
      world_ = std::move(w);
      numbers_ = std::move(numbers);
      stage_.clear();
    } catch (const std::exception& ex) {
      std::lock_guard<std::mutex> l(mutex_);
      stage_ = std::string("could not generate the preview: ") + ex.what();
    }
    seconds_ = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    dirty_ = true;
    ready_ = true;
    busy_ = false;
  });
}

void WorldPreview::render() {
  if (!world_) return;
  MapInputs mi;
  mi.countries = &world_->people.formed;
  mi.deposits = &world_->deposits;
  mi.people = &world_->people;
  mi.use = &world_->use;
  mi.projection = Projection::EqualEarth;
  raster_ = render_layer(world_->grid, world_->fields, world_->land, mi, layer_, 1024, 512);
}

void WorldPreview::draw(SDL_Renderer* renderer, void (*image)(SDL_Texture*, const char*, float, float)) {
  if (busy_) {
    std::string stage;
    {
      std::lock_guard<std::mutex> l(mutex_);
      stage = stage_;
    }
    ImGui::TextDisabled("Previewing at coarse cells: %s...", stage.c_str());
    return;
  }
  if (!ready_ || !world_) {
    if (!stage_.empty()) ImGui::TextColored({1, 0.5f, 0.5f, 1}, "%s", stage_.c_str());
    return;
  }
  // Spec 83's layers, the same row as the planet's.
  for (int k = 0; k < kMapLayers; ++k) {
    const auto l = static_cast<MapLayer>(k);
    if (k % 6) ImGui::SameLine();  // two rows of six
    if (ImGui::RadioButton((std::string(layer_name(l)) + "##pv").c_str(), layer_ == l) && layer_ != l) layer_ = l, dirty_ = true;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", layer_legend(l).c_str());
  }
  if (dirty_) {
    render();
    if (texture_) SDL_DestroyTexture(texture_), texture_ = nullptr;
    dirty_ = false;
  }
  if (!texture_ && raster_.w > 0) {
    std::vector<unsigned char> rgba(static_cast<std::size_t>(raster_.w) * raster_.h * 4);
    for (std::size_t i = 0; i < static_cast<std::size_t>(raster_.w) * raster_.h; ++i) {
      for (int q = 0; q < 3; ++q) rgba[i * 4 + q] = raster_.rgb[i * 3 + q];
      rgba[i * 4 + 3] = 255;
    }
    texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, raster_.w, raster_.h);
    if (texture_) {
      SDL_UpdateTexture(texture_, nullptr, rgba.data(), raster_.w * 4);
      SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_LINEAR);
    }
  }
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
  ImGui::TextWrapped("Preview: %zu cells of about %.0f km2, %.1f s. Create makes the same planet at 5,000 km2 a "
                      "cell: the same continents, climate and rivers, with finer detail.",
                      static_cast<std::size_t>(world_->grid.size()), world_->grid.sphere_area_km2() / world_->grid.size(), seconds_);
  ImGui::PopStyleColor();
  if (texture_) {
    const float w = std::min(ImGui::GetContentRegionAvail().x, 900.0f);
    image(texture_, "preview", w, w / 2);
  }
  if (ImGui::BeginTable("preview numbers", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
    ImGui::TableSetupColumn("What");
    ImGui::TableSetupColumn("This world");
    ImGui::TableSetupColumn("Earth");
    ImGui::TableSetupColumn("");
    ImGui::TableHeadersRow();
    for (const PreviewNumber& n : numbers_) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(n.what.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%.3g %s", n.value, n.unit.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%.3g %s", n.earth, n.unit.c_str());
      ImGui::TableNextColumn();
      ImGui::TextColored(n.realistic ? ImVec4(0.55f, 0.9f, 0.55f, 1) : ImVec4(0.95f, 0.75f, 0.35f, 1), "%s",
                         n.realistic ? "realistic" : "sandbox");
    }
    ImGui::EndTable();
  }
}

}  // namespace zs::atlas
