#ifndef __time_range_c__
#define __time_range_c__

#include "impl.h"
#include <string.h>

a_time_range time_range_empty()
{
  static a_time_range x;
  return x;
}

bool time_range_contains(a_time_range *range, a_time *t)
{
  bool x = ttime_cmp(&range->start, t) <= 0;
  bool y = ttime_cmp(t, &range->stop) < 0;
  return x && y;
}

bool time_range_overlap_or_abut(a_time_range *a, a_time_range *b)
{
  a_time *start = ttime_max(&a->start, &b->start);
  a_time *stop = ttime_min(&a->stop, &b->stop);
  return (0 <= ttime_diff(stop, start)) ? true : false;
}

a_time_range *time_range_overlap_alloc(a_time_range *range,
                                       a_time_range *during)
{
  a_time *start = ttime_max(&range->start, &during->start);
  a_time *stop = ttime_min(&range->stop, &during->stop);
  if (ttime_diff(stop, start) <= 0)
    return 0;
  a_time_range *ret = malloc(sizeof(a_time_range));
  memcpy(&ret->start, start, sizeof(a_time));
  memcpy(&ret->stop, stop, sizeof(a_time));
  return ret;
}

void time_range_init_tm(a_time_range *range, struct tm *tm, int seconds)
{
  time_t time = mktime(tm);
  time_range_init_time(range, time, seconds);
}

void time_range_init_time(a_time_range *range, time_t time, int seconds)
{
  ttime_init(&range->start, time);
#if CHECK
  if (seconds < 0) {
    BUG();
  }
#endif
  ttime_init(&range->stop, time + seconds);
}

#if RUN_TESTS
int test_time_range_init()
{
  time_t now = time(0);
  a_time_range range;
  time_range_init_time(&range, now, 10);
  return 0;
}

int test_time_range_contains()
{
  a_time start = time_now();
  a_time prior = time_now(); time_incr(&prior, -10);
  a_time late  = time_now(); time_incr(&late,    9);
  a_time stop  = time_now(); time_incr(&stop,  10);
  a_time after = time_now(); time_incr(&after,  20);
  a_time_range range;
  time_range_init_time(&range, ttime_time(&start), 10);
  if ( time_range_contains(&range, &prior)) TFAIL();
  if (!time_range_contains(&range, &start)) TFAIL();
  if ( time_range_contains(&range, &stop)) TFAIL();
  if ( time_range_contains(&range, &after)) TFAIL();
  return 0;
}

void __attribute__((constructor)) test_time_range()
{
  test_time_range_init();
  test_time_range_contains();
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "tm_time.c"
#include "main.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o time_range time_range.c && ./time_range"
 * End:
 */

#endif /* __time_range_c__ */
