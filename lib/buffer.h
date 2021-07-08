#ifndef _buff_H
#define _buff_H

#include <string.h>
#include <stdlib.h>

typedef struct buff
{
    char *data;
    size_t size;
} buff_t;

buff_t *buff_new ();

void buff_append (buff_t *buf, char *data, size_t len);

void buff_free (buff_t *buf);

#endif /* buff.h */