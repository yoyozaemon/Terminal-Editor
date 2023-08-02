#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>

/*** Defination ***/

#define TEDIT_VERSION "0.0.1"
#define TAB_STOP 4
#define QUIT_TIMES 2
#define CTRL_KEY(k) ((k) & 0x1f)
#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

enum editorKey{
  BACKSPACE = 127, 
  ARROW_LEFT =  1000, 
  ARROW_RIGHT, 
  ARROW_UP, 
  ARROW_DOWN,
  DEL_KEY, 
  HOME_KEY, 
  END_KEY, 
  PAGE_UP, 
  PAGE_DOWN
};

enum editorHighlight{
  HL_NORMAL = 0, 
  HL_COMMENT, 
  HL_MLCOMMENT, 
  HL_KEYWORD1, 
  HL_KEYWORD2,
  HL_STRING, 
  HL_NUMBER, 
  HL_MATCH
};

/** DATA **/

struct editorSyntax{
  char *filetype;
  char **filematch;
  char **keywords;
  char *singleline_comment_start;
  char *multiline_comment_start;
  char *multiline_comment_end;
  int flags;
};

// Stores the row of the text in editor 
typedef struct erow{
  int idx;
  int size;
  int rsize;
  char *chars;
  char * render;
  unsigned char *hl;
  int hl_open_comment;
} erow;

// Keeps track of global editor state 
struct editorConfig{
  int cx, cy;
  int rx;
  int screenrows;
  int screencols;
  int rowoff;
  int coloff;
  int numrows;
  int dirty;
  erow *row;
  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
  struct editorSyntax *syntax;
  struct termios orig_termios;
};

struct editorConfig E;

char *C_HL_extensions[] = {".c",".h",".cpp",NULL};
char *C_HL_keywords[] = {"switch","if","while","for","break","continue", 
  "struct","union","typedef","static","enum","class","case","return","else", 
  "int|","long|","double|","float|","char|","unsigned|","signed|","void|",NULL
};

struct editorSyntax HLDB[] ={
  {
    "c", 
    C_HL_extensions, 
    C_HL_keywords, 
    "//","/*","*/", 
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
};

#define HLDB_ENTRIES (sizeof(HLDB)) / sizeof(HLDB[0]))

/***Prototypes***/
void editorSetStatusMessage(const char *fmt, ...);
void editorRefreshScreen();
char *editorPrompt(char *prompt, void (*callback)(char *, int));

/***Append buffer***/
struct abuf{
  char *b;
  int len;
};

#define ABUF INIT {NULL,0}

void abAppend(struct abuf *ab, const char *s, int len){
  char *new = realloc(ab->b, ab->len + len);
  if(new == NULL)
    return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len; 
}

void abFree(struct abuf *ab){
  free(ab->b);
}

/***Terminal***/
void die(char *s){
  // write(STDOUT_FILENO, "\x1b[2J",4);
  // write(STDOUT_FILENO, "\x1b[H",3);
  perror(s);
  exit(1);
}

int editorReadKey(){
  int nread;
  char c;
  while((nread = read(STDIN_FILENO,&c,1)) != 1){
    if(nread == -1 && errno != EAGAIN)
      die("read");
  }
  if(c == '\x1b'){
    char seq[3];
    if(read(STDIN_FILENO, &seq[0],1) != 1)
      return '\x1b';
    if(read(STDIN_FILENO, &seq[1],1) != 1)
      return '\x1b';

    if(seq[0] == '['){
      if(seq[1] >= '0' && seq[1] <= '9'){
        if(read(STDIN_FILENO, &seq[2],1) != 1)
          return '\x1b';
        if(seq[2] == '~'){
          switch(seq[1]){
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        } else{
          switch(seq[1]){
            case 'A': return ARROW_UP;
            case 'B': return ARROW_DOWN;
            case 'C': return ARROW_RIGHT;
            case 'D': return ARROW_LEFT;
            case 'H': return HOME_KEY;
            case 'F': return END_KEY;
          }
        } else if(seq[0] == 'O'){
          switch(seq[1]){
            case 'H': return HOME_KEY;
            case 'F': return END_KEY;
          }
        }
        return '\x1b';
      } else {
         return c;
      }
    }
  }
}
int getCursorPosition(int *rows, int *cols){
  char buf[32];
  unsigned int i = 0;
}
