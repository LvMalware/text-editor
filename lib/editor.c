#include "editor.h"

int cx2rx(row_t *row, int cx)
{
    int j, rx = 0;
    for (j = 0; j < cx; j++)
    {
        if (row->text[j] == '\t')
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx ++;
    }
    return rx;
}

int rx2cx(row_t *row, int rx)
{
    int cx, cur_rx = 0;
    for (cx = 0; cx < row->size; cx++)
    {
        if (row->text[cx] == '\t')
            cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
        cur_rx ++;
        if (cur_rx > rx)
            return cx;
    }
    return cx;
}


state_t *
editor_new ()
{
    state_t *state = (state_t *) calloc(1, sizeof(state_t));
    if (window_size(&state->screen_rows, &state->screen_cols) == -1)
        error("%s(): can't get window size.", __func__); //error condition
    state->screen_rows -= 2;
    return state;
}


void
editor_find_callback (state_t *state, char *query, int key)
{
    static int direction = 1;
    static int last_match = -1;

    static int saved_hl_line;
    static char *saved_hl = NULL;

    if (saved_hl)
    {
        memcpy(state->rows[saved_hl_line].hlt,
            saved_hl, state->rows[saved_hl_line].rsize);
        free(saved_hl);
        saved_hl = NULL;
    }

    if (key == '\r' || key == '\x1b')
    {
        direction = 1;
        last_match = -1;
        return;
    }
    else if (key == ARROW_RIGHT || key == ARROW_DOWN)
        direction = 1;
    else if (key == ARROW_LEFT || key == ARROW_UP)
        direction = -1;
    else
    {
        direction = 1;
        last_match = -1;
    }

    if (last_match == -1)
        direction = 1;
    int current = last_match;
    int i;
    for (i = 0; i < state->numrows; i++)
    {
        current += direction;
        if (current == -1)
            current = state->numrows - 1;
        else if (current == state->numrows)
            current = 0;
        
        row_t *row = &state->rows[current];
        char *match = strstr(row->render, query);
        if (match)
        {
            last_match = current;
            state->cy = current;
            state->cx = rx2cx(row, match - row->render);
            state->roffset = state->numrows;

            saved_hl_line = current;
            saved_hl = malloc(row->rsize);
            memcpy(saved_hl, row->hlt, row->rsize);
            memset(&row->hlt[match - row->render], HL_MATCH, strlen(query));
            break;
        }
    }
}

void
editor_find(state_t *state)
{
    int cursor_x = state->cx;
    int cursor_y = state->cy;
    int col_offset = state->coffset;
    int row_offset = state->roffset;

    char *query = editor_prompt(state, "Search: %s (Use ESC/Arrows/Enter)",
        editor_find_callback);
    if (query)
    {
        free(query);
    }
    else
    {
        state->cx = cursor_x;
        state->cy = cursor_y;
        state->coffset = col_offset;
        state->roffset = row_offset;
    }
}

void
editor_save (state_t *state)
{
    if (!state->filename)
    {
        state->filename = editor_prompt(state,
                        "Save as: %s (ESC to calcel)", NULL);
        if (state->filename == NULL)
        {
            set_status_message(state, "Save aborted.");
            return;
        }
        select_syntax(state);
    }
    char tmp[] = ".tmpfileXXXXXX";
    int fd = mkstemp(tmp);
    
    ssize_t total = 0;
    if (fd != -1)
    {
        unsigned i;
        for (i = 0; i < state->numrows; i ++)
        {
            row_t *row = &state->rows[i];
            size_t count = write(fd, row->text, row->size);
            if (count != row->size)
            {
                close(fd);
                set_status_message(state, "I/O error: %s", strerror(errno));
                return;
            }
            count += write(fd, "\n", 1);
            total += count;
        }
        
        close(fd);
        
        if (access(state->filename, F_OK) != -1)
            remove(state->filename);
        
        if (rename(tmp, state->filename) != -1)
        {
            state->dirty = 0;
            set_status_message(state, "%d bytes written to disk.", total);
            return;
        }
    }
    set_status_message(state, "Can't save! I/O error: %s", strerror(errno));
}

