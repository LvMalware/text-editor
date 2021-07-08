#include "term.h"

void
raw_mode ()
{
    static int enabled = 0;
    static struct termios original;
    if (!enabled)
    {
        if (tcgetattr(STDIN_FILENO, &original) == -1)
            error("%s(): can't enable raw mode! tcsetattr: %s",
                  __func__, strerror(errno));
        struct termios raw = original;
        raw.c_cflag |= (CS8);
        raw.c_oflag &= ~(OPOST);
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        enabled = 1;
        atexit(raw_mode);
    }
    else
    {
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original) == -1)
            error("%s(): can't restore from raw mode! tcsetattr: %s",
                  __func__, strerror(errno));
    }
}

int
read_key ()
{
    char c;
    int nread;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            return 0; //should displau an error message and exit
    }
    if (c == '\x1b') //escape
    {
        char sequence[3];
        if (read(STDIN_FILENO, &sequence[0], 1) != 1)
            return c;
        if (read(STDIN_FILENO, &sequence[1], 1) != 1)
            return c;
        if (sequence[0] == '[')
        {

            if (sequence[1] >= '0' && sequence[1] <= '9') //numpad keys
            {
                if (read(STDIN_FILENO, &sequence[2], 1) != 1)
                    return c;
                if (sequence[2] == '~')
                {
                    switch (sequence[1])
                    {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            }
            else
            {
                switch (sequence[1])
                {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'F': return END_KEY;
                    case 'H': return HOME_KEY;
                }
            }
        }
        else if (sequence[0] == 'O')
        {
            switch (sequence[1])
            {
                case 'F': return END_KEY;
                case 'H': return HOME_KEY;
            }
        }
    }
    return c;
}

int
cursor_position(int *rows, int *cols)
{
    char buffer[32];
    int i = 0;
    if (write(STDIN_FILENO, "\x1b[6n", 4) != 4)
        return -1;
    while (i < sizeof(buffer))
    {
        if (read(STDIN_FILENO, &buffer[i], 1) != 1)
            break;
        if (buffer[i] == 'R')
            break;
        i ++;
    }
    buffer[i] = 0;
    if (buffer[0] != '\x1b' || buffer[1] != '[')
        return -1;
    if (sscanf(&buffer[2], "%d;%d", rows, cols) != 2)
        return -1;
    
    return 0;
}

int
window_size (int *rows, int *cols)
{
    struct winsize ws;
    if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return cursor_position(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
