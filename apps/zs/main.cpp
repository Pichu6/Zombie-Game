// zs: the one executable (decision 0052). No arguments open the Atlas (spec 55, decision 0073); --play or a window
// flag opens the game window; a headless flag runs without one; "gen" runs the content generators; "world" manages
// the world folders (decision 0074).
//
//   zs [--atlas [--shot out.bmp] [--page P] [--stop N]]   the Atlas: worlds, the tour, parts, try, breakthroughs
//   zs --play [--world NAME] [--from SAVE] [--seed N] [--watch] [--shot out.bmp ...]   the window
//   zs --run|--group|--stage6|... [options]         headless (zs --help lists them)
//   zs --day [--seed N] [--country C] [--people N] [--date D]   a town's day hour by hour (spec 54)
//   zs --diverge [--seed N] [--hours H] [--every T] [--mode jump|saveload|both]   the first divergent tick and system (spec 56)
//   zs gen town|census|names|cultures|world ...     content generators
//   zs world list|new|fork|delete ...               worlds as folders
#include <cstring>

#include "day_mode.h"
#include "diverge_mode.h"
#include "modes.h"
#include "worlds.h"

#ifdef ZS_WITH_WINDOW
#include <SDL3/SDL_main.h>
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace {

bool headless_flag(const char* a) {
  static const char* const kFlags[] = {"--run",   "--group",     "--stage6",    "--stage7", "--scripted-pop",
                                       "--body-lab", "--bench",  "--pop-bench", "--town",   "--map",
                                       "--days",  "--save",      "--load",      "--help"};
  for (const char* f : kFlags)
    if (!std::strcmp(a, f)) return true;
  return false;
}

bool window_flag(const char* a) {
  static const char* const kFlags[] = {"--play", "--seed", "--watch", "--shot", "--world", "--from"};
  for (const char* f : kFlags)
    if (!std::strcmp(a, f)) return true;
  return false;
}

// Opened by a double click, the console belongs to this process alone: hide it, the window is the program.
void hide_own_console() {
#ifdef _WIN32
  DWORD ids[2];
  if (GetConsoleProcessList(ids, 2) == 1) FreeConsole();
#endif
}

}  // namespace

int main(int argc, char** argv) {
  if (argc >= 2 && !std::strcmp(argv[1], "gen")) return gen_main(argc - 1, argv + 1);
  if (argc >= 2 && !std::strcmp(argv[1], "world")) return zs::worlds::cli(argc - 1, argv + 1);
  if (argc >= 2 && !std::strcmp(argv[1], "--day")) return zs::atlas::day_main(argc, argv);  // spec 54
  if (argc >= 2 && !std::strcmp(argv[1], "--diverge")) return zs::atlas::diverge_main(argc, argv);  // spec 56
  for (int i = 1; i < argc; ++i)
    if (headless_flag(argv[i])) return headless_main(argc, argv);
  hide_own_console();
  for (int i = 1; i < argc; ++i)
    if (!std::strcmp(argv[i], "--atlas")) return atlas_main(argc, argv);
  for (int i = 1; i < argc; ++i)
    if (window_flag(argv[i])) return window_main(argc, argv);
  return atlas_main(argc, argv);
}
