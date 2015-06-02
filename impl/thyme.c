#ifndef __thyme_c__
#define __thyme_c__

#include "impl.h"
#include <string.h>

int thyme_cmp(const a_time *a, const a_time *b)
{
  int x = thyme_diff(a, b);
  if (x < 0) /* b is bigger than a */ return -1;
  if (0 < x) /* a is bigger than b */ return 1;
  return 0;
}

int thyme_diff(const a_time *later, const a_time *earlier)
{
  return thyme_time(later) - thyme_time(earlier);
}

a_time thyme_clone(const a_time *src)
{
  return *src;
}

void thyme_copy(a_time *dest, const a_time *src)
{
  memcpy(dest, src, sizeof(a_time));
}

void thyme_incr(a_time *t, int sec)
{
  time_t stamp = thyme_time(t);
  thyme_init(t, stamp + sec);
}

a_time thyme_plus(const a_time *t, int sec)
{
  a_time ret = thyme_clone(t);
  thyme_incr(&ret, sec);
  return ret;
}

time_t thyme_time(const a_time *t)
{
  if (!t->time) {
#if CHECK
    BUG();
#endif
  }
  return t->time;
}

const struct tm *thyme_tm(const a_time *t)
{
  if (!t->tm.tm_year) {
#if CHECK
    if (!t->time)
      BUG();
#endif
    localtime_r(&t->time, (struct tm *)&t->tm);
  }
  return &t->tm;
}

a_time *thyme_min(const a_time *a, const a_time *b)
{
  time_t time_a = thyme_time(a);
  time_t time_b = thyme_time(b);
  return (a_time *)(time_a < time_b ? a : b);
}

a_time *thyme_max(const a_time *a, const a_time *b)
{
  time_t time_a = thyme_time(a);
  time_t time_b = thyme_time(b);
  return (a_time *)(time_a < time_b ? b : a);
}

void thyme_init(a_time *t, time_t time)
{
  if (t->time != time) {
    t->time = time;
    memset(&t->tm, 0, sizeof(struct tm));
  }
}

void thyme_s(a_time *t, int sec)
{
  const struct tm *tm = thyme_tm(t);
  if (tm->tm_sec == sec)
    return;
  thyme_incr(t, sec - tm->tm_sec);
  thyme_s(t, sec);
}

void thyme_ms(a_time *t, int min, int sec)
{
  const struct tm *tm = thyme_tm(t);
  if (tm->tm_min == min) {
    thyme_s(t, sec);
    return;
  }
  thyme_incr(t, 60 * (min - tm->tm_min));
  thyme_ms(t, min, sec);
}

void thyme_hms(a_time *t, int hour, int min, int sec)
{
  const struct tm *tm = thyme_tm(t);
  if (tm->tm_hour == hour) {
    thyme_ms(t, min, sec);
    return;
  }
  thyme_incr(t, 3600 * (hour - tm->tm_hour));
  thyme_hms(t, hour, min, sec);
}

void thyme_whms(a_time *t, int wday, int hour, int min, int sec)
{
  const struct tm *tm = thyme_tm(t);
  if (tm->tm_wday == wday) {
    thyme_hms(t, hour, min, sec);
    return;
  }
  thyme_incr(t, 3600 * 24 * (wday - tm->tm_wday));
  thyme_whms(t, wday, hour, min, sec);
}

void thyme_dhms(a_time *t, int mday, int hour, int min, int sec)
{
  const struct tm *tm = thyme_tm(t);
  if (tm->tm_mday == mday) {
    thyme_hms(t, hour, min, sec);
    return;
  }
  thyme_incr(t, 3600 * 24 * (mday - tm->tm_mday));
  thyme_dhms(t, mday, hour, min, sec);
}

void thyme_mdhms(a_time *t, int mon, int mday, int hour, int min, int sec)
{
  const struct tm *tm = thyme_tm(t);
  if (tm->tm_mon + 1 == mon) {
    thyme_dhms(t, mday, hour, min, sec);
    return;
  }
  thyme_incr(t, 3600 * 24 * (mon - (tm->tm_mon + 1)));
  thyme_mdhms(t, mon, mday, hour, min, sec);
}

void thyme_ymdhms(a_time *t,
                  int year, int mon, int mday,
                  int hour, int min, int sec)
{
  const struct tm *tm = thyme_tm(t);
  if (tm->tm_year + 1900 == year) {
    thyme_mdhms(t, mon, mday, hour, min, sec);
    return;
  }
  thyme_incr(t, 3600 * 24 * 365 * (year - (tm->tm_year + 1900)));
  thyme_ymdhms(t, year, mon, mday, hour, min, sec);
}

