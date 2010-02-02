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
} first, *here = &first, *prev, *idle;

coro corunning(void) {
	return(here);
}

va_list coto(coro next, ...) {
	static va_list ap; va_start(ap, next);
	prev = here; here = next;
	if(!setjmp(prev->buf)) longjmp(next->buf, 1);
	return(ap);
}

void coroutine1(void), coroutine2(void*);

va_list coroutine(int (*fun)(coro,va_list), ...) {
	va_list ap; va_start(ap, fun);
	if(!idle && !setjmp(here->buf)) coroutine1();
	return(coto(idle, fun, ap));
}

void coroutine1(void) {
	char dummy[16*1024];
	coroutine2(dummy);
}

void coroutine2(void *dummy) {
	struct coro me;
	va_list ap1, ap2;
	int (*fun)(coro,va_list);
	prev = here; here = idle = &me;
	ap1 = coto(prev, dummy);
	fun = va_arg(ap1, int (*)(coro,va_list));
	ap2 = va_arg(ap1, va_list);
	      va_end(ap1);
	if(!setjmp(here->buf)) coroutine1();
	exit(fun(here, ap2));
}

/* eof */
