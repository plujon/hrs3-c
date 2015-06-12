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

bool time_range_overlaps_or_abuts(a_time_range *a, a_time_range *b)
{
  a_time *start = thyme_maximum(&a->start, &b->start);
  a_time *stop = thyme_minimum(&a->stop, &b->stop);
  return (0 <= thyme_diff(stop, start)) ? true : false;
}

a_time_range *time_range_overlap_alloc(a_time_range *range,
                                       a_time_range *during)
{
  a_time *start = thyme_maximum(&range->start, &during->start);
  a_time *stop = thyme_minimum(&range->stop, &during->stop);
  if (thyme_diff(stop, start) <= 0)
    return 0;
  a_time_range *ret = malloc(sizeof(a_time_range));
  memcpy(&ret->start, start, sizeof(a_time));
  memcpy(&ret->stop, stop, sizeof(a_time));
  return ret;
}

void time_range_copy(a_time_range *dest, a_time_range *src)
{
  memcpy(dest, src, sizeof(a_time_range));
}

void time_range_merge(a_time_range *dest, a_time_range *src)
{
  thyme_copy(&dest->start, thyme_minimum(&dest->start, &src->start));
  thyme_copy(&dest->stop, thyme_maximum(&dest->stop, &src->stop));
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

status time_range_verify(a_time_range *range)
{
  if (0 <= thyme_cmp(&range->start, &range->stop))
    return __LINE__;
  return OK;
}

status time_range_parse(const char *s, size_t len, a_time_range *range)
{
  const char *dash = strnchr(s, len, '-');
  if (!dash)
    return __LINE__;
  ptrdiff_t dash_offset = dash - s;
  if (len <= dash_offset)
    return __LINE__;
  NOD(thyme_parse(s, dash_offset, &range->start));
  len -= dash_offset - 1;
  s += dash_offset + 1;
  NOD(thyme_parse(s, len, &range->stop));
  NOD(time_range_verify(range));
  return OK;
}

size_t time_range_to_s(char *buffer, const a_time_range *range)
{
  size_t offset = 0;
  offset += thyme_to_s(&buffer[offset], &range->start);
  buffer[offset++] = '-';
  offset += thyme_to_s(&buffer[offset], &range->stop);
  return offset;
}

a_remaining_result time_range_remaining(a_time_range *range, a_time *t)
{
  int until_stop = thyme_diff(&range->stop, t);
  if (until_stop <= 0) {
    a_remaining_result result = { 1, 0, 0 };
    return result;
  }
  int until_start = thyme_diff(&range->start, t);
  if (until_start <= 0) {
    a_remaining_result result = { 1, 1, until_stop };
    return result;
  }
  a_remaining_result result = { 1, 0, until_start };
  return result;
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

void test_time_range_merge()
{
  const a_time *now = thyme_now();
  a_time later = thyme_clone(now); thyme_incr(&later, 60);
  a_time_range dest;
  time_range_init(&dest, now, 10);
  a_time_range src;
  time_range_init(&src, &later, 10);
  time_range_merge(&dest, &src);
  if (0 != thyme_cmp(&dest.start, now)) TFAIL();
  if (0 != thyme_cmp(&dest.stop, &src.stop)) TFAIL();
}

void __attribute__((constructor)) test_time_range()
{
  test_time_range_init();
  test_time_range_contains();
  test_time_range_merge();
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
