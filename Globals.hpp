#pragma once

#include "libutils/src/CLIParser.hpp"
#include "Routine.hpp"
#include <string>

struct Files {
  std::string home_dir, routine_dir;          // directories
  std::string routines_json, info_json, logs; // files
};

class Globals {
public:
  std::string VERSION = "26.9.13";
  bool running = true;
  int selected = 0;
  Files files;
  CLIParser parser;
  RoutineManager routine_manager;

  static Globals &getInstance() {
    static Globals g;
    return g;
  }

  Globals(const Globals &) = delete;
  Globals(Globals &&) = delete;
  Globals &operator=(const Globals &) = delete;
  Globals &operator=(Globals &&) = delete;

private:
  Globals() = default;
}; // Singleton class Globals