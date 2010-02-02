/*
 * picoro: minimal coroutines in pure C
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef PICORO_H
#define PICORO_H

typedef struct coro *coro;

coro corunning(void);
coro coroutine(int fun(void*));
void *coto(coro, void*);

#endif /* PICORO_H */