void
editor_scroll (state_t *state)
{
    state->rx = 0;
    if (state->cy < state->numrows)
        state->rx = cx2rx(&state->rows[state->cy], state->cx);
    
    if (state->cy < state->roffset)
        state->roffset = state->cy;

    if (state->cy >= state->roffset + state->screen_rows)
        state->roffset = state->cy - state->screen_rows + 1;
    
    if (state->rx < state->coffset)
        state->coffset = state->rx;

    if (state->rx >= state->coffset + state->screen_cols)
        state->coffset = state->rx - state->screen_cols + 1;

}

void
select_syntax (state_t *state)
{
    state->syntax = NULL;
    if (state->filename == NULL)
        return ;
    
    char *ext = strrchr(state->filename, '.');
    
    for (unsigned int j = 0; j < HLDB_ENTRIES; j ++)
    {
        syntax_t *s = &HLDB[j];
        unsigned int i = 0;
        while (s->filematch[i])
        {
            int is_ext = (s->filematch[i][0] == '.');
            if ((is_ext && ext && !strcmp(ext, s->filematch[i]))||
                (!is_ext && strstr(state->filename, s->filematch[i])))
            {
                state->syntax = s;
                
                int filerow;
                for (filerow = 0; filerow < state->numrows; filerow ++)
                {
                    update_syntax(state, &state->rows[filerow]);
                }

                return;
            }
            i ++;
        }
    }
}

void
refresh_screen (state_t *state)
{
    editor_scroll(state);

    buff_t *buf = buff_new();
    
    buff_append(buf, "\x1b[?25l", 6);
    buff_append(buf, "\x1b[H", 3);
    
    draw_rows(state, buf);
    draw_status_bar(state, buf);
    draw_message_bar(state, buf);
    
    char str[32];
    snprintf(str, sizeof(str), "\x1b[%d;%dH",
            (state->cy - state->roffset) + 1,
            (state->rx - state->coffset) + 1);
    
    buff_append(buf, str, strlen(str));

    buff_append(buf, "\x1b[?25h", 6);
    
    write(STDOUT_FILENO, buf->data, buf->size);
    
    buff_free(buf); 
}

void
update_syntax (state_t *state, row_t *row)
{
    syntax_t *syntax = state->syntax;
    if (!syntax)
            return;
    
    int repeat = 1;

    while (repeat)
    {
        set_status_message(state, "Updating row %d/%d", row->index, state->numrows);
        if (row->hlt)
            row->hlt = (unsigned char *) realloc(row->hlt, row->rsize);
        else
            row->hlt = (unsigned char *) malloc(row->rsize);
        memset(row->hlt, HL_NORMAL, row->rsize);
        
        char **keywords = syntax->keywords;
        
        char *scs = syntax->singleline_comment_start;
        char *mcs = syntax->ml_comment_start;
        char *mce = syntax->ml_comment_end;

        int scs_len = scs ? strlen(scs) : 0;
        int mcs_len = mcs ? strlen(mcs) : 0;
        int mce_len = mce ? strlen(mce) : 0;

        int i = 0, prev_sep = 1, in_string = 0;
        int in_comment = (row->index > 0 &&
                         state->rows[row->index - 1].comment);

        while (i  < row->rsize)
        {
            char c = row->render[i];
            unsigned prev_hl = (i > 0) ? row->hlt[i - 1] : HL_NORMAL;
            if (scs_len && ! in_string && !in_comment)
            {
                if (!strncmp(&row->render[i], scs, scs_len))
                {
                    memset(&row->hlt[i], HL_COMMENT, row->rsize - i);
                    break;
                }
            }
            if (mcs_len && mce_len && !in_string)
            {
                if (in_comment)
                {
                    row->hlt[i] = HL_COMMENT;
                    if (!strncmp(&row->render[i], mce, mce_len))
                    {
                        memset(&row->hlt[i], HL_MLCOMMENT, mce_len);
                        i += mce_len;
                        in_comment = 0;
                        prev_sep = 1;
                        continue;
                    }
                    i ++;
                    continue;
                }
                else if (!strncmp(&row->render[i], mcs, mcs_len))
                {
                    memset(&row->hlt[i], HL_MLCOMMENT, mcs_len);
                    i += mcs_len;
                    in_comment = 1;
                    continue;
                }
            }

            if (syntax->flags & HL_HIGHLIGHT_STRINGS)
            {
                if (in_string)
                {
                    row->hlt[i] = HL_STRING;
                    if (c == '\\' && i + 1 < row->rsize)
                    {
                        row->hlt[i + 1] = HL_STRING;
                        i += 2;
                        continue;
                    }
                    if (c == in_string)
                        in_string = 0;
                    i ++;
                    prev_sep = 1;
                    continue;
                }
                else if (c == '"' || c == '\'')
                {
                    in_string = c;
                    row->hlt[i] = HL_STRING;
                    i ++;
                    continue;
                }
            }
            if (syntax->flags & HL_HIGHLIGHT_NUMBERS)
            {
                if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
                    (c == '.' && prev_hl == HL_NUMBER))
                {
                    row->hlt[i] = HL_NUMBER;
                    i ++;
                    prev_sep = 0;
                    continue;
                }
            }

            if (prev_sep)
            {
                int j;
                for (j = 0; keywords[j]; j++)
                {
                    int klen = strlen(keywords[j]);
                    int kw2 = (keywords[j][klen - 1] == '|');
                    if (kw2)
                        klen --;
                    if (!strncmp(&row->render[i], keywords[j], klen) && 
                        is_separator(row->render[i + klen]))
                    {
                        memset(&row->hlt[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1,
                            klen);
                        i += klen;
                        break;
                    }
                }
                if (keywords[j] != NULL)
                {
                    prev_sep = 0;
                    continue;
                }
            }

            prev_sep = is_separator(c);
            i ++;
        }
        
        repeat = (row->comment != in_comment) &&
                 (row->index + 1 < state->numrows);
        row->comment = in_comment;
        if (repeat)
            row = &state->rows[row->index + 1];
        //if (repeat)
        //    error("%s(): repeat %d", __func__, repeat);
    }
}

