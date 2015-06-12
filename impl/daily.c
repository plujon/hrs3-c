#ifndef __daily_c__
#define __daily_c__

#include "impl.h"
#include <string.h>
#include <stdlib.h>

void day_clone(a_day *dest, a_day *src)
{
  if (dest->ranges) {
    day_destroy(dest);
  }
  if (src->ranges) {
    dest->capacity = src->n_ranges;
    dest->n_ranges = src->n_ranges;
    dest->ranges = malloc(sizeof(*dest->ranges) * dest->capacity);
    memcpy(dest->ranges, src->ranges, sizeof(src->ranges[0]) * src->n_ranges);
  }
}

void day_copy(a_day *dest, a_day *src)
{
  if (dest->ranges) {
    day_destroy(dest);
  }
  if (src->ranges) {
    dest->capacity = src->capacity;
    dest->n_ranges = src->n_ranges;
    dest->ranges = src->ranges;
  }
}

void day_destroy(a_day *day)
{
  if (day->ranges)
    free(day->ranges);
  day->capacity = 0;
  day->n_ranges = 0;
  day->ranges = 0;
}

/* Merge any overlapping ranges.  Recurse if anything changed. */
void day_coallesce(a_day *day)
{
  a_military_range *prev = 0;
  size_t i = 0;
  for (; i < day->n_ranges; ++i) {
    a_military_range *range = &day->ranges[i];
    if (prev) {
      if (military_range_overlaps_or_abuts(prev, range)) {
        /* merge, remove, recurse, and return */
        military_range_merge(prev, range);
        memmove(range, range + 1, day->n_ranges - i);
        day->n_ranges -= 1;
        day_coallesce(day);
        return;
      }
    }
    prev = range;
  }
}

void day_grow(a_day *day)
{
  if (!day->capacity)
    day->capacity = 4;
  day->capacity *= 2;
  if (day->ranges)
    day->ranges = realloc(day->ranges, sizeof(day->ranges[0]) * day->capacity);
  else
    day->ranges = malloc(sizeof(day->ranges[0]) * day->capacity);
}

void day_insert_at(a_day *day, size_t index, a_military_range *insertee)
{
  while (day->capacity <= day->n_ranges)
    day_grow(day);
  size_t len = index - day->n_ranges;
  if (0 < len) {
    memmove(&day->ranges[index],
            &day->ranges[index + 1],
            len);
  }
  memcpy(&day->ranges[index], insertee, sizeof(a_military_range));
  day->n_ranges += 1;
}

void day_insert(a_day *dest, a_military_range *range)
{
  size_t i = 0;
  for (; i < dest->n_ranges; ++i) {
    a_military_range *x = &dest->ranges[i];
    if (military_range_overlaps_or_abuts(range, x)) {
      /* range and x overlap or abut, so merge them together */
      military_range_merge(x, range);
    } else if (military_time_cmp(&range->start, &x->start) < 0) {
      /* range precedes x so insert range before x */
      day_insert_at(dest, i, range);
      break;
    }
  }
  if (i == dest->n_ranges) {
    /* insert range at end */
    day_insert_at(dest, i, range);
  }
  day_coallesce(dest);
}

void day_merge(a_day *dest, a_day *src)
{
  while (dest->capacity < dest->n_ranges + src->n_ranges) {
    day_grow(dest);
  }
  size_t i = 0;
  for (; i < src->n_ranges; ++i) {
    a_military_range *range = &src->ranges[i];
    day_insert(dest, range);
  }
}

/* 10-12 */
status day_parse_single(const char *s, size_t len, a_day *day)
{
  if (day)
    memset(day, 0, sizeof(a_day));
  a_military_range range;
  NOD(military_parse_range(s, len, &range));
  if (day)
    day_insert(day, &range);
  return OK;
}

