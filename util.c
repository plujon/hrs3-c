#ifndef __util_c__
#define __util_c__

#include "impl.h"

a_time beginning_of_day(a_time *t)
{
  a_time ret;
  struct tm *tm = ttime_tm(t);
  int seconds_into_day = tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
  time_t time = ttime_time(t);
  time -= seconds_into_day;
  ttime_init(&ret, time);
  return ret;
}

a_time beginning_of_week(a_time *t)
{
  a_time ret;
  struct tm *tm = ttime_tm(t);
  int seconds_into_week = tm->tm_wday * 3600 * 24 +
    tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
  time_t time = ttime_time(t);
  time -= seconds_into_week;
  ttime_init(&ret, time);
  return ret;
}

char *strnchr(const char *s, size_t len, char c)
{
  const char *p = s, *end = s + len;
  for (; p < end; ++p) {
    if (c == p[0])
      return (char *)p;
  }
  return 0;
}

#if RUN_TESTS
int test_beginning_of_day()
{
  a_time now = time_now();
  a_time t = beginning_of_day(&now);
  struct tm *tm = ttime_tm(&t);
  struct tm *now_tm = ttime_tm(&now);
  if (tm->tm_hour) TFAIL();
  if (tm->tm_min) TFAIL();
  if (tm->tm_sec) TFAIL();
  if (tm->tm_wday != now_tm->tm_wday) TFAIL();
  return 0;
}

int test_beginning_of_week()
{
  a_time now = time_now();
  a_time t = beginning_of_week(&now);
  struct tm *tm = ttime_tm(&t);
  if (tm->tm_wday) TFAIL();
  if (tm->tm_hour) TFAIL();
  if (tm->tm_min) TFAIL();
  if (tm->tm_sec) TFAIL();
  return 0;
}

void __attribute__((constructor)) test_util()
{
  test_beginning_of_day();
  test_beginning_of_week();
}

#if ONE_OBJ
#include "tm_time.c"
#include "main.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o util util.c && ./util"
 * End:
 */
#endif

#endif /* __util_c__ */