void
insert_row(state_t *state, int index, char *text, int len)
{
    if (index < 0 || index > state->numrows)
        return;
    
    state->rows = realloc(state->rows, sizeof(row_t) * (state->numrows + 1));
    set_status_message(state, "allocating a new row");
    memmove(&state->rows[index + 1], &state->rows[index],
        sizeof(row_t) * (state->numrows - index));
    int j;
    for (j = index + 1; j <= state->numrows; j++)
        state->rows[j].index ++;

    state->rows[index].index = index;

    state->rows[index].size = len;
    state->rows[index].text = malloc(len + 1);
    memcpy(state->rows[index].text, text, len);
    state->rows[index].text[len] = '\0';
    
    state->rows[index].hlt = NULL;
    state->rows[index].rsize = 0;
    state->rows[index].render = NULL;
    state->rows[index].comment = 0;
    update_row(&state->rows[index]);
    
    update_syntax(state, &state->rows[index]);

    state->numrows ++;
    state->dirty ++;

}

void
editor_new_line (state_t *state)
{
    if (state->cx == 0)
        insert_row(state, state->cy, "", 0);
    else
    {
        /*
         * watch out for errors here... (what if cy = numrows ?)
         */
        row_t *row = &state->rows[state->cy];
        insert_row(state, state->cy + 1,
                  &row->text[state->cx], row->size - state->cx);
        row = &state->rows[state->cy];
        row->size = state->cx;
        row->text[row->size] = '\0';
        update_row(row);
        update_syntax(state, row);
    }
    state->cy ++;
    state->cx = 0;
}

void
delete_row(state_t *state, int index)
{
    if (index < 0 || index >= state->numrows)
        return;

    free_row(&state->rows[index]);
    memmove(&state->rows[index], &state->rows[index + 1],
            sizeof(row_t) * ((state->numrows - index) - 1));
    state->rows = (row_t *) realloc(state->rows,
                                    sizeof(row_t) * (state->numrows - 1));
    int j;
    for (j = index; j < state->numrows - 1; j ++)
        state->rows[j].index --;
    state->numrows --;
    state->dirty ++;
}

