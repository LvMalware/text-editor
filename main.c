#include "lib/editor.h"

int
main(int argc, char *argv[])
{
    raw_mode();
    state_t *editor = editor_new();
    if (argc >= 2)
        editor_open(editor, argv[1]);
    set_status_message(editor,
                       "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
    while (1)
    {
        refresh_screen(editor);
        process_keypress(editor);
    }
    return 0;
}