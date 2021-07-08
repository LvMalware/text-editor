#include "syntax.h"

/* syntaxes */

/* C/C++ */
char *C_HL_extensions[] = { ".c", ".h", ".cpp", ".hpp", NULL };

char *C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "extern", "class", "case", "struct", "union", "typedef", "static",
    "enum", "#include", "#define", "#if", "#endif", "#ifdef", "#ifndef",
    "int|", "long|", "double|", "float|", "char|", "const|", "volatile|", 
    "unsigned|", "signed|", "short|", "void|", "NULL|", NULL
};

/* Perl */

char *Perl_HL_extensions[] = { ".pl", ".pm", ".PM", ".PL", NULL };
char *Perl_HL_keywords[] = {
    "chomp", "chop", "chr", "crypt", "hex", "index", "lc", "lcfirst", "caller",
    "length", "oct", "ord", "pack", "reverse", "rindex", "sprintf", "unless",
    "substr", "uc", "ucfirst", "pos", "quotemeta", "split", "print", "sub",
    "abs", "exp", "int", "long", "rand", "srand", "each", "for", "keys", 
    "pop", "push", "shift", "splice", "unshift", "grep", "join", "map", 
    "sort", "unpack", "delete", "exists", "values", "binmode", "close",
    "die", "eof", "printf", "say", "select", "break", "if", "next", "return",
    "last", "exit", "continue", "kill", "fork", "exec", "system", "waitpid",
    "lock", "scalar", "do", "require", "use", "ref", "package", "eval",
    "undef|", "@_|", "$_|", "%_|", "strict|", "warnings|", "$self|", NULL
};

/* HLDB */

syntax_t HLDB[] = {
    {
        "C/C++",
        C_HL_extensions,
        C_HL_keywords,
        "//", "/*", "*/",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
    {
        "Perl",
        Perl_HL_extensions,
        Perl_HL_keywords,
        "#", NULL, NULL,
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    }
};

int HLDB_ENTRIES = 2;

/* functions */

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