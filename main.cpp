#include "Globals.hpp"
#include "helpers.hpp"
#include "libutils/src/funcs.hpp"

int main(int argc, char **argv) {
  Globals &g = Globals::getInstance();
  g.parser.setArgs(argc, argv);
  parseArgs(g);
  createFiles(g);
  g.routine_manager.load(g.files.routines_json);
  LOG("User started program");

  while (g.running) {
    funcs::clearTerminal();
    g.routine_manager.checkAndReset(g.files.info_json);

    mainMenu();
    const std::string input = funcs::getKeyPress();
    handleInput(g, input, g.selected);
    g.routine_manager.save(g.files.routines_json);
  }

  funcs::printCentered("Thanks for using Routine!\n");
  funcs::printCentered("By HassanIQ777\n");
}