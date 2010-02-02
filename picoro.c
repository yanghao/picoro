/*
 * picoro: minimal coroutines in pure C
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
	static void *it; it = arg;
	prev = here; here = next;
	if(!setjmp(prev->buf)) longjmp(next->buf, 1);
	return(it);
}

void coroutine1(void), coroutine2(void*);

coro coroutine(int (*fun)(void*)) {
	if(!idle && !setjmp(here->buf)) coroutine1();
	return(coto(idle, &fun));
}

void coroutine1(void) {
	char dummy[16*1024];
	coroutine2(dummy);
}

void coroutine2(void *dummy) {
	int (*fun)(void*), (**pfn)(void*);
	struct coro me, *back = here;
	idle = here = &me;
	pfn = coto(back, dummy);
	fun = *pfn; back = prev;
	if(!setjmp(here->buf)) coroutine1();
	exit(fun(coto(back, &me)));
}

/* eof */
