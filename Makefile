all:
	gcc main.c lib/editor.c lib/row.c lib/syntax.c lib/term.c lib/buffer.c lib/debug.c -o editor -Wall
