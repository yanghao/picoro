/*
 * picoro: minimal coroutines in pure C
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>

#include "picoro.h"

static struct coro {
	jmp_buf buf;
} first, *here = &first, *idle;

coro cohere(void) { return(here); }

va_list coto(coro next, ...) {
	static va_list ap; va_start(ap, next);
	coro prev = here; here = next;
	if(!setjmp(prev->buf)) longjmp(next->buf, 1);
	return(ap);
}

void coroutine1(void), coroutine2(void*);

va_list coroutine(corofun fun, ...) {
	va_list ap; va_start(ap, fun);
	if(!idle && !setjmp(here->buf)) coroutine1();
	return(coto(idle, fun, ap));
}

void coroutine1(void) {
	char dummy[16*1024];
	coroutine2(dummy);
}

void coroutine2(void *dummy) {
	struct coro me, *prev = here; here = idle = &me;
	va_list ap1 = coto(prev, dummy);
	corofun fun = va_arg(ap1, corofun);
	va_list ap2 = va_arg(ap1, va_list);
	if(!setjmp(here->buf)) coroutine1();
	exit(fun(here, ap2));
}

/* eof */
