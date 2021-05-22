#include <cstdlib>
#include <curses.h>

//----------------------------------------------------------------------------
struct curses
{
  curses()
  {
    // Win32a PDCurses -- make window resizable
    ttytype[0] = 25;                  // min  25 lines
    ttytype[1] = (unsigned char)255;  // max 255 lines
    ttytype[2] = 80;                  // min  80 columns
    ttytype[3] = (unsigned char)255;  // max 255 columns
    
    initscr();
    noecho();
    nonl();
    raw();
    keypad( stdscr, TRUE );  // (required for resizing to work on Win32a ???)
    curs_set( 0 );

    #ifndef _WIN32
    // Make the ESC key take only a little time,
    // unless the user has set a specific ESCDELAY in their environment.
    const char* escdelay = std::getenv( "ESCDELAY" );
    if (escdelay) ESCDELAY = std::atoi( escdelay );
    else          ESCDELAY = 250;
    #endif
  }
  
 ~curses()
  {
    endwin();
  }
};

curses _;

//----------------------------------------------------------------------------
void resize_event()
{
  int w, h;
  resize_term( 0, 0 );
  getmaxyx( stdscr, h, w );
  clear();
  border(ACS_VLINE,ACS_VLINE,ACS_HLINE,ACS_HLINE,ACS_LRCORNER,ACS_URCORNER,ACS_ULCORNER,ACS_LLCORNER);
  refresh();
}

//----------------------------------------------------------------------------
int main()
{
  printw( "Resize your window!\n\rPress either ENTER or ESC to quit." );
  resize_event();
  
  bool done = false;
  while (!done)
  {
    int ch = getch();
    switch (ch)
    {
      case KEY_RESIZE: 
        resize_event(); 
        break;
      
      case '\033':
      case '\n':
      case '\r':
        done = true; 
        break;
    }
  }
  
  mvprintw( 10, 10, "Press the 'any' key                  " );
  refresh();
  getch();
}