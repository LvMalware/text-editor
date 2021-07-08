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

static char *C_HL_extensions[] = { ".c", ".h", ".cpp", ".hpp", NULL };

static char *C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "extern", "class", "case", "struct", "union", "typedef", "static",
    "enum", "#include", "#define", "#if", "#endif", "#ifdef", "#ifndef",
    "int|", "long|", "double|", "float|", "char|", "const|", "volatile|", 
    "unsigned|", "signed|", "short|", "void|", "NULL|", NULL
};

static syntax_t HLDB[] = {
    {
        "C/C++",
        C_HL_extensions,
        C_HL_keywords,
        "//", "/*", "*/",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    }
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

#endif /* syntax.h */