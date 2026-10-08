# Juego Zombie

A realistic zombie survival simulation for PC, on our own C++ engine with a Vulkan renderer. The question it asks: how long can you really survive? You play one survivor in real time, and an XCOM-like commander layer opens as you find others. Player, survivors and zombies share one simulated body model and one set of rules. Everything built (walls, pipes, engines, items) can be scavenged and crafted, tied by one quality system. We start with a 2D simulation prototype (Tier 0) before any renderer work.

## Language

**Everything in this repo is in English:** docs, code, comments, commit messages, and the game's own text. The user may write in Spanish in chat; answer in the language they write in, but keep the repo in English.

## Before you work

0. **One line of work** ([0047](docs/decisions/0047-one-line-of-work.md)): `git fetch origin` first and read `docs/status.md` from `origin/main`; check nothing you are about to build already exists or is on an open branch or pull request; take decision numbers from `origin/main`; push before you stop. A cloud session leaves a Windows build (CI zip) and says which commit it came from; a local session verifies on MSVC.
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
| Research | `docs/research/` | Dated notes; not decisions. Findings with IDs and confidence, checked by `python3 tools/docs/check_research.py` ([0054](docs/decisions/0054-research-notes-findings-and-links.md)). |
| Prototype lessons | `docs/lessons/` | What failed and why. |
| Terms | `docs/glossary.md` | |

## Working rules

- **Always distinguish** *Decided* (the user confirmed it), *Proposal* (not confirmed), *Implemented* (exists in code) and *Validated* (tested, with evidence). A passing test is not a visual validation.
- **The simulation core does not depend on the renderer.** It is a C++ library that runs and is tested without a window. Renderer, cameras and UI read its state and send it commands.
- **Write only what makes this game different.** Use proven libraries for the rest (see `docs/decisions/0002-engine-stack.md`).
- **Every engine task serves a vertical-slice scenario** (`docs/status.md`). If none needs it, it waits.
- Small files with one responsibility, by design rather than by a line limit.
- Don't spend credits on paid services or take external actions without asking.
- **One executable, one simulation, nothing waiting** ([0052](docs/decisions/0052-one-executable-one-simulation.md)): every new mode goes into `zs`, every built system into the one world. When a choice comes up, take a sensible default, record it in a decision marked "default taken by Claude", and keep going; don't leave items waiting on the user.

## Old prototype

`D:/Proyectos/juego zombie` (Godot, Mixamo, Blender, Tripo3D) is **reference only**. Don't continue it or copy its code. Its lessons are distilled in `docs/lessons/` and its design in `docs/design/`.

## Build and test

Needs CMake 3.25+, Ninja and a C++20 compiler (MSVC 2022, GCC 13 or Clang 17). Test libraries are vendored in `third_party/`.

```
cmake --preset debug          # or: release
cmake --build --preset debug
ctest --preset debug          # unit, property and golden tests
build/debug/apps/zs/zs --days 14   # stage 1 demo run, one line per game day
build/debug/apps/zs/zs --map       # stage 2: room graph of the Tier 0 block
build/release/apps/zs/zs --body-lab  # stage 3: body calibration scenarios vs their bands
build/release/apps/zs/zs --bench     # stage 3: body cost, 4 pops + 60 walkers for a game day
build/release/apps/zs/zs --scripted-pop [--ai]  # stage 4: a pop eats, treats a wound, salvages, barricades
build/release/apps/zs/zs --stage6 [--seed N]  # stage 6: grid fails, generator, fuel, fridge; night noise and light snapshot
build/release/apps/zs/zs --stage7 [--seed N]  # stage 7: stage 6 with the dead (census, walkers, bites, turning); runs until the pop rises
build/release/apps/zs/zs --group [--seed N] [--days D] [--seeds N]  # stage 8: 4 AI pops with the dead; --seeds runs a batch and counts causes
build/release/apps/zs/zs --run [--seed N] [--seeds N] [--careless]  # stage 9: the 14-day run (found survivors, stranger, radio), death screen and day-by-day chronicle
build/release/apps/zs/zs gen town --seed 1 [--country base_low]   # content tools: a generated town with every pop's dossier, optionally for a base country (also: census, names, cultures)
build/release/apps/zs/zs [--seed N] [--watch]   # the window (spec 16), the default with no headless flag; --shot f.bmp [--ticks N] [--menu x,y] [--overlay noise|light] renders offscreen
# -DZS_BUILD_WINDOW=OFF builds zs without the window (headless modes and generators only)
# replay viewer: --stage6, --stage7, --group or --run with --trace trace.json, then python3 tools/viewer/embed.py trace.json replay.html [--max-mb 12] (open in a browser; --max-mb thins long runs)
```

Work is done locally on Windows ([0052](docs/decisions/0052-one-executable-one-simulation.md)): run these from a "x64 Native Tools Command Prompt for VS 2022". Golden hashes in the tests are pinned on MSVC; other compilers must reproduce them (cross-compiler determinism, [spec 01](docs/tier0/specs/01-quantity.md)); the Windows CI build is an extra check.

Layout: `engine/sim` is the simulation library (no window), `engine/gen` the content generators (names, pops, towns; data in `data/gen/`), `apps/zs` the one executable (`zs`: the window, the headless modes, `zs gen`), `docs/tier0` the plan and specs.
