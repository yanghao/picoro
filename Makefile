CFLAGS = -pipe -O -g -std=c99 -pedantic -Wall -Wextra \
	-Wbad-function-cast -Wcast-align -Wcast-qual -Wconversion -Winline \
	-Wmissing-declarations -Wmissing-prototypes -Wnested-externs \
	-Wold-style-definition -Wpointer-arith -Wredundant-decls -Wshadow \
	-Wstrict-prototypes -Wunreachable-code -Wwrite-strings

obj:
	gcc ${CFLAGS} -c -o picoro.o picoro.c

count:
	cat picoro.[hc] | egrep -vc '^(#|/\*| \*|	*}*$$)'
