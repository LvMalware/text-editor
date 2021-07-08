#ifndef _ROW_H
#define _ROW_H

#include <stdlib.h>
#include <string.h>

typedef struct text_row
{
    int index;
    char *text;
    int comment;
    char *render;
    unsigned size;
    unsigned rsize;
    unsigned char *hlt;
} row_t;

#define TAB_STOP 4

void free_row (row_t *);

void update_row (row_t *);

void row_delete_char (row_t *, int);

void row_insert_char (row_t *, int, int);

void row_delete_interval (row_t *, int, size_t);

void row_append_string (row_t *, char *, size_t);

void row_insert_string (row_t *, int, char *, size_t);


#endif /* row.h */