# Juego Zombie

A realistic zombie survival simulation for PC, on our own C++ engine with a Vulkan renderer. The question it asks: how long can you really survive? You play one survivor in real time, and an XCOM-like commander layer opens as you find others. Player, survivors and zombies share one simulated body model and one set of rules. Everything built (walls, pipes, engines, items) can be scavenged and crafted, tied by one quality system. We start with a 2D simulation prototype (Tier 0) before any renderer work.

## Language

**Everything in this repo is in English:** docs, code, comments, commit messages, and the game's own text. The user may write in Spanish in chat; answer in the language they write in, but keep the repo in English.

## Before you work

1. Read `docs/status.md` (where we are) and `docs/vision.md` (what the game is).
2. Read the design doc for the system you touch in `docs/design/`, and its decisions in `docs/decisions/`.
3. The user's current instructions override any document.

## Where things live

| What | Where | Rule |
| --- | --- | --- |
| Current state and next step | `docs/status.md` | One screen. Rewrite it; don't append. |
| Game vision | `docs/vision.md` | One page. |
| Design per system | `docs/design/<system>.md` | One source of truth per system. |
| Tier 0 plan and unit specs | `docs/tier0/` | A unit's spec is written before its code. |
| Decisions | `docs/decisions/NNNN-topic.md` | One per file, dated. Never rewritten; a new decision supersedes an old one and says so. |
| Research | `docs/research/` | Dated notes; not decisions. |
| Prototype lessons | `docs/lessons/` | What failed and why. |
| Terms | `docs/glossary.md` | |

## Working rules

- **Always distinguish** *Decided* (the user confirmed it), *Proposal* (not confirmed), *Implemented* (exists in code) and *Validated* (tested, with evidence). A passing test is not a visual validation.
- **The simulation core does not depend on the renderer.** It is a C++ library that runs and is tested without a window. Renderer, cameras and UI read its state and send it commands.
- **Write only what makes this game different.** Use proven libraries for the rest (see `docs/decisions/0002-engine-stack.md`).
- **Every engine task serves a vertical-slice scenario** (`docs/status.md`). If none needs it, it waits.
- Small files with one responsibility, by design rather than by a line limit.
- Don't spend credits on paid services or take external actions without asking.

## Old prototype

`D:/Proyectos/juego zombie` (Godot, Mixamo, Blender, Tripo3D) is **reference only**. Don't continue it or copy its code. Its lessons are distilled in `docs/lessons/` and its design in `docs/design/`.

## Build and test

Needs CMake 3.25+, Ninja and a C++20 compiler (MSVC 2022, GCC 13 or Clang 17). Test libraries are vendored in `third_party/`.

```
cmake --preset debug          # or: release
cmake --build --preset debug
ctest --preset debug          # unit, property and golden tests
build/debug/apps/headless/zs_headless --days 14   # stage 1 demo run, one line per game day
build/debug/apps/headless/zs_headless --map       # stage 2: room graph of the Tier 0 block
build/release/apps/headless/zs_headless --body-lab  # stage 3: body calibration scenarios vs their bands
build/release/apps/headless/zs_headless --bench     # stage 3: body cost, 4 pops + 60 walkers for a game day
build/release/apps/headless/zs_headless --scripted-pop [--ai]  # stage 4: a pop eats, treats a wound, salvages, barricades
build/release/apps/headless/zs_headless --stage6 [--seed N]  # stage 6: grid fails, generator, fuel, fridge; night noise and light snapshot
```

On Windows, run these from a "x64 Native Tools Command Prompt for VS 2022". Golden hashes in the tests are pinned on Linux; Windows must reproduce them (cross-compiler determinism, [spec 01](docs/tier0/specs/01-quantity.md)).

Layout: `engine/sim` is the simulation library (no window), `apps/headless` the command-line runner, `docs/tier0` the plan and specs.
