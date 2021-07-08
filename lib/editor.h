#ifndef _EDITOR_H
#define _EDITOR_H

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <stdarg.h>

#include "row.h"
#include "term.h"
#include "debug.h"
#include "syntax.h"
#include "buffer.h"

#define QUIT_TIMES 3

typedef struct editor_state
{
    int cx;
    int cy;
    int rx;
    int dirty;
    int numrows;
    row_t *rows;
    int roffset;
    int coffset;
    char *filename;
    int screen_rows;
    int screen_cols;
    syntax_t *syntax;
    time_t status_time;
    char status_msg[80];
} state_t;


state_t *editor_new ();

void editor_find (state_t *);

void editor_save (state_t *);

void editor_scroll (state_t *);

void select_syntax (state_t *);

void refresh_screen (state_t *);

void editor_new_line (state_t *);

void editor_del_char (state_t *);

void move_cursor (state_t *, int);

void process_keypress (state_t *);

void editor_open (state_t *, char *);

void draw_rows (state_t *, buff_t *);

int update_syntax (state_t *, row_t *);

void editor_insert_char (state_t *, int);

void draw_status_bar (state_t *, buff_t *);

void draw_message_bar (state_t *, buff_t *);

void set_status_message (state_t *, const char *fmt, ...);

char *editor_prompt (state_t *, char *,
                    void (*callback) (state_t *, char *, int));

#endif /* editor.h */