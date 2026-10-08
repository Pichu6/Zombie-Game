// Built without the window (ZS_BUILD_WINDOW=OFF): the headless modes and the generators still work.
#include <cstdio>

#include "modes.h"

int window_main(int, char**) {
  std::fputs("zs: built without the window (ZS_BUILD_WINDOW=OFF); see zs --help for the headless modes\n", stderr);
  return 2;
}
