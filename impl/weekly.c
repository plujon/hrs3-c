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
status week_parse_single(a_week *week, const char *s, size_t len)
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
  NOD(day_init(&day, s, len));
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
status week_init(a_week *week, const char *s, size_t len)
{
  const char *period = strnchr(s, len, '.');
  if (period) {
    size_t offset = period - s;
    NOD(week_parse_single(week, s, offset));
    s += offset + 1;
    len -= offset + 1;
    a_week rest;
    NOD(week_init(week ? &rest : 0, s, len));
    if (week) {
      week_merge(week, &rest);
      week_destroy(&rest);
    }
    return OK;
  }
  return week_parse_single(week, s, len);
}

void week_add_to_schedule(const a_week *week, const a_time *t, a_schedule *schedule)
{
  a_time date_ = time_clone(t), *date = &date_;
  size_t i = 0;
  for (; i < DIM(week->days); ++i) {
    const a_day *day = &week->days[i];
    if (0 == day->n_ranges) continue;
    time_whms(date, i, 0, 0, 0);
    day_add_to_schedule(day, date, schedule);
  }
}

#if RUN_TESTS
static void test_gobble_days()
{
#define X(S, LEN, MASK) do {                            \
    a_day_mask mask;                                    \
    if (LEN != gobble_days(S, LEN, &mask)) TFAIL();     \
    if ((MASK) != mask) TFAIL();                        \
  } while_0
  X("U1-2", 1, SUNDAY);
  X("UM1-2", 2, SUNDAY | MONDAY);
  X("UU1-2", 2, SUNDAY);
  X("AUMTWRF1-2", 7, SUNDAY | MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY | SATURDAY );
#undef X
}

static a_remaining_result hrs3_remaining_(const char *hrsss, time_t time);

static int thwr_aux(const char *hrsss,
                    int wday, int hour, int minute, int second,
                    int is_valid, int time_is_in_schedule, int seconds)
{
  a_remaining_result expected = { is_valid, time_is_in_schedule, seconds };
  a_time t = time_clone(time_now());
  time_whms(&t, wday, hour, minute, second);
  a_remaining_result result = hrs3_remaining_(hrsss, time_time(&t));
  return (expected.is_valid == result.is_valid &&
          expected.time_is_in_schedule == result.time_is_in_schedule &&
          expected.seconds == result.seconds) ? 0 : 1;
}

static void test_weekly_remaining()
{
#define IN(hrsss, wday, h, m, s, seconds)                             \
  if (thwr_aux(hrsss, wday, h, m, s, 1, 1, seconds)) TFAIL()
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
#define OUT(hrsss, wday, h, m, s, seconds)                            \
  if (thwr_aux(hrsss, wday, h, m, s, 1, 0, seconds)) TFAIL()
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
#define BAD(hrsss) do {                                                 \
    a_time t = time_clone(time_now());                                \
    a_remaining_result result = hrs3_remaining_(hrsss, time_time(&t)); \
    if (result.is_valid) TFAIL();                                       \
  } while_0
  BAD("U");
  BAD("U1");
  BAD("U-");
  BAD("U13-12");
  BAD("X1-2");
#undef BAD
}

PRE_INIT(test_weekly)
{
  test_gobble_days();
  test_weekly_remaining();
}

#include "daily.c"
#include "util.c"
#include "time_range.c"
#include "remaining.c"
#include "schedule.c"
#include "../hrs3.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o weekly weekly.c && ./weekly"
 * End:
 */

#endif /* __weekly_c__ */
