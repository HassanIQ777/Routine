#pragma once

#include "json.hpp"
#include "libutils/src/color.hpp"
#include "libutils/src/funcs.hpp"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

using json = nlohmann::json;

class Routine {
  std::string title;
  std::string description;
  bool completed;

public:
  Routine(const std::string &title, const std::string &description,
          bool completed)
      : title(title), description(description), completed(completed) {}

  void setDescription(const std::string &description) {
    this->description = description;
  }
  std::string getDescription() const { return description; }

  void toggleCompleted() { completed = !completed; }
  void setCompleted(bool completed) { this->completed = completed; }
  bool isCompleted() const { return completed; }

  std::string getTitle() const { return title; }

  json toJSON() const {
    return json{{"title", title},
                {"description", description},
                {"completed", completed}};
  }

  static Routine fromJSON(const json &j) {
    return Routine(j["title"], j["description"], j["completed"]);
  }

  void edit(const std::string &new_title, const std::string &new_description) {
    if (new_title.size() > 1) {
      title = new_title;
    }
    if (new_description.size() > 1) {
      description = new_description;
    }
    // we won't be editing completed
  }

  bool operator==(const Routine &other) {
    return other.description == description && other.title == title;
  }
}; // end of class Routine

class RoutineManager {
  std::vector<Routine> routines;

public:
  RoutineManager() {}

  void addRoutine(const Routine &r) { routines.push_back(r); }

  bool removeRoutine(const Routine &r) {
    auto it = std::find(routines.begin(), routines.end(), r);
    if (it == routines.end())
      return false;
    routines.erase(it);
    return true;
  }

  int getRoutineIndex(const Routine &r) {
    for (size_t i = 0; i < routines.size(); i++)
      if (routines[i] == r)
        return (int)i;
    return -1;
  }

  Routine &getRoutineByIndex(int i) { return routines[i]; }

  int size() const { return (int)routines.size(); }
  // returns how many routines have been marked as completed
  int completed() const {
    int completed = 0;
    for (const auto &r : routines) {
      if (r.isCompleted())
        completed++;
    }
    return completed;
  }

  void save(const std::string &filename) {
    json data;
    for (const auto &r : routines)
      data.push_back(r.toJSON());
    std::ofstream file(filename);
    file << data.dump(2);
  }

  void load(const std::string &filename) {
    std::ifstream file(filename);
    json data;
    file >> data;
    routines.clear();
    for (const auto &j : data)
      routines.push_back(Routine::fromJSON(j));
  }

  void printRoutines(int selected) {
    using namespace color;
    using funcs::print;

    // if routines is empty
    if (size() == 0) {
      print(fg_rgb(50, 50, 50));
      print(_ITALIC);
      print("You have no routines...");
      print(_RESET);
      print("\n");
    }

    for (size_t i = 0; i < routines.size(); i++) {
      bool completed = routines[i].isCompleted();
      if (completed) {
        print(_STRIKE_THROUGH);
        print(fg_rgb(50, 50, 50));
        print(i + 1, ". ", routines[i].getTitle());
        print(_RESET);
        print("\n");
      } else if ((int)i == selected) {
        print(_RESET, bg_rgb(120, 120, 120));
        print(i + 1, ". ");
        print(routines[i].getTitle());
        print(_RESET);
        print("\n");
      } else {
        print(_RESET, fg_rgb(0, 255, 255));
        print(i + 1, ". ");
        print(_RESET, fg_rgb(255, 215, 0));
        print(routines[i].getTitle());
        print(_RESET);
        print("\n");
      }
    }
  }

  void checkAndReset(const std::string &info_json) {
    std::ifstream file(info_json);
    json data;
    file >> data;
    file.close();

    time_t ctime = time(0);
    tm *now = localtime(&ctime);

    int last_reset_time = data["last_reset_time"];

    // Thanks claude for the following code!
    // true if we're past 3AM today (in seconds since midnight)
    int seconds_since_midnight =
        now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
    bool past_3am = seconds_since_midnight >= 3 * 3600;

    // last reset was before today's 3AM threshold
    time_t todays_3am = ctime - seconds_since_midnight + 3 * 3600;
    bool already_reset_today = last_reset_time >= todays_3am;

    if (past_3am && !already_reset_today) {
      for (auto &r : routines) {
        r.setCompleted(false);
      }

      data["last_reset_time"] = (int)ctime;
      std::ofstream out(info_json);
      out << data.dump(2);
      printSummary();
    }
  }

  void printSummary() {
    using funcs::print;
    using namespace color; // so unnecessary

    print(TXT_CYAN, " ----- Summary ----- ", _RESET, "\n\n");
    print("You completed ", TXT_GREEN, completed(), "/", size(), _RESET,
          " of the routines.\n");
    funcs::getKeyPress();
  }
}; // end of class RoutineManager