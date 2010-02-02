/*
 * picoro - minimal coroutines for C.
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include <setjmp.h>
#include <stdlib.h>

#include "picoro.h"

static struct coro {
	jmp_buf buf;
} first, *here = &first, *prev, *idle;

coro corunning(void) {
	return(here);
}

void *coto(coro next, void *arg) {
	static void *saved; saved = arg;
	prev = here; here = next;
	if(!setjmp(prev->buf)) longjmp(next->buf, 1);
	return(saved);
}

void coroutine_new(void), coroutine_starter(void *);

coro coroutine(int (*fun)(void *)) {
	if(!idle && !setjmp(here->buf)) coroutine_new();
	return(coto(idle, &fun));
}

void coroutine_new(void) {
	char dummy[16 * 1024];
	coroutine_starter(dummy);
}

void coroutine_starter(void *dummy) {
	int (*fun)(void *), (**pfn)(void *);
	struct coro me, *back = here;
	idle = here = &me;
	pfn = coto(back, dummy);
	fun = *pfn; back = prev;
	if(!setjmp(here->buf)) coroutine_new();
	exit(fun(coto(back, &me)));
}

/* eof */
