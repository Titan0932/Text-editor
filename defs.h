#ifndef   DEFS_H
#define   DEFS_H

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#define CTRL_KEY(k) ((k) & 0x1f)

#define HIDE_CURSOR "\x1b[?25l"
#define SHOW_CURSOR "\x1b[?25h"

#define CLEAR_SCREEN "\x1b[2J"
#define REPOSITION_CURSOR_TL "\x1b[H"
#define CLEAN_LINE "\x1b[K"

struct editorConfig {
    int cx, cy;                         // tracks cursor position 
    int screenrows;
    int screencols;
    struct termios orig_termios;          // stores terminal state
};
struct editorConfig E;

enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME,
  END,
  DELETE
};


// append buffer //
struct abuf {
  char *b;
  int len;
  int capacity;
};
#define ABUF_INIT {NULL, 0, 0}

void die(const char *s);

void disableRawMode();
void enableRawMode();

/* input */
int editorReadKey();
void editorProcessKeypress();
void editorMoveCursor(int key);

/* output */
void editorDrawRows(struct abuf *ab);
void editorRefreshScreen();
void clearScreen();
void repositionCursor();

int getWindowSize(int *rows, int *cols);
void initEditor();

/* Append buffer*/
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

#endif