project(PDCurses)

cmake_minimum_required(VERSION 3.0)

if (MINGW)
    add_definitions(-DHAVE_NO_INFOEX)
endif()
if (WIN32 OR MSYS OR CYGWIN OR MINGW OR MSVC)
    set (SOURCE_FILES
        pdcurses/addch.c
        pdcurses/addchstr.c
        pdcurses/addstr.c
        pdcurses/attr.c
        pdcurses/beep.c
        pdcurses/bkgd.c
        pdcurses/border.c
        pdcurses/clear.c
        pdcurses/color.c
        pdcurses/debug.c
        pdcurses/delch.c
        pdcurses/deleteln.c
        pdcurses/getch.c
        pdcurses/getstr.c
        pdcurses/getyx.c
        pdcurses/inch.c
        pdcurses/inchstr.c
        pdcurses/initscr.c
        pdcurses/inopts.c
        pdcurses/insch.c
        pdcurses/insstr.c
        pdcurses/instr.c
        pdcurses/kernel.c
        pdcurses/keyname.c
        pdcurses/mouse.c
        pdcurses/move.c
        pdcurses/outopts.c
        pdcurses/overlay.c
        pdcurses/pad.c
        pdcurses/panel.c
        pdcurses/printw.c
        pdcurses/refresh.c
        pdcurses/scanw.c
        pdcurses/scr_dump.c
        pdcurses/scroll.c
        pdcurses/slk.c
        pdcurses/termattr.c
        pdcurses/touch.c
        pdcurses/util.c
        pdcurses/window.c
        wincon/pdcclip.c
        wincon/pdcdisp.c
        wincon/pdcgetsc.c
        wincon/pdckbd.c
        wincon/pdcscrn.c
        wincon/pdcsetsc.c
        wincon/pdcutil.c
    )
    set (HEADER_FILES
        curses.h
        curspriv.h
        wincon/pdcwin.h
    )

    add_library(pdcurses
        ${SOURCE_FILES}
        ${HEADER_FILES}
    )

    target_include_directories(pdcurses
            PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

endif()