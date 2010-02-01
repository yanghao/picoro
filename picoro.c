/*
 * picoro - minimal coroutines for C.
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>

#include "picoro.h"

static struct coro {
	jmp_buf state;
} first, *running = &first, *idle;

void *coto(coro dst, void *arg) {
	static void *saved;
	coro src = running;
	running = dst;
	saved = arg;
	if(!setjmp(src->state))
		longjmp(dst->state, 1);
	return(saved);
}

void coroutine_start(void), coroutine_main(void*);

coro coroutine(int fun(coro)) {
	if(idle == NULL && !setjmp(running->state))
		coroutine_start();
	return(coto(idle, fun));
}

void coroutine_start(void) {
	char stack[16 * 1024];
	coroutine_main(stack);
}

void coroutine_main(void *stack) {
	int (*fun)(coro);
	struct coro me;
	idle = &me;
	fun = coto(running, stack);
	if(!setjmp(running->state))
		coroutine_start();
	exit(fun(&me));
}

/* eof */
