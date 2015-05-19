#ifndef __daily_c__
#define __daily_c__

#include "hrs3.h"
#include "military.h"
#include "remaining.h"
#include <string.h>

/*
 * Return (a) whether t is within s, and (b) the number of seconds
 * after t that the answer to (a) is guaranteed.  See hrs3_remaining.
 *
 * "8-12"
 * "0800-1200"
 * "0800-1200&1300-1600&1700-1730"
 */

a_remaining_result hrs3_daily_remaining(const char *s, time_t t)
{
  if (!s || !*s)
    return remaining_invalid();
  struct tm ymdhms;
  localtime_r(&t, &ymdhms);
  a_military_time mt = { ymdhms.tm_hour, ymdhms.tm_min };
  int tm_sec = ymdhms.tm_sec;
  const char *p = s;
  a_military_time first_shift = { 24, 0 };
  while (p[0]) {
    a_military_range range;
    const char *amp = strchr(p, '&');
    if (amp) {
      if (military_parse_range(p, amp - p, &range))
        return remaining_invalid();
      p = amp + 1;
    } else {
      size_t len = strlen(p);
      if (military_parse_range(p, len, &range))
        return remaining_invalid();
      p += len;
    }
    if (first_shift.hour == 24) {
      first_shift = range.start;
    }
    switch (military_time_cmp(&mt, &range.start)) {
    case -1: {
      /* t is before the start of the time range */
      int seconds = military_time_diff(&range.start, &mt) - tm_sec;
      return remaining_result(0, seconds);
    }
    case 0: {
      /* Because t is the beginning of the time range, return the
         number of seconds in the time range. */
      int seconds = military_range_in_seconds(&range) - tm_sec;
      return remaining_result(1, seconds);
    }
    case 1: /* t follows the beginning of the time range */
      switch (military_time_cmp(&mt, &range.stop)) {
      case -1: {
        /* t is in the time range! */
        int seconds = military_time_diff(&range.stop, &mt);
        return remaining_result(1, seconds - tm_sec);
      }
      case 0: /* t follows the entire time range */
        break;
      case 1: /* t follows the entire time range */
        break;
      default:
        return remaining_invalid();
      }
      break;
    default:
      return remaining_invalid();
    }
  }
  /* t follows all time ranges in s */
  a_military_time end_of_day = { 24, 0 };
  a_military_time start_of_day = { 0, 0 };
  int seconds = military_time_diff(&end_of_day, &mt);
  seconds += military_time_diff(&first_shift, &start_of_day);
  seconds -= tm_sec;
  return remaining_result(0, seconds);
}

#if TEST
#include "test.h"
int test_hrs3_daily()
{
  struct tm ymdhms;
  time_t t = time(0);
  localtime_r(&t, &ymdhms);
#define X(x, h, m, s, is_in_schedule, secs)                  \
  do {                                                       \
    ymdhms.tm_hour = h;                                      \
    ymdhms.tm_min = m;                                       \
    ymdhms.tm_sec = s;                                       \
    t = mktime(&ymdhms);                                     \
    a_remaining_result result = hrs3_daily_remaining(x, t);  \
    if (!result.is_valid)                                    \
      TFAIL();                                               \
    if (is_in_schedule != result.time_is_in_schedule)        \
      TFAIL();                                               \
    if (secs != result.seconds)                              \
      TFAIL();                                               \
  } while(0)
  X("830-12",  7,  0,  0, 0, 60 * 90);
  X("830-12",  7,  0,  1, 0, 60 * 90 - 1);
  X("830-12",  7,  1,  0, 0, 60 * 89);
  X("830-12",  8, 29, 59, 0, 1);
  X("830-12",  8, 30, 00, 1, 60 * 210);
  X("830-12",  8, 30,  1, 1, 60 * 210 - 1);
  X("830-12",  8, 31,  1, 1, 60 * 209 - 1);
  X("830-12", 11, 59, 59, 1, 1);
  X("830-12", 12,  0,  0, 0, 3600 * 24 - 60 * 210);
  X("830-12", 12,  0,  1, 0, 3600 * 24 - 60 * 210 - 1);
  X("830-12", 12,  1,  1, 0, 3600 * 24 - 60 * 210 - 61);
  X("830-12", 13,  0,  0, 0, 3600 * 23 - 60 * 210);
  X("830-12&13-14",  7,  0,  0, 0, 60 * 90);
  X("830-12&13-14",  7,  0,  1, 0, 60 * 90 - 1);
  X("830-12&13-14",  7,  1,  0, 0, 60 * 89);
  X("830-12&13-14",  8, 29, 59, 0, 1);
  X("830-12&13-14",  8, 30, 00, 1, 60 * 210);
  X("830-12&13-14",  8, 30,  1, 1, 60 * 210 - 1);
  X("830-12&13-14",  8, 31,  1, 1, 60 * 209 - 1);
  X("830-12&13-14", 11, 59, 59, 1, 1);
  X("830-12&13-14", 12,  0,  0, 0, 3600);
  X("830-12&13-14", 12,  0,  1, 0, 3600 - 1);
  X("830-12&13-14", 12,  1,  1, 0, 3600 - 61);
  X("830-12&13-14", 12, 59, 59, 0, 1);
  X("830-12&13-14", 13,  0,  0, 1, 3600);
  X("830-12&13-15", 15,  0,  0, 0, 63000);
  X("830-12&13-15", 15,  0,  1, 0, 63000 - 1);
#undef X
  return 0;
}

int main()
{
  int military_main();
  return test_hrs3_daily() + military_main();
}

#undef main
#define main military_main
#endif

#include "military.c"
#include "remaining.c"

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o daily daily.c && ./daily"
 * End:
 */

#endif /* __daily_c__ */
