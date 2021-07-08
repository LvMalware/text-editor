#ifndef _TERM_H
#define _TERM_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "debug.h"

enum term_keys
{
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

#define CTRL_KEY(k) ((k) & 0x1F)

void raw_mode ();

int read_key ();

int cursor_position (int *, int*);

int window_size (int *, int *);


#endif /* term.h */