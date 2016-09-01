#ifndef __util_h__
#define __util_h__

#include "time.h"

a_time beginning_of_day(const a_time *t);
a_time beginning_of_week(const a_time *t);
void dd_to_s(char *buffer, int decimal);
int s_to_d(const char *s, size_t len, char **endptr);
char *strnchr(const char *s, size_t len, char c);
void remove_char(char *s, char c);

#endif /* __util_h__ */
