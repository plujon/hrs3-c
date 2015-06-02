#ifndef __weekly_c__
#define __weekly_c__

#include "impl.h"
#include <string.h>

#define SUNDAY (1 << 0)
#define MONDAY (1 << 1)
#define TUESDAY (1 << 2)
#define WEDNESDAY (1 << 3)
#define THURSDAY (1 << 4)
#define FRIDAY (1 << 5)
#define SATURDAY (1 << 6)

typedef struct day_descriptor {
  char c;
  unsigned int mask_bit;
} day_descriptor;

static day_descriptor day_descriptors[] = {
  { 'U', SUNDAY },
  { 'M', MONDAY },
  { 'T', TUESDAY },
  { 'W', WEDNESDAY },
  { 'R', THURSDAY },
  { 'F', FRIDAY },
  { 'A', SATURDAY }
};

typedef unsigned int a_day_mask;

static size_t gobble_days(const char *s, size_t len, a_day_mask *mask)
{
  size_t n_gobbled = 0;
  if (mask)
    *mask = 0;
  if (!s)
    return 0;
  const char *end = s + len;
  for (; s < end; ++s) {
    size_t i = 0;
    for (; i < DIM(day_descriptors); ++i) {
      if (s[0] == day_descriptors[i].c) {
        if (mask)
          *mask |= day_descriptors[i].mask_bit;
        break;
      }
    }
    if (i == DIM(day_descriptors)) {
      break;
    } else {
      ++n_gobbled;
    }
  }
  return n_gobbled;
}

void week_destroy(a_week *week)
{
  if (!week) return;
  size_t i = 0;
  for (; i < DIM(week->days); ++i) {
    day_destroy(&week->days[i]);
  }
}

void week_merge(a_week *dest, a_week *src)
{
  size_t i = 0;
  for (; i < DIM(src->days); ++i) {
    a_day *day = &src->days[i];
    if (day->ranges) {
      if (dest->days[i].ranges) {
        day_merge(&dest->days[i], day);
        day_destroy(day);
      } else {
        day_copy(&dest->days[i], day);
        day->ranges = 0;
      }
    }
  }
}

/* MWF10-12 */
status week_parse_single(const char *s, size_t len, a_week *week)
{
  if (week)
    memset(week, 0, sizeof(a_week));
  a_day_mask mask;
  size_t offset = gobble_days(s, len, &mask);
  if (0 == offset)
    return __LINE__;
  s += offset;
  len -= offset;
  a_day day;
  NOD(day_parse(s, len, &day));
  if (week) {
    size_t i = 0;
    for (; i < DIM(day_descriptors); ++i) {
      if (mask & day_descriptors[i].mask_bit) {
        day_clone(&week->days[i], &day);
      }
    }
  }
  day_destroy(&day);
  return OK;
}

/* MWF10-12.T8-9 */
size_t week_parse_(const char *buffer, a_week *week)
{
  size_t offset = 0;
  return offset;
}

status week_parse(const char *s, size_t len, a_week *week)
{
  const char *period = strnchr(s, len, '.');
  if (period) {
    size_t offset = period - s;
    NOD(week_parse_single(s, offset, week));
    s += offset + 1;
    len -= offset + 1;
    a_week rest;
    NOD(week_parse(s, len, week ? &rest : 0));
    if (week) {
      week_merge(week, &rest);
      week_destroy(&rest);
    }
    return OK;
  }
  return week_parse_single(s, len, week);
}

a_time_range week_find_lowerbound(a_week *week, a_time *target)
{
  a_time_range ret;
  a_time anchor = beginning_of_week(target);
  a_military_time midnight = military_midnight();
  int target_seconds_into_week = thyme_diff(target, &anchor);
  size_t i = 0;
  for (; i < DIM(week->days) + 1; ++i) {
    a_day *day = &week->days[i % 7];
    if (0 == day->n_ranges)
      continue;
    int day_seconds = 3600 * 24 * i;
    if (day_seconds + 3600 * 24 < target_seconds_into_week) {
      /* skip this day */
      continue;
    }
    int target_second = target_seconds_into_week - day_seconds;
    size_t j = 0;
    for (; j < day->n_ranges; ++j) {
      a_military_range *range = &day->ranges[j];
      int stop_second = military_time_diff(&range->stop, &midnight);
      if (target_second < stop_second) {
        /* bingo */
        int sec = military_range_in_seconds(range);
        thyme_incr(&anchor, day_seconds + stop_second - sec);
        time_range_init(&ret, &anchor, sec);
        return ret;
      }
    }
  }
  BUG();
  return time_range_empty();
}

/*
 * Return (a) whether t is within s, and (b) the number of seconds
 * after t that the answer to (a) is guaranteed.  See hrs3_remaining.
 *
 * "M8-12"
 * "MWF8-12&13-15"
 * "MWF8-12&13-15.TR5-6"
 */
