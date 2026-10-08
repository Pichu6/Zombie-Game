// The modes of the one executable (decision 0052): the window, the headless runs, the content generators.
#pragma once

int window_main(int argc, char** argv);    // window.cpp: the sandbox window (spec 16)
int headless_main(int argc, char** argv);  // headless.cpp: runs without a window, text out
int gen_main(int argc, char** argv);       // gen_cli.cpp: argv[0] is "gen", argv[1] the generator
