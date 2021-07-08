#include "buffer.h"

buff_t *
buff_new ()
{
    return (buff_t *) calloc(1, sizeof(buff_t));
}

void
buff_append (buff_t *buf, char *data, size_t len)
{
    if (!buf->data)
        buf->data = malloc(len + 1);
    else
        buf->data = realloc(buf->data, buf->size + len + 1);
    memcpy(&buf->data[buf->size], data, len);
    buf->size += len;
    buf->data[buf->size] = '\0';
}

void
buff_free (buff_t *buf)
{
    if (buf->data)
        free(buf->data);
    buf->data = NULL;
    buf->size = 0;
    free(buf);
}