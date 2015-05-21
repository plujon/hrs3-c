#ifndef __tm_time_c__
#define __tm_time_c__

#include "tm_time.h"
#include <string.h>

int tm_time_diff(a_time *later, a_time *earlier)
{
  time_t t1 = tm_time_time(later);
  time_t t2 = tm_time_time(earlier);
  return t1 - t2;
}

time_t tm_time_time(a_time *t)
{
  if (!t->time) {
#if TEST
    if (!t->tm.tm_year)
      BUG();
#endif
    t->time = mktime(&t->tm);
  }
  return t->time;
}

struct tm *tm_time_tm(a_time *t)
{
#if TEST
    if (!t->time)
      BUG();
#endif
  if (!t->tm.tm_year) {
    localtime_r(&t->time, &t->tm);
  }
  return &t->tm;
}

a_time *tm_time_min(a_time *a, a_time *b)
{
  time_t time_a = tm_time_time(a);
  time_t time_b = tm_time_time(b);
  return time_a < time_b ? a : b;
}

a_time *tm_time_max(a_time *a, a_time *b)
{
  time_t time_a = tm_time_time(a);
  time_t time_b = tm_time_time(b);
  return time_a < time_b ? b : a;
}

void tm_time_init_tm(a_time *t, struct tm *tm)
{
  t->time = 0;
  memcpy(&t->tm, tm, sizeof(struct tm));
}

void tm_time_init_time(a_time *t, time_t time)
{
  t->time = time;
  memset(&t->tm, 0, sizeof(struct tm));
}

#if TEST
#include "test.c"
static int test_tm_time()
{
  time_t time1 = time(0);
  time_t time2 = time1 + 1;
  a_time t1, t2;
  tm_time_init_time(&t1, time1);
  struct tm tm2;
  localtime_r(&time2, &tm2);
  tm_time_init_tm(&t2, &tm2);
  if (1 != tm_time_diff(&t2, &t1))
    TFAIL();
  return 0;
}

int main()
{
  return test_tm_time();
}
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o tm_time tm_time.c && ./tm_time"
 * End:
 */

#endif /* __tm_time_c__ */

