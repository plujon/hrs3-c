#ifndef __tm_time_h__
#define __tm_time_h__

#include <time.h>

/*
 * Sometimes we want a timestamp (time_t), and sometimes a struct tm
 * (in localtime).
 */
typedef struct a_time {
  time_t time;  /* 0 means unset */
  struct tm tm; /* a tm_year of 0 means unset */
} a_time;

int tm_time_diff(a_time *later, a_time *earlier);
a_time *tm_time_min(a_time *a, a_time *b);
a_time *tm_time_max(a_time *a, a_time *b);
time_t tm_time_time(a_time *t);
struct tm *tm_time_tm(a_time *t);
void tm_time_init_time(a_time *t, time_t time);
void tm_time_init_tm(a_time *t, struct tm *tm);
void tm_time_copy(a_time *dest, a_time *src);

#endif /* __tm_time_h__ */