a_remaining_result weekly_remaining(const char *s, a_time *t)
{
  if (!s || !s[0] || !t)
    return remaining_invalid();
  a_week week;
  if (OK != week_parse(s, strlen(s), &week))
    return remaining_invalid();
  a_time_range range = week_find_lowerbound(&week, t);
  bool is_in_range = time_range_contains(&range, t);
  int seconds = is_in_range ?
    thyme_diff(&range.stop, t) :
    thyme_diff(&range.start, t);
  a_remaining_result result = { 1, is_in_range, seconds };
  return result;
}

#if RUN_TESTS
static void test_gobble_days()
{
#define X(S, LEN, MASK) do {                            \
    a_day_mask mask;                                    \
    if (LEN != gobble_days(S, LEN, &mask)) TFAIL();     \
    if ((MASK) != mask) TFAIL();                        \
  } while (0)
  X("U1-2", 1, SUNDAY);
  X("UM1-2", 2, SUNDAY | MONDAY);
  X("UU1-2", 2, SUNDAY);
  X("AUMTWRF1-2", 7, SUNDAY | MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY | SATURDAY );
#undef X
}

static int thwr_aux(const char *hrs3,
                    int wday, int hour, int minute, int second,
                    int is_valid, int time_is_in_schedule, int seconds)
{
  a_remaining_result expected = { is_valid, time_is_in_schedule, seconds };
  a_time t = thyme_clone(thyme_now());
  thyme_whms(&t, wday, hour, minute, second);
  a_remaining_result result = weekly_remaining(hrs3, &t);
  return (expected.is_valid == result.is_valid &&
          expected.time_is_in_schedule == result.time_is_in_schedule &&
          expected.seconds == result.seconds) ? 0 : 1;
}

static void test_weekly_remaining()
{
#define IN(hrs3, wday, h, m, s, seconds)                        \
  if (thwr_aux(hrs3, wday, h, m, s, 1, 1, seconds)) TFAIL()
  IN("U8-9",                 0,  8,  0,  0,  3600);
  IN("U8-9",                 0,  8, 59, 59,     1);
  IN("UA6-7&8-9",            0,  6,  0,  0,  3600);
  IN("UA6-7&8-9",            0,  8, 59, 59,     1);
  IN("UA6-7&8-9",            6,  6, 59, 59,     1);
  IN("UA6-7&8-9",            6,  8, 59, 59,     1);
  IN("U1-2&3-4.M6-7&8-9",    0,  1, 30,  0,  1800);
  IN("U1-2&3-4.M6-7&8-9",    0,  3,  0,  0,  3600);
  IN("U1-2&3-4.M6-7&8-9",    1,  6,  0,  0,  3600);
  IN("U1-2&3-4.M6-7&8-9",    1,  8,  0,  0,  3600);
#undef IN
#define OUT(hrs3, wday, h, m, s, seconds)                       \
  if (thwr_aux(hrs3, wday, h, m, s, 1, 0, seconds)) TFAIL()
  OUT("U8-9",                 0,  7, 59, 59,     1);
  OUT("U8-9",                 0,  9,  0,  0, 6 * 24 * 3600 + 23 * 3600);
  OUT("U8-9",                 1,  8,  0,  0, 5 * 24 * 3600 + 24 * 3600);
  OUT("U8-9",                 1,  9,  0,  0, 5 * 24 * 3600 + 23 * 3600);
  OUT("U1-2&3-4.M6-7&8-9",    0,  0, 59, 59,     1);
  OUT("U1-2&3-4.M6-7&8-9",    0,  2,  0,  0,  3600);
  OUT("U1-2&3-4.M6-7&8-9",    0,  2, 59, 59,     1);
  OUT("U1-2&3-4.M6-7&8-9",    0,  4,  0,  0, 3600 * 24 + 2 * 3600);
  OUT("U1-2&3-4.M6-7&8-9",    1,  5, 59, 59,     1);
  OUT("U1-2&3-4.M6-7&8-9",    1,  7,  0,  0,  3600);
  OUT("U1-2&3-4.M6-7&8-9",    1,  7, 59, 59,     1);
  OUT("U1-2&3-4.M6-7&8-9",    1,  9,  0,  0, 3600 * 24 * 6 - 3600 * 8);
#undef OUT
#define BAD(hrs3) do {                                                  \
    a_time t = thyme_clone(thyme_now());                                \
    a_remaining_result result = weekly_remaining(hrs3, &t);             \
    if (result.is_valid) TFAIL();                                       \
  } while(0)
  BAD("U");
  BAD("U1");
  BAD("U-");
  BAD("U13-12");
  BAD("X1-2");
#undef BAD
}

void __attribute__((constructor)) test_weekly()
{
  test_gobble_days();
  test_weekly_remaining();
}

#include "daily.c"
#include "util.c"
#include "time_range.c"
#include "remaining.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o weekly weekly.c && ./weekly"
 * End:
 */

#endif /* __weekly_c__ */
