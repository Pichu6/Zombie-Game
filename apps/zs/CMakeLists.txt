# zs: the one executable (decision 0052): the window, the headless modes and the content generators.
add_executable(zs main.cpp headless.cpp pop_bench.cpp town_demo.cpp gen_cli.cpp)
target_link_libraries(zs PRIVATE zs_sim zs_gen)

if(ZS_BUILD_WINDOW)
  # The Tier 0 window (spec 16): SDL3 + Dear ImGui, 2D, the SDL renderer (no Vulkan).
  # Both libraries are fetched at configure time, pinned to a release commit. To build offline, point
  # FETCHCONTENT_SOURCE_DIR_SDL3 / FETCHCONTENT_SOURCE_DIR_IMGUI at local checkouts.
  include(FetchContent)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG d9d5536704d585616d4db3c8ba3c4ff6fc2757e1  # release-3.4.8
    GIT_SHALLOW FALSE)
  FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG 01380c579715e62fb9a8d6ec0502c4ea83bfde6e  # v1.92.9
    GIT_SHALLOW FALSE)
  FetchContent_MakeAvailable(sdl3 imgui)

  add_library(zs_imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp)
  target_include_directories(zs_imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
  target_link_libraries(zs_imgui PUBLIC SDL3::SDL3-static)

  target_sources(zs PRIVATE window.cpp)
  target_link_libraries(zs PRIVATE zs_imgui)
  target_compile_definitions(zs PRIVATE ZS_WITH_WINDOW)
else()
  target_sources(zs PRIVATE window_none.cpp)
endif()
