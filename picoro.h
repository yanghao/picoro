/*
 * picoro: minimal coroutines in pure C
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef PICORO_H
#define PICORO_H

typedef struct coro *coro;

coro corunning(void);
va_list coroutine(int fun(coro,va_list), ...);
va_list coto(coro, ...);

#endif /* PICORO_H */
