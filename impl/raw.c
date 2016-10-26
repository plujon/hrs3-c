#ifndef __raw_c__
#define __raw_c__

#include "impl.h"
#include <string.h>
#include <time.h>

#if RUN_TESTS
static a_remaining_result hrs3_remaining_(const char *hrsss, time_t time);

void test_raw_remaining(void)
{
#define X(hrs3, ymdhms, is_in_schedule, secs) do {                      \
    a_time _t, *t = &_t;                                                \
    if (OK != (time_parse(t, ymdhms, sizeof(ymdhms)-1)))                \
      TFAIL();                                                          \
    a_remaining_result result = hrs3_remaining_(hrs3, time_time(t));   \
    if (!result.is_valid) TFAIL();                                      \
    if (result.time_is_in_schedule != is_in_schedule)                   \
      TFAIL();                                                          \
    if (result.seconds != secs) TFAIL();                                \
  } while_0
  X("20150429120000-20150429120001", "20150429120000", 1, 1);
  X("20150429120000-20150429120001", "20150429120001", 0, 0);
  X("20150429120000-20150429120001", "20150429115959", 0, 1);
#undef X
}

PRE_INIT(test_raw)
{
  test_raw_remaining();
}
#endif

#if ONE_OBJ
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
