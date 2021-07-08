#ifndef _SYNTAX_H
#define _SYNTAX_H

#include <ctype.h>
#include <stddef.h>
#include <string.h>

typedef struct syntax {
    char *filetype;
    char **filematch;
    char **keywords;
    char *singleline_comment_start;
    char *ml_comment_start;
    char *ml_comment_end;
    int flags;
} syntax_t;

/* highlight classes */
enum syntax_highlight
{
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};

/* flags */
#define HL_HIGHLIGHT_NUMBERS 0x01
#define HL_HIGHLIGHT_STRINGS 0x02

/* functions */

int is_separator (int);

int syntax_to_color (int);

/* HLDB */

extern syntax_t HLDB[];

extern int HLDB_ENTRIES;

#endif /* syntax.h */