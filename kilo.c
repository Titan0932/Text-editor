#include "defs.h"




void die(const char *s){
    clearScreen();
    repositionCursor();
    perror(s);
    exit(1);
}


void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)    // returns -1 when error
    die("tcsetattr");
}

void editorDrawRows(struct abuf *ab) {
  for (int y = 0; y < E.screenrows-1; y++) {
    abAppend(ab, "~", 1);
    abAppend(ab, CLEAN_LINE, 3);
    if (y < E.screenrows - 1) {
        abAppend(ab, "\r\n", 2);
    }
  }
}



int getCursorPosition(int *rows, int *cols) {
    char buff[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
    while(i< sizeof(buff) - 1){
        if(read(STDIN_FILENO, &buff[i], 1) != 1) break;
        if(buff[i] == 'R') break;
        ++i;
    }
    buff[i] = '\0';

    printf("\r\n&buf[1]: '%s'\r\n", &buff[1]);

    if (buff[0] != '\x1b' || buff[1] != '[') return -1;
    if (sscanf(&buff[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}


int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) { 
    // fallback
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;  
    return getCursorPosition(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void enableRawMode(){
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) // reads the terminal attributes
        die("tcgetattr");; 

    atexit(disableRawMode); 

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT |           
                        INPCK | 
                            ISTRIP |
                                ICRNL |        // prevent translation of carriage returns into newline
                                    IXON);         // prevent flow control from being stopped via ctrl + s
    raw.c_cflag |= (CS8);
    
    raw.c_oflag &= ~(OPOST);        // prevent translation of \n to \r\n

    raw.c_lflag &= ~(ECHO |         // ECHO returns the user input back in the terminal for user to see what was typed (we dont want this) | 
                        ICANON |              // ICANON exits as soon as q is pressed
                            ISIG    |            // prevents exit signal being sent with ctrl + C and ctrl + z 
                                IEXTEN);            // Disables ctrl + V        
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;


    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)      // sets the terminal attributes
        die("tcsetattr");

}


char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)     // EAGAIN is for timouts: we don't want to exit at timeouts, only at errors
        die("read");

    // if (iscntrl(c)) {
    //   printf("%d\r\n", c);
    // } else {
    //   printf("%d ('%c')\r\n", c, c);
    // }
  }
  return c;
}

void clearScreen(){
  write(STDOUT_FILENO, CLEAR_SCREEN, 4);       // clear screen at each keypress
}

void repositionCursor(){
  write(STDOUT_FILENO, REPOSITION_CURSOR_TL, 3);        // reposition cursor to top left
}

void editorRefreshScreen() {
    struct abuf ab = ABUF_INIT;
    abAppend(&ab, HIDE_CURSOR, 6);      // hide cursor
    abAppend(&ab, REPOSITION_CURSOR_TL, 3);         // reposition cursor topleft

    editorDrawRows(&ab);

    abAppend(&ab, REPOSITION_CURSOR_TL, 3);
      abAppend(&ab, SHOW_CURSOR, 6);        // unhide cursor
    // printf("%s", ab.b);
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}       


void editorProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
        clearScreen();
        repositionCursor();
        exit(0);
        break;
  }
}


void initEditor() {
  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}


void abAppend(struct abuf *ab, const char *s, int len) {
    if(ab->capacity < ab->len + len){
        int newCap = (ab->capacity << 1) + 1;
        char* new = (char*)realloc(ab->b, newCap);
        if (new == NULL) return;
        ab->capacity= newCap;
        ab->b = new;
    }
    memcpy(&ab->b[ab->len], s, len);
    ab->len += len;
}

void abFree(struct abuf *ab) {
  free(ab->b);
}


int main(){
    enableRawMode();
    initEditor();
    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    };
    return 0;
}