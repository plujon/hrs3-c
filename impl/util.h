#ifndef __util_h__
#define __util_h__

#include "time.h"

a_time beginning_of_day(a_time *t);
a_time beginning_of_week(a_time *t);
char *strnchr(const char *s, size_t len, char c);

#endif /* __util_h__ */
