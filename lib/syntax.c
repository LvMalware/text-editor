#include "syntax.h"

int
is_separator (int c)
{
    return isspace(c) || (c == '\0') || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

int
syntax_to_color (int class)
{
    switch (class)
    {
        case HL_COMMENT:
        case HL_MLCOMMENT:
            return 36;
        case HL_KEYWORD1:
            return 33;
        case HL_KEYWORD2:
            return 32;
        case HL_STRING:
            return 35;
        case HL_NUMBER:
            return 31;
        case HL_MATCH:
            return 32;
        default:
            return 37;
    }
}