#ifndef __time_c__
#define __time_c__

/*
 * Recommended reading:
 *
 * https://en.wikipedia.org/wiki/Unix_time
 * https://en.wikipedia.org/wiki/Leap_second
 * https://en.wikipedia.org/wiki/Daylight_saving_time
 *
 * time is primarily unix time, with cached translation into human time.
 */

#include "impl.h"
#include <string.h>

int time_cmp(const a_time *a, const a_time *b)
{
  int x = time_diff(a, b);
  if (x < 0) /* b is bigger than a */ return -1;
  if (0 < x) /* a is bigger than b */ return 1;
  return 0;
}

int time_diff(const a_time *later, const a_time *earlier)
{
  return (int)(time_time(later) - time_time(earlier));
}

bool time_precedes(const a_time *earlier, const a_time *later)
{
  return 0 < time_diff(later, earlier);
}

a_time time_clone(const a_time *src)
{
  return *src;
}

void time_copy(a_time *dest, const a_time *src)
{
  if (dest != src)
    memcpy(dest, src, sizeof(a_time));
}

void time_incr(a_time *t, int sec)
{
  if (0 != sec) {
    time_t stamp = time_time(t);
    time_init(t, stamp + sec);
  }
}

void time_incr_days(a_time *t, int days)
{
  /*
   * Adjust t by N days, taking into account DST.
   */
  if (0 == days)
    return;
  a_time old = time_clone(t);
  int old_hour = time_tm(&old)->tm_hour;
  time_incr(t, days * 3600 * 24);
  int new_hour = time_tm(t)->tm_hour;
  if (new_hour <= 1 && 22 <= old_hour)
    time_incr(t, -3600 * 2); /* DST oops, go back a bit */
  else if (22 <= new_hour && old_hour <= 2)
    time_incr(t,  3600 * 2); /* DST oops, go forward a bit */
  time_hms(t,
           time_tm(&old)->tm_hour,
           time_tm(&old)->tm_min,
           time_tm(&old)->tm_sec);
}

void time_next_day(a_time *t)
{
  /*
   * Most days are 3600 * 24 seconds, but some have +/- 1 leap second
   * and others have +/- 3600 because of DST.
   */
#if CHECK
  int tm_wday = time_tm(t)->tm_wday;
#endif
  if (!time_hms(t, 23, 59, 59) && !time_hms(t, 23, 59, 58))
    BUG();
  while (time_tm(t)->tm_sec != 0)
    time_incr(t, 1);
#if CHECK
  if (0 != time_tm(t)->tm_hour) BUG();
  if (0 != time_tm(t)->tm_min) BUG();
  if ((tm_wday + 1) % 7 != time_tm(t)->tm_wday) BUG();
#endif
}

void time_next_week(a_time *t)
{
  time_whms(t, 6, 23, 59, 59);
  time_incr(t, 1);
#if CHECK
  if (0 != time_tm(t)->tm_wday) BUG();
  if (0 != time_tm(t)->tm_hour) BUG();
  if (0 != time_tm(t)->tm_min) BUG();
  if (0 != time_tm(t)->tm_sec) BUG();
#endif
}

a_time time_plus(const a_time *t, int sec)
{
  a_time ret = time_clone(t);
  time_incr(&ret, sec);
  return ret;
}

time_t time_time(const a_time *t)
{
#if CHECK
  if (!t->time) BUG();
#endif
  return t->time;
}

const struct tm *time_tm(const a_time *t)
{
  if (!t->tm.tm_year) {
#if CHECK
    if (!t->time) BUG();
#endif
    LOCALTIME_R(&t->time, (struct tm *)&t->tm);
  }
  return &t->tm;
}

a_time *time_minimum(const a_time *a, const a_time *b)
{
  time_t time_a = time_time(a);
  time_t time_b = time_time(b);
  return (a_time *)(time_a < time_b ? a : b);
}

a_time *time_maximum(const a_time *a, const a_time *b)
{
  time_t time_a = time_time(a);
  time_t time_b = time_time(b);
  return (a_time *)(time_a < time_b ? b : a);
}

void time_init(a_time *t, time_t time)
{
  t->time = time;
  memset(&t->tm, 0, sizeof(struct tm));
}

