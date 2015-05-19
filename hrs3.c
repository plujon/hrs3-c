#include "hrs3.h"
#include "daily.h"
#include <string.h>

typedef enum an_hrs3_kind {
  Unknown,
  Invalid,
  Daily,
  Weekdaily,
  Weekly,
  Biweekly,
  Raw
} an_hrs3_kind;

static an_hrs3_kind hrs3_kind(const char *s)
{
  if (!s) return Invalid;
  const char *dash = strchr(s, '-');
  if (!dash)
    return Invalid;
  char c = *s;
  if ('0' <= c && c <= '9') {
    /*
     * This is either a raw schedule such as
     * 20150516120100-20150516120200 or a daily schedule such as
     * 2015-2215 (8:15pm to 10:15pm) or
     * 20-21 (8pm to 9pm) or
     * 2000-21 (8pm to 9pm) or
     * 20-2100 (8pm to 9pm).
     */
    if (dash - s == 14)
      return Raw;
    else if (dash - s <= 4)
      return Daily;
    else
      return Invalid;
  }
  if ('P' ==  c)
    return Weekdaily;
  if (strchr("MTWRFAU", c))
    return Weekly;
  if ('B' ==  c)
    return Biweekly;
  return Invalid;
}

/*
 * hrs3_remaining evaluates whether t falls within the schedule noted
 * by s.  The return value x indicates both (a) whether t is within
 * the schedule and (b) the number of seconds after t that (a) is
 * valid.  A return value of -1 indicates an error.  Otherwise, the
 * high bit of x indicates (a), and all other bits indicate (b).
 */
static a_remaining_result hrs3_remaining(const char *s, time_t t)
{
  an_hrs3_kind kind = hrs3_kind(s);
  switch (kind) {
  case Invalid: return remaining_invalid();
  case Daily: return hrs3_daily_remaining(s, t);
  default: break;
  }
  return remaining_invalid();
}

int hrs3_remaining_in(const char *s, time_t t)
{
  /* TODO - canonicalize s (guarantee order and short form) */
  a_remaining_result result = hrs3_remaining(s, t);
  if (!result.is_valid)
    return -1;
  if (result.time_is_in_schedule)
    return result.seconds;
  return 0;
}

int hrs3_remaining_out(const char *s, time_t t)
{
  /* TODO - canonicalize s (guarantee order and short form) */
  a_remaining_result result = hrs3_remaining(s, t);
  if (!result.is_valid)
    return -1;
  if (result.time_is_in_schedule)
    return 0;
  return result.seconds;
}

#if TEST
#include "test.h"
int test_hrs3_kind()
{
#define X(x) if (!(x)) TFAIL();
  X(Invalid == hrs3_kind(0));
  X(Invalid == hrs3_kind(""));
  X(Daily == hrs3_kind("8-12"));
  X(Weekdaily == hrs3_kind("P8-12"));
  X(Weekly == hrs3_kind("MWF8-12"));
  X(Biweekly == hrs3_kind("BM8-12|T8-12"));
  X(Raw == hrs3_kind("20150516121900-20150516122000"));
#undef X
  return 0;
}

int test_hrs3_remaining_in()
{
  struct tm ymdhms;
  time_t t = time(0);
  localtime_r(&t, &ymdhms);
#define X(ret, x, h, m, s)                                   \
  do {                                                       \
    ymdhms.tm_hour = h;                                      \
    ymdhms.tm_min = m;                                       \
    ymdhms.tm_sec = s;                                       \
    t = mktime(&ymdhms);                                     \
    if (ret != hrs3_remaining_in(x, t)) TFAIL();             \
  } while(0)
  X(0,    "9-10",  8, 59, 59);
  X(3600, "9-10",  9,  0,  0);
  X(3599, "9-10",  9,  0,  1);
  X(   1, "9-10",  9, 59, 59);
  X(   0, "9-10", 10,  0,  0);
  X(  -1, "abc",   0,  0,  0);
#undef X
  return 0;
}

int test_hrs3_remaining_out()
{
  struct tm ymdhms;
  time_t t = time(0);
  localtime_r(&t, &ymdhms);
#define X(ret, x, h, m, s)                                   \
  do {                                                       \
    ymdhms.tm_hour = h;                                      \
    ymdhms.tm_min = m;                                       \
    ymdhms.tm_sec = s;                                       \
    t = mktime(&ymdhms);                                     \
    if (ret != hrs3_remaining_out(x, t)) TFAIL();            \
  } while(0)
  X(   1, "9-10",  8, 59, 59);
  X(   0, "9-10",  9,  0,  0);
  X(   0, "9-10",  9, 59, 59);
  X(3600 * 23, "9-10", 10,  0,  0);
  X(  -1, "abc",   0,  0,  0);
#undef X
  return 0;
}

int hrs3_main()
{
  int daily_main();
  return test_hrs3_kind() +
    test_hrs3_remaining_in() +
    test_hrs3_remaining_out() +
    daily_main() +
    0;
}

#undef main
#define main daily_main
#endif

#include "daily.c"

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -Dhrs3_main=main -g -o hrs3 hrs3.c && ./hrs3"
 * End:
 */
