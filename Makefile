CFLAGS = -pipe -O -g -std=c89 -pedantic -Wall -Wextra \
	-Wbad-function-cast -Wcast-align -Wcast-qual -Wconversion -Winline \
	-Wmissing-declarations -Wmissing-prototypes -Wnested-externs \
	-Wold-style-definition -Wpointer-arith -Wredundant-decls -Wshadow \
	-Wstrict-prototypes -Wunreachable-code -Wwrite-strings

obj:
	gcc ${CFLAGS} -c -o picoro.o picoro.c

count:
	cat picoro.[hc] | egrep -vc '^(#|/\*| \*|	*}*$$)'

up:
	git gc
	git update-server-info
	touch .git/git-daemon-export-ok
	echo "Various little pure C coroutine implementations." >.git/description
	rsync --delete --recursive --links .git/ chiark:public-git/picoro.git/