/*
 * Some hour, min, sec combinations are not valid.  E.g., in PST in
 * Spring, the clock jumps from 2am to 3am on March 8, 2015.  I.e.,
 *
 *   false == time_hms(march_8_2015, 2, 0, 0);
 *
 * Likewise, some hour, min, sec combinations can lead to two
 * different results for t, both of which are valid.  E.g., in PST in
 * Fall, the clock jumps from 2am back to 1am on November 11, 2015.
 * I.e.,
 *
 *   true == time_hms(november_11_2015, 1, 0, 0);
 *   1446368400 == november_11_2015 || 1446372000 == november_11_2015;
 *   1446368400 + 3600 == 1446372000;
 */
bool time_hms(a_time *t, int hour, int min, int sec)
{
  return time_ymdhms(t,
                      time_tm(t)->tm_year + 1900,
                      time_tm(t)->tm_mon + 1,
                      time_tm(t)->tm_mday,
                      hour, min, sec);
}

static bool time_wday(a_time *t, int wday)
{ 
  /*
   * Find the day in the same week as t that has wday.  If the day is
   * earlier in the week, set t to the last second of the target day.
   * If the day is later in the week, set to to the first second of
   * the target day.  This preserves the "nearest match" behavior
   * described in time_ymdhms.  Avoid overshooting into another week
   * by incrementing conservatively.  This is not a simple 24 because
   * of DST and leap seconds.
   */
  int tm_wday = time_tm(t)->tm_wday;
  if (tm_wday == wday)
    return true;
  time_incr(t, 3600 * (24 - 1) * (wday - tm_wday));
  if (time_tm(t)->tm_wday == wday)
    return tm_wday < wday ? time_hms(t, 0, 0, 0) : time_hms(t, 23, 59, 59);
  return time_wday(t, wday);
}

/*
 * time_whms takes care to prefer the nearest future time when matching time.
 */
bool time_whms(a_time *t, int wday, int hour, int min, int sec)
{
  int tm_wday = time_tm(t)->tm_wday;
  if (tm_wday == wday)
    return time_hms(t, hour, min, sec);
  a_time target_day = time_clone(t);
  if (!time_wday(&target_day, wday))
    return false;
  if (tm_wday < wday) {
    /* go forward to start of correct day */
    if (!time_hms(&target_day, 0, 0, 0))
      return false;
  } else {
    /* go back to end of correct day */
    if (!time_hms(&target_day, 23, 59, 59))
      return false;
  }
  if (!time_hms(&target_day, hour, min, sec))
    return false;
  time_copy(t, &target_day);
  return true;
}

static bool time_is_ymdhms(a_time *t, int year, int mon, int mday,
                            int hour, int min, int sec)
{
  return (sec == time_tm(t)->tm_sec &&
          hour == time_tm(t)->tm_hour &&
          min == time_tm(t)->tm_min &&
          mday == time_tm(t)->tm_mday &&
          mon - 1 == time_tm(t)->tm_mon &&
          year - 1900 == time_tm(t)->tm_year) ? true : false;
}