int is_leap_year(int nyear)
{
  return 0 == nyear % 4 &&
    (0 != nyear % 100 || 0 == nyear % 1000);
}

int days_in_mon(int nmon, int nyear)
{
  switch (nmon) {
  case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
  case 4: case 6: case 9: case 11: return 30;
  case 2: return is_leap_year(nyear) ? 29 : 28;
  }
  BUG();
  return 0;
}

status thyme_parse(const char *s, size_t len, a_time *out)
{
  memset(out, 0, sizeof(a_time));
  if (len < 4 /* year */ + 2 /* mon */ + 2 /* day */ + 2 /* hour */ + 2 /* min */) {
    return __LINE__;
  }
  int year = s_to_d(s, 4); s += 4;
  int mon = s_to_d(s, 2); s += 2;
  if (mon <= 0 || 13 <= mon) return __LINE__;
  int day = s_to_d(s, 2); s += 2;
  if (day < 0 ||  days_in_mon(mon, year) < day) return __LINE__;
  int hour = s_to_d(s, 2); s += 2;
  if (hour < 0 || 24 <= hour) return __LINE__;
  int minute = s_to_d(s, 2); s += 2;
  if (minute < 0 || 60 <= minute) return __LINE__;
  int second = 0;
  thyme_copy(out, thyme_now());
  thyme_ymdhms(out, year, mon, day, hour, minute, second);
  return OK;
}

/* TODO - Maybe put seconds in buffer too. */
size_t thyme_to_s(char *buffer, const a_time *t)
{
  const struct tm *tm = thyme_tm(t);
  dd_to_s(buffer, ((tm->tm_year + 1900) / 100));
  dd_to_s(&buffer[2], ((tm->tm_year + 1900) % 100));
  dd_to_s(&buffer[4], tm->tm_mon + 1);
  dd_to_s(&buffer[6], tm->tm_mday);
  dd_to_s(&buffer[8], tm->tm_hour);
  dd_to_s(&buffer[10], tm->tm_min);
  return 12;
}

const a_time *thyme_now()
{
  static a_time now;
  if (!now.time)
    thyme_init(&now, time(0));
  return &now;
}

#if RUN_TESTS
static void test_thyme_()
{
  a_time t1;
  thyme_init(&t1, time(0));
}

static void test_thyme_ymdhms()
{
  a_time t;
  time_t now = time(0);
  thyme_init(&t, now);
#define X(year, mon, day, hour, min, sec) do {        \
    thyme_ymdhms(&t, year, mon, day, hour, min, sec); \
    const struct tm *tm = thyme_tm(&t);               \
    if (tm->tm_year != year - 1900 ||                 \
        tm->tm_mon != mon - 1 ||                      \
        tm->tm_mday != day ||                         \
        tm->tm_hour != hour ||                        \
        tm->tm_min != min ||                          \
        tm->tm_sec != sec)                            \
      TFAIL();                                        \
  } while(0)
  X(2015, 06, 01, 17, 26, 46);
  X(2015, 01, 01, 01, 01, 01);
  X(2015, 12, 31, 11, 59, 59);
#undef X
}

static void test_thyme_whms()
{
  a_time t_ = thyme_clone(thyme_now()), *t = &t_;
#define X(wday, hour, min, sec) do {            \
    thyme_whms(t, wday, hour, min, sec);        \
    time_t time = thyme_time(t);                \
    thyme_init(t, time);                        \
    const struct tm *tm = thyme_tm(t);          \
    if (tm->tm_wday != wday ||                  \
        tm->tm_hour != hour ||                  \
        tm->tm_min != min ||                    \
        tm->tm_sec != sec)                      \
      TFAIL();                                  \
  } while (0)
  X(0, 0, 0, 0);
  X(1, 1, 1, 1);
  X(6, 23, 59, 59);
#undef X
}

static void test_is_leap_year()
{
#define X(yn, nyear)                            \
  if (yn != is_leap_year(nyear)) TFAIL()
  X(0, 1900);
  X(0, 1901);
  X(0, 1902);
  X(0, 1903);
  X(1, 1904);
  X(1, 2000);
#undef X
}

void __attribute__((constructor)) test_thyme()
{
  test_thyme_();
  test_thyme_whms();
  test_thyme_ymdhms();
  test_is_leap_year();
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "main.c"
#include "util.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o thyme thyme.c && ./thyme"
 * End:
 */

#endif /* __thyme_c__ */

