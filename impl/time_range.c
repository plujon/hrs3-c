#ifndef __time_range_c__
#define __time_range_c__

#include "impl.h"
#include <string.h>

a_time_range time_range_empty()
{
  static a_time_range x;
  return x;
}

bool time_range_contains(a_time_range *range, const a_time *t)
{
  bool x = thyme_cmp(&range->start, t) <= 0;
  bool y = thyme_cmp(t, &range->stop) < 0;
  return x && y;
}

bool time_range_overlap_or_abut(a_time_range *a, a_time_range *b)
{
  a_time *start = thyme_max(&a->start, &b->start);
  a_time *stop = thyme_min(&a->stop, &b->stop);
  return (0 <= thyme_diff(stop, start)) ? true : false;
}

a_time_range *time_range_overlap_alloc(a_time_range *range,
                                       a_time_range *during)
{
  a_time *start = thyme_max(&range->start, &during->start);
  a_time *stop = thyme_min(&range->stop, &during->stop);
  if (thyme_diff(stop, start) <= 0)
    return 0;
  a_time_range *ret = malloc(sizeof(a_time_range));
  memcpy(&ret->start, start, sizeof(a_time));
  memcpy(&ret->stop, stop, sizeof(a_time));
  return ret;
}

void time_range_init(a_time_range *range, const a_time *start, int seconds)
{
  thyme_copy(&range->start, start);
#if CHECK
  if (seconds < 0) {
    BUG();
  }
#endif
  thyme_copy(&range->stop, start);
  thyme_incr(&range->stop, seconds);
}

#if RUN_TESTS
void test_time_range_init()
{
  a_time_range range;
  time_range_init(&range, thyme_now(), 10);
}

void test_time_range_contains()
{
  const a_time *start = thyme_now();
  a_time prior = thyme_clone(start); thyme_incr(&prior, -10);
  a_time late  = thyme_clone(start); thyme_incr(&late,    9);
  a_time stop  = thyme_clone(start); thyme_incr(&stop,  10);
  a_time after = thyme_clone(start); thyme_incr(&after,  20);
  a_time_range range;
  time_range_init(&range, start, 10);
  if ( time_range_contains(&range, &prior)) TFAIL();
  if (!time_range_contains(&range, start)) TFAIL();
  if ( time_range_contains(&range, &stop)) TFAIL();
  if ( time_range_contains(&range, &after)) TFAIL();
}

void __attribute__((constructor)) test_time_range()
{
  test_time_range_init();
  test_time_range_contains();
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "thyme.c"
#include "main.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o time_range time_range.c && ./time_range"
 * End:
 */

#endif /* __time_range_c__ */
