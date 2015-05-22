#ifndef __ttime_c__
#define __ttime_c__

#include "impl.h"
#include <string.h>

int ttime_cmp(a_time *a, a_time *b)
{
  int x = ttime_diff(a, b);
  if (x < 0) /* b is bigger than a */ return -1;
  if (0 < x) /* a is bigger than b */ return 1;
  return 0;
}

int ttime_diff(a_time *later, a_time *earlier)
{
  return ttime_time(later) - ttime_time(earlier);
}

a_time *time_incr(a_time *t, int sec)
{
  time_t stamp = ttime_time(t);
  ttime_init(t, stamp + sec);
  return t;
}

time_t ttime_time(a_time *t)
{
  if (!t->time) {
#if CHECK
    if (!t->tm.tm_year)
      BUG();
#endif
    t->time = mktime(&t->tm);
  }
  return t->time;
}

struct tm *ttime_tm(a_time *t)
{
  if (!t->tm.tm_year) {
#if CHECK
    if (!t->time)
      BUG();
#endif
    localtime_r(&t->time, &t->tm);
  }
  return &t->tm;
}

a_time *ttime_min(a_time *a, a_time *b)
{
  time_t time_a = ttime_time(a);
  time_t time_b = ttime_time(b);
  return time_a < time_b ? a : b;
}

a_time *ttime_max(a_time *a, a_time *b)
{
  time_t time_a = ttime_time(a);
  time_t time_b = ttime_time(b);
  return time_a < time_b ? b : a;
}

void ttime_init_tm(a_time *t, struct tm *tm)
{
  t->time = 0;
  if (tm != &t->tm)
    memcpy(&t->tm, tm, sizeof(struct tm));
}

void ttime_init(a_time *t, time_t time)
{
  t->time = time;
  memset(&t->tm, 0, sizeof(struct tm));
}

void time_whms(a_time *t, int wday, int hour, int min, int sec)
{
  struct tm *tm = ttime_tm(t);
  tm->tm_wday = wday;
  tm->tm_hour = hour;
  tm->tm_min = min;
  tm->tm_sec = sec;
  ttime_init_tm(t, tm);
}

a_time time_now()
{
  static a_time now;
  ttime_init(&now, time(0));
  return now;
}

#if RUN_TESTS
static int test_ttime()
{
  time_t time1 = time(0);
  time_t time2 = time1 + 1;
  a_time t1, t2;
  ttime_init(&t1, time1);
  struct tm tm2;
  localtime_r(&time2, &tm2);
  ttime_init_tm(&t2, &tm2);
  if (1 != ttime_diff(&t2, &t1))
    TFAIL();
  return 0;
}

void __attribute__((constructor)) test_tm_time()
{
  test_ttime();
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "main.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o tm_time tm_time.c && ./tm_time"
 * End:
 */

#endif /* __ttime_c__ */

