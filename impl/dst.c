#ifndef __dst_c__
#define __dst_c__

#include "impl.h"

/*
 * hrsss --- hrs3_parse() --> hrs3 --- to_schedule() --> schedule
 *                                      |       ^
 *                                      v       | 
 *                                      +- dst -+
 */

#if RUN_TESTS
#include <string.h>

void test_dst_()
{
#define X(HRS3, TOD, T_OFFSET, IS_IN, SECS)                             \
  do {                                                                  \
    a_hrs3 _hrs3, *hrs3 = &_hrs3;                                       \
    char hrsss[0x40];                                                   \
    memcpy(hrsss, HRS3, sizeof(HRS3));                                  \
    remove_char(hrsss, ':');                                            \
    if (OK != hrs3_init(hrs3, hrsss, strlen(hrsss))) {                  \
      TFAILF(" failed to parse %s", hrsss);                             \
      break;                                                            \
    }                                                                   \
    char time_str[0x40];                                                \
    memcpy(time_str, DATE TOD, sizeof(DATE TOD));                       \
    remove_char(time_str, ':');                                         \
    a_time time_, *t = &time_;                                          \
    if (OK != time_parse(t, time_str, strlen(time_str))) {              \
      TFAILF(" failed to parse %s", time_str);                          \
      break;                                                            \
    }                                                                   \
    time_incr(t, T_OFFSET);                                            \
    a_remaining_result result = hrs3_remaining(hrs3, t);                \
    if (!result.is_valid)                                               \
      TFAILF(" %s", "not valid");                                       \
    else if (IS_IN != result.time_is_in_schedule)                       \
      TFAILF(" %s in schedule", (IS_IN ? "is not" : "is"));             \
    else if (SECS != result.seconds)                                    \
      TFAILF(" %d vs %d", SECS, result.seconds);                        \
  } while(0)
#define IN(HRS3, T, T_OFFSET, SECS) X( HRS3, T, T_OFFSET, 1, SECS)
#define OUT(HRS3, T, T_OFFSET, SECS) X( HRS3, T, T_OFFSET, 0, SECS)
  /*
   * In the fall, 1am to 2am happens twice on a certain day, and in
   * the spring, 2am to 3am is skipped on a certain day (spring
   * forward, fall back).  The following tests verify that hrs3
   * handles schedules that
   *
   * 1. occur during part or all of one of the 2 "weird" periods.
   *
   * 2. do not occur during one of the weird periods, but whose
   * remaining result needs to take the weird periods into account
   * (and have either one more hour or one less hour).
   */
  /* Schedules that occur during 1am to 2am, etc. */
  /* Schedules that straddle 2am to 3am */
  /* Schedules that are before 2am, and remaining before, during, and after */
  /* Schedules that are after 3am, and remaining before, during, and after */
  /* Schedules that are after 2am to 3am and the  */
  /* SPRING FORWARD at 2015-03-08 02:00:00 */
#define DATE "20150308"
  /* at 1:59:59 */
  /* hrs3 straddling 2am */
  X("1:59-2:00", "01:59:59", 0, 1, 1);
  X("1:59-2:01", "01:59:59", 0, 1, 61);
  X("1:59-3:00", "01:59:59", 0, 1, 1);
  X("1:59-3:01", "01:59:59", 0, 1, 1 + 60);

  /* hrs3 at 2am */
  X("2-3"    , "01:59:59", 0, 0, 1);
  X("2-2:59"  , "01:59:59", 0, 0, 1);

  /* hrs3 straddling 3am */
  X("2-3:01"  , "01:59:59", 0, 0, 1);
  X("3-3:01"  , "01:59:59", 0, 0, 1);

  /* hrs3 straddling 2am and 3am */
  X("1:59-3:01", "01:59:59", 0, 1, 61);

  /* hrs3 after 3am */
  X("3:01-4", "01:59:59", 0, 0, 61);

  /* TODO: raw, weekly, biweekly, and weekdaily schedules */

  /* FALL BACK at 2015-11-01 02:00:00 */
#undef DATE
#define DATE "20151101"
  /* at 00:59:59 */
  X("00:59-1", "00:59:59",  0, 1, 1);
  X("00:58-00:59", "00:59:00",  0, 0, 3600 * 25 - 60);
  X("1-1:01", "00:59:59",  0, 0, 1);
  X("00:59-2", "00:59:59",  0, 1, 1 + 3600 * 2);
  X("00:59-3", "00:59:59",  0, 1, 1 + 3600 * 3);

  /* at 1:00:00, take 1 */
  X("00:59-1", "01:00:00",  0, 0, 3600 * 25 - 60);
  X("00:59-1:01", "01:00:00",  0, 1, 60);
  X("00:59-2", "01:00:00",  0, 1, 3600 * 2);
  X("1-1:01", "01:00:00",  0, 1, 60);
  X("1-2",   "01:00:00",  0, 1, 3600 * 2);
  X("0-4",   "01:00:00",  0, 1, 3600 * 4);
  X("0-4",   "01:00:00",  0, 1, 3600 * 4);
  X("3-4",   "01:00:00",  0, 0, 3600 * 3);

  /* at 1:00:00, take 2 */
  X("00:59-1", "01:00:00",  3600, 0, 3600 * 24 - 60);
  X("00:59-1:01", "01:00:00",  3600, 1, 60);
  X("00:59-2", "01:00:00",  3600, 1, 3600);
  X("1-1:01", "01:00:00",  3600, 1, 60);
  X("1-2",   "01:00:00",  3600, 1, 3600 * 1);
  X("0-4",   "01:00:00",  3600, 1, 3600 * 3);
  X("0-4",   "01:00:00",  3600, 1, 3600 * 3);
  X("3-4",   "01:00:00",  3600, 0, 3600 * 2);

  /* at 2:00:00 */
  X("1-1:01", "02:00:00",   0,  0, 3600 * 23);

#undef DATE
#undef X
#undef IN
#undef OUT
}

PRE_INIT(test_dst)
{
  test_dst_();
}
#endif /* RUN_TESTS */

#if ONE_OBJ
#include "a_hrs3.c"
#include "main.c"
#include "util.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o dst dst.c && ./dst"
 * End:
 */

#endif /* __dst_c__ */
