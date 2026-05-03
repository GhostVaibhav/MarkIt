#include "LogoPanel.h"

#include "AppConfig.h"

LogoPanel::LogoPanel(WINDOW* w, int x, int y) : Panel(w, x, y) {}

void LogoPanel::render() {
  wattron(win, COLOR_PAIR(3));
  mvwprintw(win, x + 1, y, R"(   __  ___)");
  mvwprintw(win, x + 2, y, R"(  /  |/  /)");
  mvwprintw(win, x + 3, y, R"( / /|_/ /)");
  mvwprintw(win, x + 4, y, R"(/_/  /_/)");
  wattroff(win, COLOR_PAIR(3));
  wattron(win, COLOR_PAIR(4));
  mvwprintw(win, x + 2, y + 10, R"(__ _)");
  mvwprintw(win, x + 3, y + 9, R"( _ `)");
  mvwprintw(win, x + 4, y + 8, R"(\_,_)");
  wattroff(win, COLOR_PAIR(4));
  wattron(win, COLOR_PAIR(4));
  mvwprintw(win, x + 2, y + 14, R"(____)");
  mvwprintw(win, x + 3, y + 13, R"(/ __)");
  mvwprintw(win, x + 4, y + 12, R"(/_/ )");
  wattroff(win, COLOR_PAIR(4));
  wattron(win, COLOR_PAIR(2));
  mvwprintw(win, x + 1, y + 19, R"(__    )");
  mvwprintw(win, x + 2, y + 18, R"(/ /__ )");
  mvwprintw(win, x + 3, y + 17, R"(/  '_/)");
  mvwprintw(win, x + 4, y + 16, R"(/_/\_\)");
  wattroff(win, COLOR_PAIR(2));
  wattron(win, COLOR_PAIR(5));
  mvwprintw(win, x + 1, y + 25, R"(____)");
  mvwprintw(win, x + 2, y + 24, R"(/  _)");
  mvwprintw(win, x + 3, y + 23, R"(_/ /)");
  mvwprintw(win, x + 4, y + 22, R"(/___/)");
  wattroff(win, COLOR_PAIR(5));
  wattron(win, COLOR_PAIR(5));
  mvwprintw(win, x + 1, y + 29, R"(__  )");
  mvwprintw(win, x + 2, y + 28, R"(/ /_)");
  mvwprintw(win, x + 3, y + 27, R"(/ __)");
  mvwprintw(win, x + 4, y + 27, R"(\__)");
  wattroff(win, COLOR_PAIR(5));
  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, x + 1, y + 33, R"(__)");
  mvwprintw(win, x + 2, y + 32, R"(/ /)");
  mvwprintw(win, x + 3, y + 31, R"(/_/)");
  mvwprintw(win, x + 4, y + 30, R"((v))");
  AppConfig config;
  mvwprintw(win, x + 4, y + 33, "%s", config.version.c_str());
  wattroff(win, COLOR_PAIR(1));
}
