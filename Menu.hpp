#pragma once

#include <functional>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

class Menu {
protected:
  std::vector<std::string> msgs_list;
  int selected = 0;

public:
  Menu(const std::vector<std::string> &msgs_list) : msgs_list(msgs_list) {}

  void print() const {
    for (size_t i = 0; i < msgs_list.size(); i++) {
      if ((int)i == selected) {
        std::cout << "" << bg_rgb(200, 100, 150) << ">" << msgs_list[i]
                  << "\x1b[0m"
                  << "\n";
      } else {
        std::cout << " " << msgs_list[i] << "\n";
      }
    }
  }

  int run(const std::string &cover = "") {
    std::string key;
    while (key != "\n") {
      clearTerminal();
      std::cout << cover << "\n";
      print();
      key = getKeyPress();
      if (key == "\033[A") { // UP ARROW
        // selected = std::max(0, --selected);
        selected--;
      } else if (key == "\033[B") { // DOWN ARROW
        // selected = std::min((int)msgs_list.size() - 1, ++selected);
        selected++;
      }
      selected %= (int)msgs_list.size();
    }
    return selected;
  }

  template <typename... Args>
  int run(std::function<void(Args... args)> printingFunc, Args... args) {
    std::string key;
    while (key != "\n") {
      clearTerminal();
      printingFunc(args...);
      print();
      key = getKeyPress();
      if (key == "\033[A") { // UP ARROW
        // selected = std::max(0, --selected);
        selected--;
      } else if (key == "\033[B") { // DOWN ARROW
        // selected = std::min((int)msgs_list.size() - 1, ++selected);
        selected++;
      }
      selected %= (int)msgs_list.size();
    }
    return selected;
  }

  int run(std::function<void()> printingFunc) {
    std::string key;
    while (key != "\n") {
      clearTerminal();
      printingFunc();
      print();
      key = getKeyPress();
      if (key == "\033[A") { // UP ARROW
        // selected = std::max(0, --selected);
        selected--;
      } else if (key == "\033[B") { // DOWN ARROW
        // selected = std::min((int)msgs_list.size() - 1, ++selected);
        selected++;
      }
      selected %= (int)msgs_list.size();
    }
    return selected;
  }

  int run(std::function<void()> printingFunc, const std::string &cover) {
    std::string key;
    while (key != "\n") {
      clearTerminal();
      printingFunc();
      std::cout << cover << "\n";
      print();
      key = getKeyPress();
      if (key == "\033[A") { // UP ARROW
        // selected = std::max(0, --selected);
        selected--;
      } else if (key == "\033[B") { // DOWN ARROW
        // selected = std::min((int)msgs_list.size() - 1, ++selected);
        selected++;
      }
      selected %= (int)msgs_list.size();
    }
    return selected;
  }

protected:
  void clearTerminal() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
  }

  /// newt.c_lflag &= ~(ICANON | ECHO);
  std::string getKeyPress() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode + echo
    newt.c_lflag &= ~(ICANON | ECHO);

    // Set read timeout: 100ms max per byte
    // newt.c_cc[VTIME] = 1; // 100ms timeout
    // newt.c_cc[VMIN] = 0;  // Don't wait for minimum bytes

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string sequence;
    unsigned char ch;

    // Read first byte
    if (read(STDIN_FILENO, &ch, 1) > 0) {
      sequence += ch;

      // If it's ESC, try to read escape sequence
      if (ch == '\033') {
        unsigned char buf[10];
        size_t bytesRead = read(STDIN_FILENO, buf, 10);
        for (size_t i = 0; i < bytesRead; i++) {
          sequence += buf[i];
        }
      }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return sequence;
  }

  std::string bg_rgb(int r, int g, int b) const {
    return "\x1b[48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" +
           std::to_string(b) + "m";
  }
};