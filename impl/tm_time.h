#ifndef __ttime_h__
#define __ttime_h__

#include <time.h>

/*
 * Sometimes we want a timestamp (time_t), and sometimes a struct tm
 * (in localtime).
 */
typedef struct a_time {
  time_t time;  /* 0 means unset */
  struct tm tm; /* a tm_year of 0 means unset */
} a_time;

int ttime_cmp(a_time *a, a_time *b);
int ttime_diff(a_time *later, a_time *earlier);
a_time *time_incr(a_time *t, int sec);
a_time *ttime_min(a_time *a, a_time *b);
a_time *ttime_max(a_time *a, a_time *b);
time_t ttime_time(a_time *t);
struct tm *ttime_tm(a_time *t);
void ttime_init(a_time *t, time_t time);
void ttime_init_tm(a_time *t, struct tm *tm);
void ttime_copy(a_time *dest, a_time *src);
void time_whms(a_time *t, int wday, int hour, int min, int sec);
a_time time_now();

#endif /* __ttime_h__ */
