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

coro corunning(void) {
	return(running);
}

void *coto(coro dst, void *arg) {
	static void *saved;
	coro src = running;
	running = dst;
	saved = arg;
	if(!setjmp(src->state)) longjmp(dst->state, 1);
	return(saved);
}

void coroutine_new(void), coroutine_starter(void*);

coro coroutine(int fun(void *)) {
	if(!idle && !setjmp(running->state)) coroutine_new();
	return(coto(idle, fun));
}

void coroutine_new(void) {
	char stack[16 * 1024];
	coroutine_starter(stack);
}

void coroutine_starter(void *stack) {
	int (*fun)(coro);
	struct coro me, *parent = running;
	running = idle = &me;
	fun = coto(parent, stack);
	if(!setjmp(running->state)) coroutine_new();
	exit(fun(coto(parent, &me)));
}

/* eof */
