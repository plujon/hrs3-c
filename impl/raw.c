#ifndef __raw_c__
#define __raw_c__

#include "impl.h"
#include <string.h>
#include <time.h>

size_t raw_to_s(char *buffer, a_time_range *range)
{
  size_t offset = 0;
  offset += thyme_to_s(&buffer[offset], &range->start);
  buffer[offset++] = '-';
  offset += thyme_to_s(&buffer[offset], &range->stop);
  return offset;
}

char *raw_to_s_dup(a_time_range *range)
{
  char buffer[0x50];
  size_t offset = raw_to_s(buffer, range);
  buffer[offset] = 0;
  return strdup(buffer);
}

status time_range_verify(a_time_range *range)
{
  if (0 <= thyme_cmp(&range->start, &range->stop))
    return __LINE__;
  return OK;
}

status raw_parse(const char *s, size_t len, a_time_range *range)
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

a_remaining_result raw_remaining(const char *s, a_time *t)
{
  a_time_range range;
  if (OK != raw_parse(s, strlen(s), &range))
    return remaining_invalid();
  int until_stop = thyme_diff(&range.stop, t);
  if (until_stop <= 0) {
    a_remaining_result result = { 1, 0, 0 };
    return result;
  }
  int until_start = thyme_diff(&range.start, t);
  if (until_start <= 0) {
    a_remaining_result result = { 1, 1, until_stop };
    return result;
  }
  a_remaining_result result = { 1, 0, until_start };
  return result;
}

#if RUN_TESTS
void test_raw_parse()
{
  a_time_range range;
#define XX(x, y) do {                                   \
    if (OK == raw_parse(x, sizeof(x) - 1, &range)) {    \
      char *s = raw_to_s_dup(&range);                   \
      if (strcmp(s, x)) TFAILF("\n%s vs\n%s", x, s);    \
    } else {                                            \
      TFAIL();                                          \
    }                                                   \
  } while(0)
#define X(x) XX(x, x)
  X("201505281624-201505281625");
  X("201501010101-201512312359");
#undef X
#undef XX
#define BAD(x) do {                                     \
    if (OK == raw_parse(x, sizeof(x) - 1, &range))      \
      TFAIL();                                          \
  } while(0)
  BAD("201501010102-201501010101");
  BAD("201501010102-201501010102");
  BAD("201501010102-201501010160");
  BAD("201501010101-20150101010");
#undef BAD
}

void test_raw_remaining()
{
#define X(hrs3, ymdhms, is_in_schedule, secs) do {        \
    a_time _t, *t = &_t;                                  \
    if (OK != (thyme_parse(ymdhms, sizeof(ymdhms)-1, t))) \
      TFAIL();                                            \
    a_remaining_result result = raw_remaining(hrs3, t);   \
    if (!result.is_valid) TFAIL();                        \
    if (result.time_is_in_schedule != is_in_schedule)     \
      TFAIL();                                            \
    if (result.seconds != secs) TFAIL();                  \
  } while(0)
  X("201504291200-201504291201", "201504291200", 1, 60);
  X("201504291200-201504291201", "201504291201", 0, 0);
  X("201504291200-201504291201", "201504291159", 0, 60);
#undef X
}

void __attribute__((constructor)) test_raw()
{
  test_raw_parse();
  test_raw_remaining();
}

#include "remaining.c"
#include "time_range.c"
#include "util.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o raw raw.c && ./raw"
 * End:
 */

#endif /* __raw_c__ */
