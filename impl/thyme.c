#ifndef __thyme_c__
#define __thyme_c__

/*
 * Recommended reading:
 *
 * https://en.wikipedia.org/wiki/Unix_time
 * https://en.wikipedia.org/wiki/Leap_second
 * https://en.wikipedia.org/wiki/Daylight_saving_time
 *
 * thyme is primarily unix time, with cached translation into human time.
 */

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

bool thyme_precedes(const a_time *earlier, const a_time *later)
{
  return 0 < thyme_diff(later, earlier);
}

a_time thyme_clone(const a_time *src)
{
  return *src;
}

void thyme_copy(a_time *dest, const a_time *src)
{
  if (dest != src)
    memcpy(dest, src, sizeof(a_time));
}

void thyme_incr(a_time *t, int sec)
{
  if (0 != sec) {
    time_t stamp = thyme_time(t);
    thyme_init(t, stamp + sec);
  }
}

void thyme_next_day(a_time *t)
{
  /*
   * Most days are 3600 * 24 seconds, but some have +/- 1 leap second
   * and others have +/- 3600 because of DST.
   */
#if CHECK
  int tm_wday = thyme_tm(t)->tm_wday;
#endif
  if (!thyme_hms(t, 23, 59, 59) && !thyme_hms(t, 23, 59, 58))
    BUG();
  while (thyme_tm(t)->tm_sec != 0)
    thyme_incr(t, 1);
#if CHECK
  if (0 != thyme_tm(t)->tm_hour) BUG();
  if (0 != thyme_tm(t)->tm_min) BUG();
  if ((tm_wday + 1 % 7) != thyme_tm(t)->tm_wday) BUG();
#endif
}

void thyme_next_week(a_time *t)
{
  thyme_whms(t, 6, 23, 59, 59);
  thyme_incr(t, 1);
#if CHECK
  if (0 != thyme_tm(t)->tm_wday) BUG();
  if (0 != thyme_tm(t)->tm_hour) BUG();
  if (0 != thyme_tm(t)->tm_min) BUG();
  if (0 != thyme_tm(t)->tm_sec) BUG();
#endif
}

a_time thyme_plus(const a_time *t, int sec)
{
  a_time ret = thyme_clone(t);
  thyme_incr(&ret, sec);
  return ret;
}

time_t thyme_time(const a_time *t)
{
#if CHECK
  if (!t->time) BUG();
#endif
  return t->time;
}

const struct tm *thyme_tm(const a_time *t)
{
  if (!t->tm.tm_year) {
#if CHECK
    if (!t->time) BUG();
#endif
    localtime_r(&t->time, (struct tm *)&t->tm);
  }
  return &t->tm;
}

a_time *thyme_minimum(const a_time *a, const a_time *b)
{
  time_t time_a = thyme_time(a);
  time_t time_b = thyme_time(b);
  return (a_time *)(time_a < time_b ? a : b);
}

a_time *thyme_maximum(const a_time *a, const a_time *b)
{
  time_t time_a = thyme_time(a);
  time_t time_b = thyme_time(b);
  return (a_time *)(time_a < time_b ? b : a);
}

void thyme_init(a_time *t, time_t time)
{
  t->time = time;
  memset(&t->tm, 0, sizeof(struct tm));
}

/*
 * Some hour, min, sec combinations are not valid.  E.g., in PST in
 * Spring, the clock jumps from 2am to 3am on March 8, 2015.  I.e.,
 *
 *   false == thyme_hms(march_8_2015, 2, 0, 0);
 *
 * Likewise, some hour, min, sec combinations can lead to two
 * different results for t, both of which are valid.  E.g., in PST in
 * Fall, the clock jumps from 2am back to 1am on November 11, 2015.
 * I.e.,
 *
 *   true == thyme_hms(november_11_2015, 1, 0, 0);
 *   1446368400 == november_11_2015 || 1446372000 == november_11_2015;
 *   1446368400 + 3600 == 1446372000;
 */
bool thyme_hms(a_time *t, int hour, int min, int sec)
{
  return thyme_ymdhms(t,
                      thyme_tm(t)->tm_year + 1900,
                      thyme_tm(t)->tm_mon + 1,
                      thyme_tm(t)->tm_mday,
                      hour, min, sec);
}

static bool thyme_wday(a_time *t, int wday)
{ 
  /*
   * Find the day in the same week as t that has wday.  If the day is
   * earlier in the week, set t to the last second of the target day.
   * If the day is later in the week, set to to the first second of
   * the target day.  This preserves the "nearest match" behavior
   * described in thyme_ymdhms.  Avoid overshooting into another week
   * by incrementing conservatively.  This is not a simple 24 because
   * of DST and leap seconds.
   */
  int tm_wday = thyme_tm(t)->tm_wday;
  if (tm_wday == wday)
    return true;
  thyme_incr(t, 3600 * (24 - 1) * (wday - tm_wday));
  if (thyme_tm(t)->tm_wday == wday)
    return tm_wday < wday ? thyme_hms(t, 0, 0, 0) : thyme_hms(t, 23, 59, 59);
  return thyme_wday(t, wday);
}

/*
 * thyme_whms takes care to prefer the nearest future time when matching time.
 */
