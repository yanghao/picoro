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

/*
 * How much space to allow for coroutine stacks and the main stack.
 */
#define COROSTACK (1<<12)
#define MAINSTACK (1<<12)

/*
 * Coroutines that are running or idle are on a list.
 * Each coroutine has a jmp_buf to hold its context when suspended.
 */
struct coro {
	struct coro *next;
	jmp_buf state;
};

/*
 * A place to hold the yield/resume argument while switching stacks.
 */
static void *yarg;

/*
 * The C stack is divided into chunks each of which is used as an
 * independent stack for a coroutine. We keep a top-of-stack pointer
 * (which is actually the base of the newest coroutine's stack) to use
 * when calculating where to allocate the next stack chunk.
 */
static char *tos;

/*
 * The list of running coroutines. The coroutine at the head of the
 * list has the CPU, and all others are suspended inside resume().
 * The "first" coro object holds the context for the program's
 * initial stack and also ensures that all list elements have
 * non-NULL next pointers.
 */
static struct coro first, *running = &first;

/*
 * The list of idle coroutines that are suspended in coroutine_main().
 * The "last" coro object just marks end of list so that all
 * list elements have non-NULL next pointers.
 */
static struct coro last, *idle = &last;

/*
 * A coroutine can be passed to resume() if
 * it is not on the running or idle lists.
 */
bool resumable(coro c) {
	return(c->next == NULL);
}

/*
 * Add a coroutine to a list and return the previous head of the list.
 */
static inline coro push(coro *list, coro c) {
	c->next = *list;
	*list = c;
	return(c->next);
}

/*
 * Remove a coroutine from a list and return it.
 */
static inline coro pop(coro *list) {
	coro c = *list;
	*list = c->next;
	c->next = NULL;
	return(c);
}

/*
 * Pass a value and control from one coroutine to another.
 * The current coroutine's state is saved in "me" and the
 * target coroutine is at the head of the "running" list.
 */
static void *pass(coro me, void *arg) {
	yarg = arg;
	if(!setjmp(me->state))
		longjmp(running->state, 1);
	return(yarg);
}

void *resume(coro c, void *arg) {
	assert(resumable(c));
	return(pass(push(&running, c), arg));
}

void *yield(void *arg) {
	return(pass(pop(&running), arg));
}

/*
 * The main loop for a coroutine is responsible for managing the "idle" list.
 *
 * We start off in the running state with a function to call. We immediately
 * yield a pointer to our context object so our creator can identify us. The
 * creator can then resume us at which point we pass the argument to the
 * function to start executing.
 *
 * When the function returns, we move ourself from the running list to
 * the idle list, before passing the result back to the resumer. (This
 * is just like yield() except for adding the coroutine to the idle
 * list.) We can then only be resumed by the coroutine() constructor
 * function which will put us back on the running list and pass us a
 * new function to call.
 *
 * We do not declare coroutine_main() static to try to stop it being inlined.
 *
 * The conversion between the function pointer and a void pointer is not
 * allowed by ANSI C but we do it anyway.
 */
void coroutine_main(void *fun(void *arg)) {
	struct coro me;
	push(&running, &me);
	for(;;) {
		void *ret = fun(yield(&me));
		push(&idle, pop(&running));
		fun = pass(&me, ret);
	}
}

/*
 * If an idle coroutine is available, activate it, otherwise create a new one.
 *
 * The way alloca() works is to subtract its argument from the stack pointer.
 * The first time we are called we move the stack pointer down by MAINSTACK.
 * Subsequent times we move it to approximately tos - COROSTACK. The address
 * of a local variable approximates the stack pointer.
 */
coro coroutine(void *fun(void *arg)) {
	if(idle != &last)
		return(resume(pop(&idle), fun));
	/* Save our state to be yielded to from coroutine_main(). */
	if(setjmp(running->state))
		return(yarg);
	/* Move stack pointer and save new top-of-stack */
	tos = alloca(tos ? (char*)&fun - tos + COROSTACK : MAINSTACK);
	/* Create coroutine's main call frame on new stack. */
	coroutine_main(fun);
	abort();
}

/* eof */
