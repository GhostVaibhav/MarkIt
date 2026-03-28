#include "ColorScheme.h"
#include <curses.h>

void ColorScheme::apply() const {
#ifdef _WIN32
    if (!has_colors())
        return;
#endif
    start_color();
    init_pair(success, COLOR_GREEN, COLOR_BLACK);
    init_pair(error, COLOR_RED, COLOR_BLACK);
    init_pair(accent, COLOR_BLUE, COLOR_BLACK);
    init_pair(highlight, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(warning, COLOR_YELLOW, COLOR_BLACK);
    init_pair(normal, COLOR_WHITE, COLOR_BLACK);
    init_pair(footer, COLOR_BLACK, COLOR_CYAN);
}
