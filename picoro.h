/*
 * picoro: minimal coroutines in pure C
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef PICORO_H
#define PICORO_H

typedef struct coro *coro;
typedef int (*corofun)(coro, va_list);

va_list coroutine(corofun, ...);
va_list coto(coro, ...);
coro cohere(void);

#endif /* PICORO_H */