bool time_ymdhms(a_time *t,
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
  if (time_is_ymdhms(t, year, mon, mday, hour, min, sec))
    return true;
   struct tm target_tm = *time_tm(t);
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
   time_init(&target, tt);
   bool target_is_future = 0 < time_diff(&target, t) ? true : false;
   int dst_check_offset = target_is_future ? -3600 : 3600;
   a_time dst_check = time_clone(&target);
   time_incr(&dst_check, dst_check_offset);
   if (time_is_ymdhms(&dst_check, year, mon, mday, hour, min, sec) &&
       0 <= time_diff(&dst_check, t)) {
     time_copy(&target, &dst_check);
   }
   time_copy(t, &target);
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

size_t time_string_length(void)
{
  return sizeof("CCYYMMDDHHMMSS") - 1; /* 14 */
}

status time_parse(a_time *out, const char *s, size_t len)
{
  memset(out, 0, sizeof(a_time));
  if (len < time_string_length()) {
    return NO;
  }
  int year = s_to_d(s, 4, 0); s += 4;
  int mon = s_to_d(s, 2, 0); s += 2;
  if (mon <= 0 || 13 <= mon) return NO;
  int day = s_to_d(s, 2, 0); s += 2;
  if (day < 0 ||  days_in_mon(mon, year) < day) return NO;
  int hour = s_to_d(s, 2, 0); s += 2;
  if (hour < 0 || 24 <= hour) return NO;
  int minute = s_to_d(s, 2, 0); s += 2;
  if (minute < 0 || 60 <= minute) return NO;
  int second = s_to_d(s, 2, 0); s += 2;
  time_copy(out, time_now());
  if (!time_ymdhms(out, year, mon, day, hour, minute, second))
    return NO;
  return OK;
}

/* TODO - Maybe put seconds in buffer too. */
size_t time_to_s(const a_time *t, char *buffer)
{
  const struct tm *tm = time_tm(t);
  dd_to_s(buffer, ((tm->tm_year + 1900) / 100));
  dd_to_s(&buffer[2], ((tm->tm_year + 1900) % 100));
  dd_to_s(&buffer[4], tm->tm_mon + 1);
  dd_to_s(&buffer[6], tm->tm_mday);
  dd_to_s(&buffer[8], tm->tm_hour);
  dd_to_s(&buffer[10], tm->tm_min);
  dd_to_s(&buffer[12], tm->tm_sec);
  return 14;
}

const a_time *time_now()
{
  static a_time now;
  if (!now.time)
    time_init(&now, time(0));
  return &now;
}

#if _WIN32
PRE_INIT(cygwin_tz_fix)
{
  // http://cygwin.1069669.n5.nabble.com/Re-PATCH-Setting-TZ-may-break-time-in-non-Cygwin-programs-tt90762.html
  const char *tz = getenv("TZ");
  if (!tz || strlen(tz) < 5)
    return;
  if ('0' <= tz[3] && tz[3] <= '9')
    return;
  if ('0' <= tz[4] && tz[4] <= '9' && ('-' == tz[3] || '+' == tz[3]))
    return;
  _putenv("TZ=");
}
#endif

#if RUN_TESTS
static void test_time_(void)
{
  a_time t1;
  time_init(&t1, time(0));
}

static void test_time_ymdhms(void)
{
  a_time t;
  time_t now = time(0);
  time_init(&t, now);
#define X(year, mon, day, hour, min, sec) do {                        \
    time_ymdhms(&t, year, mon, day, hour, min, sec);                 \
    const struct tm *tm = time_tm(&t);                               \
    if (tm->tm_year != year - 1900 ||                                 \
        tm->tm_mon != mon - 1 ||                                      \
        tm->tm_mday != day ||                                         \
        tm->tm_hour != hour ||                                        \
        tm->tm_min != min ||                                          \
        tm->tm_sec != sec)                                            \
      TFAIL();                                                        \
  } while_0
  X(2015, 06, 01, 17, 26, 46);
  X(2015, 01, 01, 01, 01, 01);
  X(2015, 12, 31, 11, 59, 59);
#undef X
}

static void test_time_whms(void)
{
  a_time t_ = time_clone(time_now()), *t = &t_;
#define X(wday, hour, min, sec) do {                                  \
    time_whms(t, wday, hour, min, sec);                              \
    time_t time = time_time(t);                                      \
    a_time t2;                                                         \
    time_init(&t2, time);                                             \
    const struct tm *tm = time_tm(&t2);                                \
    if (tm->tm_wday != wday) TFAILF(" %s", "wday mismatch");          \
    if (tm->tm_sec != sec) TFAILF(" sec %d vs %d", tm->tm_sec, sec); \
    if (tm->tm_min != min) TFAILF(" min %d vs %d", tm->tm_min, min); \
    if (tm->tm_hour != hour) TFAILF(" hour %d vs %d", tm->tm_hour, hour); \
  } while_0
  X(0, 0, 0, 0);
  X(1, 1, 1, 1);
  X(6, 23, 59, 59);
#undef X
}

static void test_time_parse(void)
{
  a_time t_, *t = &t_;
#define X(S) do {                                                     \
    if (time_parse(t, S, sizeof(S) - 1)) TFAIL();                     \
  } while_0
  X("20150611163613");
  X("20150111163613");
  X("20151211163613");
  X("20150308015959");
#undef X
}

static void test_is_leap_year(void)
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

PRE_INIT(test_time)
{
  test_time_();
  test_time_whms();
  test_time_ymdhms();
  test_time_parse();
  test_is_leap_year();
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "main.c"
#include "util.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o time time.c && ./time"
 * End:
 */

#endif /* __time_c__ */