void
editor_del_char (state_t *state)
{
    if (state->cy >= state->numrows)
        return;
    if (state->cx == 0 && state->cy == 0)
        return;

    state->dirty ++;

    row_t *row = &state->rows[state->cy];
    if (state->cx > 0)
    {
        row_delete_char(row, state->cx - 1);
        state->cx --;
        return;
    }

    state->cx = state->rows[state->cy - 1].size;
    row_append_string(&state->rows[state->cy - 1], row->text, row->size);
    delete_row(state, state->cy); //delete_row needs state, so it should be defined here
    state->cy --;

}

void
move_cursor (state_t *state, int key)
{
    row_t *row = (state->cy > state->numrows) ? NULL : &state->rows[state->cy];
    switch (key)
    {
        case ARROW_LEFT:
            if (state->cx != 0)
                state->cx --;
            else if (state->cy > 0)
            {
                state->cy --;
                state->cx = state->rows[state->cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && state->cx < row->size)
                state->cx ++;
            else if (row && state->cx == row->size)
            {
                if (state->cy < state->numrows)
                {
                    state->cy ++;
                    state->cx = 0;
                }
            }
            break;
        case ARROW_UP:
            if (state->cy != 0)
                state->cy --;
            break;
        case ARROW_DOWN:
            if (state->cy < state->numrows)
                state->cy ++;
            break;
    }
    row = (state->cy > state->numrows) ? NULL : &state->rows[state->cy];
    int len = row ? row->size : 0;
    if (state->cx > len)
        state->cx = len;
}

void
editor_open (state_t *state, char *filename)
{
    if (state->filename)
        free(state->filename);
    state->filename = strdup(filename);

    select_syntax(state);

    /*
     * NOTE: here we are opening the file using "a+" as mode so the file will
     * be created if it doesn't exist already.
     */
    FILE *fp = fopen(filename, "a+");

    if (!fp)
        error("%s(): can't open %s for reading", __func__, filename);
    
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while((linelen = getline(&line, &linecap, fp)) != -1)
    {
        if (linelen != -1)
        {
            while (linelen > 0 && (line[linelen - 1] == '\n' ||
                                line[linelen - 1] == '\r'))
            {    
                linelen --;
            }
            insert_row(state, state->numrows, line, linelen);
        }
    }
    free(line);
    fclose(fp);
    state->dirty = 0;
}

void
draw_rows (state_t *state, buff_t *buf)
{
    int y;
    for (y = 0; y < state->screen_rows; y ++)
    {
        int filerow = y + state->roffset;
        if (filerow < state->numrows)
        {
            int len = state->rows[filerow].rsize - state->coffset;
            if (len < 0)
                len = 0;
            if (len > state->screen_cols)
                len = state->screen_cols;
            char *c = &state->rows[filerow].render[state->coffset];
            unsigned char *hl = &state->rows[filerow].hlt[state->coffset];
            int current_color = -1;
            int j;
            for (j = 0; j < len; j++)
            {
                if (iscntrl(c[j]))
                {
                    char sym = (*c <= 26) ? '@' : '?';
                    buff_append(buf, "\x1b[7m", 4);
                    buff_append(buf, &sym, 1);
                    buff_append(buf, "\x1b[m", 3);
                    if (current_color != -1)
                    {
                        char str[16];
                        int clen = snprintf(str, sizeof(str),
                            "\x1b[%dm", current_color);
                        buff_append(buf, str, clen);
                    }
                }
                else if (hl[j] == HL_NORMAL)
                {
                    if (current_color != -1) {
                        buff_append(buf, "\x1b[39m", 5);
                        current_color = -1;
                    }
                    buff_append(buf, &c[j], 1);
                }
                else
                {
                    int color = syntax_to_color(hl[j]);
                    if (color != current_color)
                    {
                        current_color = color;
                        char str[16];
                        int clen = snprintf(str, sizeof(str), "\x1b[%dm", color);
                        buff_append(buf, str, clen);
                    }
                    buff_append(buf, &c[j], 1);
                }
            }
            buff_append(buf, "\x1b[39m", 5);
        }
        buff_append(buf, "\x1b[K", 3);
        buff_append(buf, "\r\n", 2);
    }
}

void
draw_status_bar (state_t *state, buff_t *buf)
{
    buff_append(buf, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
        state->filename ? state->filename : "[No name]", state->numrows,
        state->dirty ? "(modified)" : "");
    if (len > state->screen_cols)
        len = state->screen_cols;
    int rlen = snprintf(rstatus, sizeof(rstatus), "%s %d/%d",
        state->syntax ? state->syntax->filetype : "no ft",
        state->cy + 1, state->numrows);
    
    buff_append(buf, status, len);
    while (len < state->screen_cols)
    {
        if (state->screen_cols - len == rlen)
        {
            buff_append(buf, rstatus, rlen);
            break;
        }
        buff_append(buf, " ", 1);
        len ++;
    }
    buff_append(buf, "\x1b[m", 3);
    buff_append(buf, "\r\n", 2);

}

void
draw_message_bar (state_t *state, buff_t *buf)
{
    buff_append(buf, "\x1b[K", 3);
    int msglen = strlen(state->status_msg);
    if (msglen > state->screen_cols)
        msglen = state->screen_cols;
    if (msglen && time(NULL) - state->status_time < 5)
    {
        buff_append(buf, state->status_msg, msglen);
    }
}

void
set_status_message (state_t *state, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(state->status_msg, sizeof(state->status_msg), fmt, ap);
    va_end(ap);
    state->status_time = time(NULL);
}

char *
editor_prompt (state_t *state, char *prompt,
                    void (*callback) (state_t *, char *, int))
{
    size_t buffsize = 128;
    char *buf = (char *) malloc(buffsize);
    size_t buflen = 0;
    buf[0] = '\0';
    while (1)
    {
        set_status_message(state, prompt, buf);
        refresh_screen(state);
        
        int c = read_key();

        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE)
        {
            if (buflen != 0)
                buf[-- buflen] = '\0';
        }
        else if (c == '\x1b')
        {
            set_status_message(state, "");
            if (callback)
                callback(state, buf, c);
            free(buf);
            return NULL;
        }
        else if (c == '\r')
        {
            if (buflen != 0)
            {
                set_status_message(state, "");
                return buf;
            }
        }
        else if (!iscntrl(c) && c < 128)
        {
            if (buflen == buffsize - 1)
            {
                buffsize *= 2;
                buf = realloc(buf, buffsize);
            }
            buf[buflen ++] = c;
            buf[buflen] = '\0';
        }
        if (callback)
            callback(state, buf, c);
    }
}