/* 10-12&13-15 */
status day_parse(const char *s, size_t len, a_day *day)
{
  if (!s || !*s)
    return __LINE__;
  const char *amp = strnchr(s, len, '&');
  if (amp) {
    size_t offset = amp - s;
    NOD(day_parse_single(s, offset, day));
    s += offset + 1;
    len -= offset + 1;
    a_day rest;
    NOD(day_parse(s, len, day ? &rest : 0));
    if (day) {
      day_merge(day, &rest);
      day_destroy(&rest);
    }
    return OK;
  }
  return day_parse_single(s, len, day);
}

size_t day_to_s(char *buffer, a_day *day)
{
  size_t offset = military_range_to_s(&buffer[0], &day->ranges[0]);
  size_t i = 1;
  for (; i < day->n_ranges; ++i) {
    buffer[offset++] = '&';
    offset += military_range_to_s(&buffer[offset], &day->ranges[i]);
  }
  return offset;
}

char *day_to_s_dup(a_day *day)
{
  char buffer[0x200];
  size_t offset = day_to_s(buffer, day);
  buffer[offset] = 0;
  return strdup(buffer);
}

void day_add_to_schedule(const a_day *day, const a_time *t, a_schedule *schedule)
{
  size_t i = 0;
  for (; i < day->n_ranges; ++i) {
    a_military_range *range = &day->ranges[i];
    a_time_range time_range_, *time_range = &time_range_;
    if (military_range_to_time_range(range, t, time_range)) {
      schedule_insert(schedule, time_range);
    }
  }
}

#if RUN_TESTS

static a_remaining_result hrs3_remaining_(const char *hrsss, time_t time);

void test_day_parse()
{
#define X(x, h, m, s, is_in_schedule, secs)                             \
  do {                                                                  \
    a_time t = *thyme_now();                                            \
    thyme_hms(&t, h, m, s);                                             \
    a_remaining_result result = hrs3_remaining_(x, thyme_time(&t));     \
    if (!result.is_valid)                                               \
      TFAIL();                                                          \
    if (is_in_schedule != result.time_is_in_schedule)                   \
      TFAIL();                                                          \
    if (secs != result.seconds)                                         \
      TFAIL();                                                          \
  } while(0)
  X("830-12",  7,  0,  0, 0, 60 * 90);
#undef X
#define BAD(s) do {                                                     \
    a_day day;                                                          \
    if (OK == day_parse(s, sizeof(s)-1, &day))                          \
      TFAIL();                                                          \
  } while(0)
  BAD("U");
  BAD("1");
  BAD("-");
  BAD("13-12");
#undef BAD
}

void test_day_merge()
{
#define X(A, B, AB)                                             \
  do {                                                          \
    a_day a; if (day_parse(A, sizeof(A) - 1, &a)) TFAIL();      \
    a_day b; if (day_parse(B, sizeof(B) - 1, &b)) TFAIL();      \
    day_merge(&a, &b);                                          \
    if (strcmp(day_to_s_dup(&a), AB))                           \
      TFAILF(" '%s' vs '%s'", day_to_s_dup(&a), AB);            \
  } while(0)
  X("6-7",       "8-9", "6-7&8-9");
  X("6-7",       "7-8", "6-8");
  X("6-730",     "7-8", "6-8");
  X("6-730&8-9", "7-8", "6-9");
#undef X
}

void test_daily_remaining()
{
#define X(x, h, m, s, is_in_schedule, secs)                             \
  do {                                                                  \
    a_time t = *thyme_now();                                            \
    thyme_hms(&t, h, m, s);                                             \
    a_remaining_result result = hrs3_remaining_(x, thyme_time(&t));     \
    if (!result.is_valid)                                               \
      TFAIL();                                                          \
    if (is_in_schedule != result.time_is_in_schedule)                   \
      TFAIL();                                                          \
    if (secs != result.seconds)                                         \
      TFAILF(" %d vs %d", secs, result.seconds);                        \
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
}

void __attribute__((constructor)) test_daily()
{
  test_day_parse();
  test_day_merge();
  test_daily_remaining();
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "main.c"
#include "military.c"
#include "remaining.c"
#include "schedule.c"
#include "thyme.c"
#include "util.c"
#include "../hrs3.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o daily daily.c && ./daily"
 * End:
 */

#endif /* __daily_c__ */
