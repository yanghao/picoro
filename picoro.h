/*
 * picoro - minimal coroutines for C.
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef PICORO_H
#define PICORO_H

typedef struct coro *coro;

coro coroutine(void *fun(void *arg));

bool resumable(coro c);

void *resume(coro c, void *arg);

void *yield(void *arg);

#endif /* PICORO_H */
