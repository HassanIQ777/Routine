#pragma once

#include "Globals.hpp"
#include "Menu.hpp"
#include "Routine.hpp"
#include "libutils/src/File.hpp"
#include "libutils/src/Log.hpp"
#include "libutils/src/funcs.hpp"
#include "libutils/src/strutils.hpp"
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

using funcs::print;
namespace fs = std::filesystem;

inline void LOG(const std::string &msg) {
  Globals &globals = Globals::getInstance();
  std::string date = funcs::currentTime();
  std::string output = date + " -> " + msg;
  File::insertline(globals.files.logs, output, 0);
}

inline std::string getdate() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t time = std::chrono::system_clock::to_time_t(now);
  const std::tm tm = *std::localtime(&time);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%a %I:%M %p");
  //  this format looks like: Sat 01:09 AM
  return oss.str();
}

inline void printLogo() {
  static constexpr const char *ROUTINE_LOGO = R"(
    ██████╗  ██████╗ ██╗   ██╗████████╗██╗███╗   ██╗███████╗
    ██╔══██╗██╔═══██╗██║   ██║╚══██╔══╝██║████╗  ██║██╔════╝
    ██████╔╝██║   ██║██║   ██║   ██║   ██║██╔██╗ ██║█████╗  
    ██╔══██╗██║   ██║██║   ██║   ██║   ██║██║╚██╗██║██╔══╝  
    ██║  ██║╚██████╔╝╚██████╔╝   ██║   ██║██║ ╚████║███████╗
    ╚═╝  ╚═╝ ╚═════╝  ╚═════╝    ╚═╝   ╚═╝╚═╝  ╚═══╝╚══════╝
)";
  std::string date = getdate();
  funcs::printLeftMiddleRight("", "", date);
  print("\n", ROUTINE_LOGO, "\n\n");
}

inline void mainMenu() {
  Globals &g = Globals::getInstance();
  printLogo();
  g.routine_manager.printRoutines(g.selected);
  print("\n[H] Help\n");
}

inline void printHelp(Globals &globals) {
  const std::string program_name = globals.parser.getArg(0);
  print("Usage:\n");
  print("  ", program_name, " <HOME_DIR>\n");
  print("  ", program_name, " -h    print this help message\n");
  print("  ", program_name, " -v    print version\n");
}

inline void assignPaths(Globals &globals) {
  globals.files.routine_dir = fs::path(globals.files.home_dir) / "Routine";
  globals.files.routines_json =
      fs::path(globals.files.routine_dir) / "routines.json";
  globals.files.info_json = fs::path(globals.files.routine_dir) / "info.json";
  globals.files.logs = fs::path(globals.files.routine_dir) / "logs.txt";
}

inline void parseArgs(Globals &globals) {
  if (int argc = globals.parser.getArgc(); argc != 2) {
    if (argc == 1) {
      Log::error("One argument is required but nothing was provided.", false);
    } else {

      Log::error("One argument is required but " +
                     funcs::str(globals.parser.getArgc() - 1) +
                     " arguments were provided.",
                 false);
    }
    printHelp(globals);
    exit(-1);
  }

  const std::string first_arg = globals.parser.getArg(1);
  if (first_arg == "-h") {
    printHelp(globals);
    exit(0);
  } else if (first_arg == "-v") {
    print("routine version ", globals.VERSION, "\n");
    exit(0);
  }

  if (File::isdirectory(first_arg)) {
    globals.files.home_dir = first_arg;
    assignPaths(globals);
  } else {
    Log::error("The provided path is not a directory.", true);
  }
}

inline bool createFile(const std::string &fp) {
  if (!File::isfile(fp)) {
    if (File::createfile(fp)) {
      LOG("Successfully created '" + fp + "'");
      return true; // we newly created this
    } else {
      LOG("Failed to create '" + fp + "'");
      exit(-3);
    }
  }
  return false; // already created
}

