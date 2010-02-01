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
 * Coroutines that are running or idle are on a list.
 * Each coroutine has a jmp_buf to hold its context when suspended.
 */
struct coro {
	struct coro *next;
	jmp_buf state;
};

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
 * The "first" coro object holds the context for the program's initial
 * stack and also ensures that all externally-visible list elements
 * have non-NULL next pointers. (The "first" coroutine isn't exposed
 * to the caller.)
 */
static struct coro first, *running = &first;

/*
 * The list of idle coroutines that are suspended in coroutine_main().
 */
static struct coro *idle;

/*
 * A coroutine can be passed to resume() if
 * it is not on the running or idle lists.
 */
bool resumable(coro c) {
	return(c != NULL && c->next == NULL);
}

/*
 * Add a coroutine to a list and return the previous head of the list.
 */
static inline void push(coro *list, coro c) {
	c->next = *list;
	*list = c;
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
static void *pass(coro me, void arg) {
	static void *saved;
	saved = arg;
	if(!setjmp(me->state))
		longjmp(running->state, 1);
	return(saved);
}

void *resume(coro c, void *arg) {
	assert(resumable(c));
	push(&running, c);
	return(pass(c->next, arg));
}

void *yield(void *arg) {
	return(pass(pop(&running), arg));
}

/* Declare for mutual recursion. */
void coroutine_start(void);

/*
 * The coroutine constructor function.
 *
 * On the first invocation there are no idle coroutines, so fork the
 * first one, which will immediately yield back to us after becoming
 * idle. When there are idle coroutines, we pass one the function
 * pointer and return the activated coroutine's address.
 */
coro coroutine(void *fun(void *arg)) {
	if(idle == NULL && !setjmp(running->state))
		coroutine_start();
	return(resume(pop(&idle), fun));
}

/*
 * The main loop for a coroutine is responsible for managing the "idle" list.
 *
 * When we start the idle list is empty, so we put ourself on it to
 * ensure it remains non-NULL. Then we immediately suspend ourself
 * waiting for the first function we are to run. (The head of the
 * running list is the coroutine that forked us.) We pass the stack
 * pointer to prevent it from being optimised away. The first time we
 * are called we will return to the fork in the coroutine()
 * constructor function (above); on subsequent calls we will resume
 * the parent coroutine_main(). In both cases the passed value is
 * lost when pass() longjmp()s to the forking setjmp().
 *
 * When we are resumed, the idle list is empty again, so we fork
 * another coroutine. When the child coroutine_main() passes control
 * back to us, we drop into our main loop.
 *
 * We are now head of the running list with a function to call. We
 * immediately yield a pointer to our context object so our creator
 * can identify us. The creator can then resume us at which point we
 * pass the argument to the function to start executing.
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
void coroutine_main(void *stack) {
	struct coro me;
	push(&idle, &me);
	void *(*fun)(void *) = pass(&me, stack);
	if(!setjmp(running->state))
		coroutine_start();
	for(;;) {
		void *ret = fun(yield(&me));
		push(&idle, pop(&running));
		fun = pass(&me, ret);
	}
}

/*
 * Allocate space for the current stack to grow before creating the
 * initial stack frame for the next coroutine.
 */
void coroutine_start(void) {
	char stack[16 * 1024];
	coroutine_main(stack);
}

/* eof */
