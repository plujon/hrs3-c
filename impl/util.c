#ifndef __util_c__
#define __util_c__

#include "impl.h"

a_time beginning_of_day(const a_time *t)
{
  a_time ret;
  thyme_copy(&ret, t);
  thyme_hms(&ret, 0, 0, 0);
  return ret;
}

a_time beginning_of_week(const a_time *t)
{
  a_time ret;
  thyme_copy(&ret, t);
  thyme_whms(&ret, 0, 0, 0, 0);
  return ret;
}

int c_to_d(char c)
{
  if ('0' <= c && c <= '9')
    return c - '0';
  return 0;
}

int s_to_d(const char *s, size_t len)
{
  int ret = 0;
  const char *end = s + len;
  for(; s < end; ++s) {
    ret *= 10;
    ret += c_to_d(*s);
  }
  return ret;
}

void dd_to_s(char *buffer, int value)
{
  buffer[0] = '0' + value / 10;
  buffer[1] = '0' + value % 10;
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
void test_beginning_of_day()
{
  a_time t = beginning_of_day(thyme_now());
  const struct tm *tm = thyme_tm(&t);
  const struct tm *now_tm = thyme_tm(thyme_now());
  if (tm->tm_hour) TFAIL();
  if (tm->tm_min) TFAIL();
  if (tm->tm_sec) TFAIL();
  if (tm->tm_wday != now_tm->tm_wday) TFAIL();
}

void test_beginning_of_week()
{
  a_time t = beginning_of_week(thyme_now());
  const const struct tm *tm = thyme_tm(&t);
  if (tm->tm_wday) TFAIL();
  if (tm->tm_hour) TFAIL();
  if (tm->tm_min) TFAIL();
  if (tm->tm_sec) TFAIL();
}

void __attribute__((constructor)) test_util()
{
  test_beginning_of_day();
  test_beginning_of_week();
}

#if ONE_OBJ
#include "thyme.c"
#include "main.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o util util.c && ./util"
 * End:
 */
#endif

#endif /* __util_c__ */
