#include "row.h"

void
free_row (row_t *row)
{
    if (!row)
        return;
    free(row->text);
    if (row->hlt)
        free(row->hlt);
    if (row->render)
        free(row->render);
    row->size = 0;
    row->text = row->hlt = row->render = NULL;
}

void
update_row (row_t *row)
{
    int j, i = 0, tabs = 0;
    for (j = 0; j < row->size; j++)
        if (row->text[j] == '\t')
            tabs ++;
    if (row->render)
        free(row->render);
    row->render = (char *) malloc(row->size + tabs * (TAB_STOP - 1) + 1);
    for (j = 0; j < row->size; j ++)
    {
        if (row->text[j] == '\t')
        {
            row->render[i ++] = ' ';
            while (i % TAB_STOP != 0)
                row->render[i ++] = ' ';
        }
        else
            row->render[i ++] = row->text[j];
    }

    row->render[i] = '\0';
    row->rsize = i;
    //update_syntax(row)
}

void
row_delete_char (row_t *row, int index)
{
    if (index < 0 || index > row->size)
        return;
    memmove(&row->text[index], &row->text[index + 1], row->size - index);
    row->size --;
    update_row(row);
    //state->dirty ++;
}

void
row_insert_char (row_t *row, int index, int c)
{
    if (index < 0 || index > row->size)
        return;

    row->text = (char *) realloc(row->text, row->size + 2);
    memmove(&row->text[index + 1], &row->text[index], row->size - index + 1);
    row->size ++;
    row->text[index] = c;
    update_row(row);
    //state->dirty ++;
}

void
row_insert_string (row_t *row, int index, char *str, size_t len)
{
    if (index < 0 || index > row->size)
        return;
    
    row->text = (char *) realloc(row->text, row->size + len + 1);
    memmove(&row->text[index + len], & row->text[index], row->size - index + 1);
    memcpy(&row->text[index], str, len);
    row->size += len;
    row->text[row->size] = '\0';
    update_row(row);
    //state->dirty ++;
}

void
row_append_string (row_t *row, char *str, size_t len)
{
    row_insert_string(row, row->size, str, len);
    //state->dirty ++;
}

void
row_delete_interval (row_t *row, int index, size_t count)
{
    if (index > row->size)
        return;
    if (index + count > row->size)
        count = row->size - index;
    if (count == 0)
        return ;
    memmove(&row->text[index], &row->text[index + count - 1], count);
    row->size -= count;
    row->text = (char *) realloc(row->text, row->size + 1);
    row->text[row->size] = '\0';
    update_row(row);
    //state->dirty ++;
}