bool thyme_whms(a_time *t, int wday, int hour, int min, int sec)
{
  int tm_wday = thyme_tm(t)->tm_wday;
  if (tm_wday == wday)
    return thyme_hms(t, hour, min, sec);
  a_time target_day = thyme_clone(t);
  if (!thyme_wday(&target_day, wday))
    return false;
  if (tm_wday < wday) {
    /* go forward to start of correct day */
    if (!thyme_hms(&target_day, 0, 0, 0))
      return false;
  } else {
    /* go back to end of correct day */
    if (!thyme_hms(&target_day, 23, 59, 59))
      return false;
  }
  if (!thyme_hms(&target_day, hour, min, sec))
    return false;
  thyme_copy(t, &target_day);
  return true;
}

static bool thyme_is_ymdhms(a_time *t, int year, int mon, int mday,
                            int hour, int min, int sec)
{
  return (sec == thyme_tm(t)->tm_sec &&
          hour == thyme_tm(t)->tm_hour &&
          min == thyme_tm(t)->tm_min &&
          mday == thyme_tm(t)->tm_mday &&
          mon - 1 == thyme_tm(t)->tm_mon &&
          year - 1900 == thyme_tm(t)->tm_year) ? true : false;
}

bool thyme_ymdhms(a_time *t,
                  int year, int mon, int mday,
                  int hour, int min, int sec)
{
  /*
   * Some times occur twice, such as 2am in fall in PST.  Prefer the
   * time nearest to t that is at least as large as t.
   *
   * Apologies to Australia's Lord Howe Island, which uses a half-hour
   * shift, and which is not handled in this code.
   */ 
  if (thyme_is_ymdhms(t, year, mon, mday, hour, min, sec))
    return true;
   struct tm target_tm = *thyme_tm(t);
   target_tm.tm_year = year - 1900;
   target_tm.tm_mon = mon - 1;
   target_tm.tm_mday = mday;
   target_tm.tm_hour = hour;
   target_tm.tm_min = min;
   target_tm.tm_sec = sec;
   target_tm.tm_sec = sec;
   target_tm.tm_isdst = -1;
   a_time target;
   time_t tt = mktime(&target_tm);
   if (-1 == tt)
     return false;
   thyme_init(&target, tt);
   bool target_is_future = 0 < thyme_diff(&target, t) ? true : false;
   int dst_check_offset = target_is_future ? -3600 : 3600;
   a_time dst_check = thyme_clone(&target);
   thyme_incr(&dst_check, dst_check_offset);
   if (thyme_is_ymdhms(&dst_check, year, mon, mday, hour, min, sec) &&
       0 <= thyme_diff(&dst_check, t)) {
     thyme_copy(&target, &dst_check);
   }
   thyme_copy(t, &target);
   return true;
 }

int is_leap_year(int nyear)
{
  if (0 != nyear % 4) return 0;
  if (0 != nyear % 100) return 1;
  if (0 != nyear % 400) return 0;
  return 1;
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

size_t thyme_string_length()
{
  return sizeof("CCYYMMDDHHMMSS") - 1; /* 14 */
}

status thyme_parse(const char *s, size_t len, a_time *out)
{
  memset(out, 0, sizeof(a_time));
  if (len < thyme_string_length()) {
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
  int second = s_to_d(s, 2); s += 2;
  thyme_copy(out, thyme_now());
  if (!thyme_ymdhms(out, year, mon, day, hour, minute, second))
    return __LINE__;
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
  dd_to_s(&buffer[12], tm->tm_sec);
  return 14;
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
#define X(year, mon, day, hour, min, sec) do {                        \
    thyme_ymdhms(&t, year, mon, day, hour, min, sec);                 \
    const struct tm *tm = thyme_tm(&t);                               \
    if (tm->tm_year != year - 1900 ||                                 \
        tm->tm_mon != mon - 1 ||                                      \
        tm->tm_mday != day ||                                         \
        tm->tm_hour != hour ||                                        \
        tm->tm_min != min ||                                          \
        tm->tm_sec != sec)                                            \
      TFAIL();                                                        \
  } while(0)
  X(2015, 06, 01, 17, 26, 46);
  X(2015, 01, 01, 01, 01, 01);
  X(2015, 12, 31, 11, 59, 59);
#undef X
}

static void test_thyme_whms()
{
  a_time t_ = thyme_clone(thyme_now()), *t = &t_;
#define X(wday, hour, min, sec) do {                                  \
    thyme_whms(t, wday, hour, min, sec);                              \
    time_t time = thyme_time(t);                                      \
    a_time t2;                                                         \
    thyme_init(&t2, time);                                             \
    const struct tm *tm = thyme_tm(&t2);                                \
    if (tm->tm_wday != wday) TFAILF(" %s", "wday mismatch");          \
    if (tm->tm_sec != sec) TFAILF(" sec %d vs %d", tm->tm_sec, sec); \
    if (tm->tm_min != min) TFAILF(" min %d vs %d", tm->tm_min, min); \
    if (tm->tm_hour != hour) TFAILF(" hour %d vs %d", tm->tm_hour, hour); \
  } while (0)
  X(0, 0, 0, 0);
  X(1, 1, 1, 1);
  X(6, 23, 59, 59);
#undef X
}

static void test_thyme_parse()
{
  a_time t_, *t = &t_;
#define X(S) do {                                                     \
    if (thyme_parse(S, sizeof(S) - 1, t)) TFAIL();                    \
  } while (0)
  X("20150611163613");
  X("20150111163613");
  X("20151211163613");
  X("20150308015959");
#undef X
}

static void test_is_leap_year()
{
#define X(yn, nyear)                                                  \
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
  test_thyme_parse();
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

