#pragma warning(disable : 4244)

#include "ColorScheme.h"

#include <curses.h>

void ColorScheme::apply() const {
#ifdef _WIN32
  if (!has_colors()) return;
#endif
  start_color();
  init_pair(success, static_cast<short>(COLOR_GREEN), static_cast<short>(COLOR_BLACK));
  init_pair(error, static_cast<short>(COLOR_RED), static_cast<short>(COLOR_BLACK));
  init_pair(accent, static_cast<short>(COLOR_BLUE), static_cast<short>(COLOR_BLACK));
  init_pair(highlight, static_cast<short>(COLOR_MAGENTA), static_cast<short>(COLOR_BLACK));
  init_pair(warning, static_cast<short>(COLOR_YELLOW), static_cast<short>(COLOR_BLACK));
  init_pair(normal, static_cast<short>(COLOR_WHITE), static_cast<short>(COLOR_BLACK));
  init_pair(footer, static_cast<short>(COLOR_BLACK), static_cast<short>(COLOR_CYAN));
}
