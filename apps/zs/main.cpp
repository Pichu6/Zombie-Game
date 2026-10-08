// zs: the one executable (decision 0052). No arguments, or window flags, open the window; a headless
// flag runs without one; "gen" runs the content generators.
//
//   zs [--seed N] [--watch] [--shot out.bmp ...]   the window
//   zs --run|--group|--stage6|... [options]         headless (zs --help lists them)
//   zs gen town|census|names|cultures|world ...     content generators
#include <cstring>

#include "modes.h"

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
  for (int i = 1; i < argc; ++i)
    if (headless_flag(argv[i])) return headless_main(argc, argv);
  hide_own_console();
  return window_main(argc, argv);
}
