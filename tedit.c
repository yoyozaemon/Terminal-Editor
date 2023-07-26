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
}
