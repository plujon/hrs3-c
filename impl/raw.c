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

status raw_parse(const char *s, size_t len, a_time_range *range)
{
  return time_range_parse(s, len, range);
}

#if RUN_TESTS

static a_remaining_result hrs3_remaining_(const char *hrsss, time_t time);

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
  X("20150528162400-20150528162500");
  X("20150101010100-20151231235900");
#undef X
#undef XX
#define BAD(x) do {                                     \
    if (OK == raw_parse(x, sizeof(x) - 1, &range))      \
      TFAIL();                                          \
  } while(0)
  BAD("20150101010200-20150101010100");
  BAD("20150101010200-20150101010200");
  BAD("20150101010200-20150101016000");
  BAD("20150101010100-2015010101000");
#undef BAD
}

void test_raw_remaining()
{
#define X(hrs3, ymdhms, is_in_schedule, secs) do {                      \
    a_time _t, *t = &_t;                                                \
    if (OK != (thyme_parse(ymdhms, sizeof(ymdhms)-1, t)))               \
      TFAIL();                                                          \
    a_remaining_result result = hrs3_remaining_(hrs3, thyme_time(t));   \
    if (!result.is_valid) TFAIL();                                      \
    if (result.time_is_in_schedule != is_in_schedule)                   \
      TFAIL();                                                          \
    if (result.seconds != secs) TFAIL();                                \
  } while(0)
  X("20150429120000-20150429120001", "20150429120000", 1, 1);
  X("20150429120000-20150429120001", "20150429120001", 0, 0);
  X("20150429120000-20150429120001", "20150429115959", 0, 1);
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
#include "../hrs3.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o raw raw.c && ./raw"
 * End:
 */

#endif /* __raw_c__ */