inline void createFiles(Globals &globals) {
  if (std::string dir = globals.files.routine_dir; !File::isdirectory(dir)) {
    if (!File::createdir(dir)) {
      Log::error("Failed to created '" + dir + "'");
      // we can't really log this to logs.txt since it doesn't exist yet
    }
  }

  if (createFile(globals.files.routines_json)) {
    File::appendline(globals.files.routines_json, "[]");
  }
  if (createFile(globals.files.info_json)) {
    // this is trash but it works i guess...
    File::appendline(globals.files.info_json, R"({
  "last_reset_time": 0
})");
  }
  createFile(globals.files.logs);
}

inline void handleInput(Globals &g, std::string input, int &selected) {
  // UP arrow
  const int num_routines = g.routine_manager.size();
  if (input == "\033[A") {
    if (g.routine_manager.size() == 0)
      return;
    selected--;
    selected = std::max(0, selected);
  }

  // DOWN arrow
  else if (input == "\033[B") {
    if (g.routine_manager.size() == 0)
      return;
    selected++;
    selected = std::min(selected, num_routines - 1);
  }

  else if (input == " " || input == "\n") {
    if (g.routine_manager.size() == 0)
      return;
    Routine &r = g.routine_manager.getRoutineByIndex(selected);
    print(r.getTitle(), "\n");
    std::string desc = r.getDescription();
    if (!desc.empty()) {
      print(desc, "\n");
    }
    print("Status: ", r.isCompleted() ? "Completed" : "Not Completed", "\n");
    funcs::getKeyPress();
  }

  // Add new routine
  else if (input == "a") {
    std::string title, description;
    print("Title: ");
    std::getline(std::cin, title);
    if (strutils::trim(title).size() < 1) {
      return;
    }
    print("Description: ");
    std::getline(std::cin, description);
    g.routine_manager.addRoutine(
        Routine(strutils::trim(title), strutils::trim(description), false));
    LOG("Added routine: " + title + "|" + description);
  }

  // mark as completed
  else if (input == "c" || input == "/") {
    if (g.routine_manager.size() == 0)
      return;
    Routine &r = g.routine_manager.getRoutineByIndex(selected);
    r.toggleCompleted();
    LOG("Completed routine " + funcs::str(g.routine_manager.completed()) + "/" +
        funcs::str(g.routine_manager.size()) + ": " + r.getTitle() + "|" +
        r.getDescription());
    if(g.routine_manager.completed() == g.routine_manager.size()){
      LOG("All routines are finished!");
    }
  }

  // Delete selected routine
  else if (input == "d") {
    if (g.routine_manager.size() == 0) {
      Log::warn("You have no routines.\n");
      funcs::getKeyPress();
      return;
    }
    std::vector<std::string> options = {"Confirm", "Go Back"};
    Menu yes_no_menu(options);
    int selection = yes_no_menu.run(
        mainMenu, "\nAre you sure you want to delete this routine?");

    auto selected_option = options[selection];
    if (selected_option == "Go Back") {
      return;
    }
    {
      Routine r = g.routine_manager.getRoutineByIndex(selected);
      LOG("Deleted routine: " + r.getTitle() + "|" + r.getDescription());
    }
    g.routine_manager.removeRoutine(
        g.routine_manager.getRoutineByIndex(selected));

    // deleting decreases RoutineManager::routines.size() down by one,
    // so we need to cap selected just in case
    selected--;
    selected = std::max(0, selected);
  }

  // Edit selected routine
  else if (input == "e") {
    if (g.routine_manager.size() == 0) {
      Log::warn("You have no routines.\n");
      funcs::getKeyPress();
      return;
    }
    std::string title, description;
    print("New title: ");
    std::getline(std::cin, title);

    print("New description: ");
    std::getline(std::cin, description);

    Routine &r = g.routine_manager.getRoutineByIndex(selected);
    LOG("Editing routine: " + r.getTitle() + "|" + r.getDescription());
    r.edit(strutils::trim(title), strutils::trim(description));
    LOG("After edit:" + r.getTitle() + "|" + r.getDescription());
  }

  // Print help
  else if (input == "h") {
    print("[A] Add New Routine", "\n");
    print("[C] Complete", "\n");
    print("[D] Delete", "\n");
    print("[E] Edit", "\n");
    print("[Space] View Info", "\n");
    print("[Q] Quit", "\n");
    funcs::getKeyPress();
  }

  // quit
  else if (input == "q") {
    g.running = false;
    LOG("User quit program");
  }
}
