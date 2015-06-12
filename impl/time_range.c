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
  bool x = time_cmp(&range->start, t) <= 0;
  bool y = time_cmp(t, &range->stop) < 0;
  return x && y;
}

bool time_range_overlaps_or_abuts(a_time_range *a, a_time_range *b)
{
  a_time *start = time_maximum(&a->start, &b->start);
  a_time *stop = time_minimum(&a->stop, &b->stop);
  return (0 <= time_diff(stop, start)) ? true : false;
}

a_time_range *time_range_overlap_alloc(a_time_range *range,
                                       a_time_range *during)
{
  a_time *start = time_maximum(&range->start, &during->start);
  a_time *stop = time_minimum(&range->stop, &during->stop);
  if (time_diff(stop, start) <= 0)
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
  time_copy(&dest->start, time_minimum(&dest->start, &src->start));
  time_copy(&dest->stop, time_maximum(&dest->stop, &src->stop));
}

void time_range_init(a_time_range *range, const a_time *start, int seconds)
{
  time_copy(&range->start, start);
#if CHECK
  if (seconds < 0) {
    BUG();
  }
#endif
  time_copy(&range->stop, start);
  time_incr(&range->stop, seconds);
}

status time_range_verify(a_time_range *range)
{
  if (0 <= time_cmp(&range->start, &range->stop))
    return __LINE__;
  return OK;
}

status time_range_parse(a_time_range *range, const char *s, size_t len)
{
  const char *dash = strnchr(s, len, '-');
  if (!dash)
    return __LINE__;
  ptrdiff_t dash_offset = dash - s;
  if (len <= dash_offset)
    return __LINE__;
  NOD(time_parse(&range->start, s, dash_offset));
  len -= dash_offset - 1;
  s += dash_offset + 1;
  NOD(time_parse(&range->stop, s, len));
  NOD(time_range_verify(range));
  return OK;
}

size_t time_range_to_s(const a_time_range *range, char *buffer)
{
  size_t offset = 0;
  offset += time_to_s(&range->start, &buffer[offset]);
  buffer[offset++] = '-';
  offset += time_to_s(&range->stop, &buffer[offset]);
  return offset;
}

a_remaining_result time_range_remaining(a_time_range *range, a_time *t)
{
  int until_stop = time_diff(&range->stop, t);
  if (until_stop <= 0) {
    a_remaining_result result = { 1, 0, 0 };
    return result;
  }
  int until_start = time_diff(&range->start, t);
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
  time_range_init(&range, time_now(), 10);
}

void test_time_range_contains()
{
  const a_time *start = time_now();
  a_time prior = time_clone(start); time_incr(&prior, -10);
  a_time late  = time_clone(start); time_incr(&late,    9);
  a_time stop  = time_clone(start); time_incr(&stop,  10);
  a_time after = time_clone(start); time_incr(&after,  20);
  a_time_range range;
  time_range_init(&range, start, 10);
  if ( time_range_contains(&range, &prior)) TFAIL();
  if (!time_range_contains(&range, start)) TFAIL();
  if ( time_range_contains(&range, &stop)) TFAIL();
  if ( time_range_contains(&range, &after)) TFAIL();
}

void test_time_range_merge()
{
  const a_time *now = time_now();
  a_time later = time_clone(now); time_incr(&later, 60);
  a_time_range dest;
  time_range_init(&dest, now, 10);
  a_time_range src;
  time_range_init(&src, &later, 10);
  time_range_merge(&dest, &src);
  if (0 != time_cmp(&dest.start, now)) TFAIL();
  if (0 != time_cmp(&dest.stop, &src.stop)) TFAIL();
}

char *time_range_to_s_dup(a_time_range *range)
{
  char buffer[0x50];
  size_t offset = time_range_to_s(range, buffer);
  buffer[offset] = 0;
  return strdup(buffer);
}

void test_time_range_parse()
{
  a_time_range range;
#define XX(x, y) do {                                          \
    if (OK == time_range_parse(&range, x, sizeof(x) - 1)) {    \
      char *s = time_range_to_s_dup(&range);                   \
      if (strcmp(s, x)) TFAILF("\n%s vs\n%s", x, s);           \
    } else {                                                   \
      TFAIL();                                                 \
    }                                                          \
  } while(0)
#define X(x) XX(x, x)
  X("20150528162400-20150528162500");
  X("20150101010100-20151231235900");
#undef X
#undef XX
#define BAD(x) do {                                                   \
    if (OK == time_range_parse(&range, x, sizeof(x) - 1))             \
      TFAIL();                                                        \
  } while(0)
  BAD("20150101010200-20150101010100");
  BAD("20150101010200-20150101010200");
  BAD("20150101010200-20150101016000");
  BAD("20150101010100-2015010101000");
#undef BAD
}

void __attribute__((constructor)) test_time_range()
{
  test_time_range_init();
  test_time_range_contains();
  test_time_range_merge();
  test_time_range_parse();
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "time.c"
#include "main.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o time_range time_range.c && ./time_range"
 * End:
 */

#endif /* __time_range_c__ */
