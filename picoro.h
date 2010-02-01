/*
 * picoro - minimal coroutines for C.
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef PICORO_H
#define PICORO_H

typedef struct coro *coro;

coro coroutine(int fun(void *));

coro corunning(void);

void *coto(coro c, void *arg);

#endif /* PICORO_H */
