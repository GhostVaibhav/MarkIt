/*
 *      __  ___         __    ____ __  __
 *     /  |/  /__ _____/ /__ /  _// /_/ /
 *    / /|_/ / _ `/ __/  '_/_/ / / __/_/
 *   /_/  /_/\_,_/_/ /_/\_\/___/ \__(_)
 *
 *   MIT License
 *
 *   Copyright (c) 2021 Vaibhav Sharma
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 *  ----------------------------------------------------------------------
 *  -------------------------PANELS FOR PDCurses--------------------------
 *  ----------------------------------------------------------------------
 */

#ifndef __PDCURSES_PANEL_H__
#define __PDCURSES_PANEL_H__ 1

#include <curses.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct panelobs {
  struct panelobs *above;
  struct panel *pan;
} PANELOBS;

typedef struct panel {
  WINDOW *win;
  int wstarty;
  int wendy;
  int wstartx;
  int wendx;
  struct panel *below;
  struct panel *above;
  const void *user;
  struct panelobs *obscure;
} PANEL;

PDCEX int bottom_panel(PANEL *pan);
PDCEX int del_panel(PANEL *pan);
PDCEX int hide_panel(PANEL *pan);
PDCEX int move_panel(PANEL *pan, int starty, int startx);
PDCEX PANEL *new_panel(WINDOW *win);
PDCEX PANEL *panel_above(const PANEL *pan);
PDCEX PANEL *panel_below(const PANEL *pan);
PDCEX int panel_hidden(const PANEL *pan);
PDCEX const void *panel_userptr(const PANEL *pan);
PDCEX WINDOW *panel_window(const PANEL *pan);
PDCEX int replace_panel(PANEL *pan, WINDOW *win);
PDCEX int set_panel_userptr(PANEL *pan, const void *uptr);
PDCEX int show_panel(PANEL *pan);
PDCEX int top_panel(PANEL *pan);
PDCEX void update_panels(void);

#ifdef __cplusplus
}
#endif

#endif