void
editor_insert_char (state_t *state, int c)
{
    if (state->cy == state->numrows)
    {
        insert_row(state, state->numrows, "", 0);
    }
    row_insert_char(&state->rows[state->cy], state->cx, c);
    state->cx ++;
    state->dirty ++;
    update_syntax(state, &state->rows[state->cy]);
}

void
process_keypress (state_t *state)
{
    static int quit_presses = QUIT_TIMES;
    int c = read_key();
    switch (c)
    {
        case '\r':
            editor_new_line(state);
            break;
        case CTRL_KEY('q'):
            if (state->dirty && quit_presses > 0)
            {
                set_status_message(state,
                                   "WARNING!!! File has unsaved changes. "
                                   "Press Ctrl+q %d more times to quit.",
                                   quit_presses);
                quit_presses --;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case CTRL_KEY('s'):
            editor_save(state);
            break;

        case HOME_KEY:
            state->cx = 0;
            break;
        case END_KEY:
            if (state->cy < state->numrows)
                state->cx = state->rows[state->cy].size;
            break;
        case CTRL_KEY('f'):
            editor_find(state);
            break;
        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY)
                move_cursor(state, ARROW_RIGHT);
            editor_del_char(state);
            break;
        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (c == PAGE_UP)
                {
                    state->cy = state->roffset;
                }
                else if (c == PAGE_DOWN)
                {
                    state->cy = state->roffset + state->screen_rows - 1;
                    if (state->cy > state->numrows)
                        state->cy = state->numrows;
                }

                int times = state->screen_rows;
                while (times --)
                {
                    move_cursor(state, c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                }
            }
            break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            move_cursor(state, c);
            break;
        case CTRL_KEY('l'):
        case '\x1b':
            break;
        default:
            editor_insert_char(state, c);
    }
    quit_presses = QUIT_TIMES;
}