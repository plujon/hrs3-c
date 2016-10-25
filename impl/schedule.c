#ifndef __schedule_c__
#define __schedule_c__

#include "impl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static a_schedule schedule_empty(void)
{
  static a_schedule empty_schedule;
  return empty_schedule;
}

a_schedule *schedule_create(void)
{
  a_schedule *schedule = malloc(sizeof(a_schedule));
  schedule_init(schedule);
  return schedule;
}

void schedule_init(a_schedule *schedule)
{
  *schedule = schedule_empty();
}

void schedule_destroy(a_schedule *schedule)
{
  if (schedule->ranges)
    free(schedule->ranges);
  *schedule = schedule_empty();
}

void schedule_grow(a_schedule *schedule)
{
  if (0 == schedule->capacity) {
    schedule->capacity = 8;
    schedule->ranges = malloc(sizeof(a_time_range) * schedule->capacity);
  } else {
    schedule->capacity *= 2;
    schedule->ranges = realloc(schedule->ranges,
                               sizeof(a_time_range) * schedule->capacity);
  }
}

bool schedule_has_range(a_schedule *schedule, a_time_range *range)
{
  int i = 0;
  for (; i < schedule->n_ranges; ++i) {
    a_time_range *x = &schedule->ranges[i];
    if (0 == time_cmp(&x->start, &range->start) &&
        0 == time_cmp(&x->stop, &range->stop))
      return true;
  }
  return false;
}

static void schedule_insert_at(a_schedule *schedule, int index, a_time_range *range)
{
  while (schedule->capacity <= schedule->n_ranges) {
    schedule_grow(schedule);
  }
  void *dest = &schedule->ranges[index + 1];
  void *src = &schedule->ranges[index];
  size_t size = sizeof(a_schedule) * (schedule->n_ranges - index);
  memmove(dest, src, size);
  time_range_copy(&schedule->ranges[index], range);
  schedule->n_ranges++;
}

/*
 * schedule_insert inserts a new range into a schedule.  If the range
 * overlaps with one or more ranges already in schedule, then the
 * overlapping ranges are merged together.  Note that it is possible
 * for the number of ranges in schedule to decrease as a result of
 * insertion because of this merging.
 *
 * For example, given:
 *
 * ^    ^ ^  ^  ^      ^
 * +----+ +--+  +------+  <- schedule
 *
 *  ^                ^
 *  +----------------+  <- range
 *
 * Produce:
 *
 * ^                   ^
 * +-------------------+  <- schedule
 */
void schedule_insert(a_schedule *schedule, a_time_range *range)
{
  bool inserted_or_merged = false;
  int i = 0;
  for(; i < schedule->n_ranges; ++i) {
    a_time_range *x = &schedule->ranges[i];
    if (!inserted_or_merged && 0 < time_diff(&range->start, &x->stop)) {
      /* x->stop precedes range->start, so continue to next x */
      continue;
    }
    if (time_range_overlaps_or_abuts(x, range)) {
      /* range and x overlap, so merge all overlapping ranges */
      time_range_merge(x, range);
      if (inserted_or_merged)
        schedule->n_ranges--;
      inserted_or_merged = true;
      continue;
    }
    if (!inserted_or_merged) {
      /* range precedes x */
      schedule_insert_at(schedule, i, range);
      inserted_or_merged = true;
    }
  }
  if (!inserted_or_merged) {
    schedule_insert_at(schedule, i, range);
  }
}

a_remaining_result schedule_remaining(const a_schedule *schedule, const a_time *t)
{
  int i = 0;
  for(; i < schedule->n_ranges; ++i) {
    a_time_range *range = &schedule->ranges[i];
    if (!time_precedes(t, &range->stop))
      continue;
    int seconds = 0;
    bool is_in_range = false;
    if (time_range_contains(range, t)) {
      is_in_range = true;
      seconds = time_diff(&range->stop, t);
    } else {
      is_in_range = false;
      seconds = time_diff(&range->start, t);
    }
    a_remaining_result result = { 1, is_in_range, seconds };
    return result;
  }
  a_remaining_result result = { 1, false, 0 };
  return result;
}

size_t schedule_to_s(const a_schedule *schedule, char *buffer, size_t size)
{
  size_t offset = 0;
  int i = 0;
  for(; i < schedule->n_ranges; ++i) {
    if (1 <= i && size < offset)
      buffer[offset++] = ' ';
    a_time_range *x = &schedule->ranges[i];
    if (size < offset + TIME_RANGE_STR_SIZE + 1)
      break;
    offset += time_range_to_s(x, &buffer[offset]);
  }
  return offset;
}

#if RUN_TESTS
static char *schedule_to_s_(a_schedule *schedule)
{
  static char buffer[0x400];
  schedule_to_s(schedule, buffer, sizeof(buffer)-1);
  return buffer;
}

PRE_INIT(test_schedule)
{
  a_time_range range = time_range_empty();
  a_schedule s = schedule_empty();
  a_time now = *time_now();
  a_time later = *time_now();
#define X(start, seconds, num) do {                                     \
    a_time_range r;                                                     \
    time_range_init(&r, start, seconds);                                \
    schedule_insert(&s, &r);                                            \
    if (num != s.n_ranges)                                              \
      TFAILF(" %d vs %d in %s", num, s.n_ranges, schedule_to_s_(&s));   \
  } while_0
  X(&now, 10, 1);
  time_range_init(&range, &now, 10);
  if (!schedule_has_range(&s, &range)) TFAIL();
  time_incr(&now, -1);
  X(&now, 1, 1);
  time_range_init(&range, &now, 11);
  if (!schedule_has_range(&s, &range)) TFAIL();
  time_incr(&now, -1);
  X(&now, 3, 1);
  time_range_init(&range, &now, 12);
  if (!schedule_has_range(&s, &range)) TFAIL();
  time_incr(&later, 10);
  X(&later, 2, 1);
  time_range_init(&range, &now, 14);
  if (!schedule_has_range(&s, &range)) TFAIL();
  time_incr(&later, 3);
  X(&later, 2, 2);
  time_range_init(&range, &later, 2);
  if (!schedule_has_range(&s, &range)) TFAIL();
  X(&now, 30, 1);
  time_range_init(&range, &now, 30);
  if (!schedule_has_range(&s, &range)) TFAIL();
#undef X
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "main.c"
#include "remaining.c"
#include "time_range.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o schedule schedule.c && ./schedule"
 * End:
 */

#endif /* __schedule_c__ */
