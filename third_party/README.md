# Third-party code

Vendored so the project builds offline on any machine. Never edited; to update, replace the folder and change this table.

| Library | Version | Licence | Used for |
| --- | --- | --- | --- |
| [doctest](https://github.com/doctest/doctest) | 2.5.3 (`doctest/doctest.h`) | MIT | Unit tests |
| [zstd](https://github.com/facebook/zstd) | 1.5.7 (`zstd/lib`: common, compress, decompress; release tarball sha256 `eb33e51f...6fa3`) | BSD 3-Clause (`zstd/LICENSE`; the project is dual BSD/GPLv2, we use BSD) | Compressing saves, autosaves and traces (spec 06 rule 17) |
| [toml++](https://github.com/marzer/tomlplusplus) | 3.4.0 (`toml.hpp` single header) | MIT | Data files (items, map) |
| [RapidCheck](https://github.com/emil-e/rapidcheck) | master @ 2c3c4365 (tests, examples and docs removed) | BSD 2-Clause | Property tests |

## Fetched at configure time

Not stored here: CMake downloads them at a pinned commit when the window is built (`apps/zs/CMakeLists.txt`). Listed so a notices file can be generated for the game (decision 0055 section 1.8).

| Library | Version (commit) | Licence | Used for |
| --- | --- | --- | --- |
| [SDL3](https://github.com/libsdl-org/SDL) | release-3.4.18 (`829a65d7`) | zlib | Window, input, 2D renderer |
| [Dear ImGui](https://github.com/ocornut/imgui) | v1.92.9b (`f1cc2ae1`) | MIT | Panels and the map's draw lists |

## Ported code

Code taken from another project and rewritten in our files; its licence travels with it.

| Source | Version (commit) | Licence | Ported to |
| --- | --- | --- | --- |
| [God's Eye View](https://github.com/bilawalsidhu/gods-eye-view) | main (`3e5a18f6`) | MIT (`engine/render/shaders/LICENSE.gods-eye-view`) | Thermal and night-vision passes (`engine/render/shaders/thermal.frag`, `night_vision.frag`) |
