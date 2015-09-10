#ifndef __hrs3_c__
#define __hrs3_c__

#include "impl/impl.h"
#include <string.h>

/*
 * hrs3_remaining evaluates whether t falls within the schedule noted
 * by hrsss.  The return value x indicates both (a) whether t is within
 * the schedule and (b) the number of seconds after t that (a) is
 * valid.  A return value of -1 indicates an error.  Otherwise, the
 * high bit of x indicates (a), and all other bits indicate (b).
 */
static a_remaining_result hrs3_remaining__(const char *hrsss, time_t time)
{
  a_hrs3 hrs3;
  if (OK != hrs3_init(&hrs3, hrsss, strlen(hrsss)))
    return remaining_invalid();
  a_time t;
  time_init(&t, time);
  a_remaining_result result = hrs3_remaining(&hrs3, &t);
  hrs3_destroy(&hrs3);
  return result;
}

static a_remaining_result hrs3_remaining_(const char *hrsss_in, time_t time)
{
  const char *hrsss = hrsss_in;
  if (strchr(hrsss_in, ':')) {
    char *s = strdup(hrsss_in);
    remove_char(s, ':');
    hrsss = s; 
  }
  a_remaining_result result = hrs3_remaining__(hrsss, time);
  if (hrsss != hrsss_in)
    free((char *)hrsss);
  return result;
}

int hrs3_remaining_in(const char *hrsss, time_t t)
{
  a_remaining_result result = hrs3_remaining_(hrsss, t);
  if (!result.is_valid)
    return -1;
  if (result.time_is_in_schedule)
    return result.seconds;
  return 0;
}

int hrs3_remaining_out(const char *hrsss, time_t t)
{
  a_remaining_result result = hrs3_remaining_(hrsss, t);
  if (!result.is_valid)
    return -1;
  if (result.time_is_in_schedule)
    return 0;
  return result.seconds;
}

#if RUN_TESTS

int test_hrs3_remaining_in()
{
  struct tm ymdhms;
  time_t t = time(0);
  LOCALTIME_R(&t, &ymdhms);
#define X(ret, x, h, m, s)                                   \
  do {                                                       \
    ymdhms.tm_hour = h;                                      \
    ymdhms.tm_min = m;                                       \
    ymdhms.tm_sec = s;                                       \
    t = mktime(&ymdhms);                                     \
    if (ret != hrs3_remaining_in(x, t)) TFAIL();             \
  } while_0
  X(0,    "9-10",  8, 59, 59);
  X(3600, "9-10",  9,  0,  0);
  X(3599, "9-10",  9,  0,  1);
  X(   1, "9-10",  9, 59, 59);
  X(   0, "9-10", 10,  0,  0);
  X(   0, "9:00-10:00", 10,  0,  0);
  X(  -1, "abc",   0,  0,  0);
#undef X
  return 0;
}

int test_hrs3_remaining_out()
{
  struct tm ymdhms;
  time_t t = time(0);
  LOCALTIME_R(&t, &ymdhms);
#define X(ret, x, h, m, s)                                   \
  do {                                                       \
    ymdhms.tm_hour = h;                                      \
    ymdhms.tm_min = m;                                       \
    ymdhms.tm_sec = s;                                       \
    t = mktime(&ymdhms);                                     \
    if (ret != hrs3_remaining_out(x, t)) TFAIL();            \
  } while_0
  X(   1, "9-10",  8, 59, 59);
  X(   0, "9-10",  9,  0,  0);
  X(   0, "9-10",  9, 59, 59);
  X(3600 * 23, "9-10", 10,  0,  0);
  X(  -1, "abc",   0,  0,  0);
#undef X
  return 0;
}

PRE_INIT(test_hrs3)
{
  test_hrs3_remaining_in();
  test_hrs3_remaining_out();
}
#endif /* RUN_TESTS */

#ifdef ONE_OBJ
#include "impl/impl.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o hrs3 hrs3.c && ./hrs3"
 * End:
 */

#endif /* __hrs3_c__ */
