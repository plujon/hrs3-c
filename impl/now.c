#ifndef __now_c__
#define __now_c__

#include "impl.h"
#include <string.h>

/*
 * now_parse_nun gobbles 1 Number and UNit designator (nun) from s,
 * and returns the number of characters gobbled.
 *
 * Here are some example nuns:
 *
 *   1d - represents 1 day
 *   1h - represents 1 hour
 *   2m - represents 2 minutes
 *   3s - represents 3 seconds
 */
static size_t now_parse_nun(a_now_range *now_range, const char *s, size_t len)
{
  char *end = 0;
  int num = s_to_d(s, len, &end);
  if (0 == num && s == end)
    return 0;
  switch (*end++) {
  case 'd': now_range->days += num           ; break;
  case 'h': now_range->seconds += 3600 * num ; break;
  case 'm': now_range->seconds +=   60 * num ; break;
  case 's': now_range->seconds +=    1 * num ; break;
  default: return 0;
  }
  return end - s;
}

static status now_parse_nuns(a_now_range *now_range, const char *s, size_t len)
{
  for (; len;) {
    size_t n_gobbled = now_parse_nun(now_range, s, len);
    if (0 == n_gobbled)
      break;
    if (len < n_gobbled) {
      BUG();
      return NO;
    }
    len -= n_gobbled;
    s += n_gobbled;
  }
  if (len)
    return NO;
  return OK;
}

status now_init(a_now_range *now_range, const char *s, size_t len)
{
  a_now_range dummy_;
  if (!now_range)
    now_range = &dummy_;
  now_range->seconds = 0;
  now_range->days = 0;
  if (!s || 0 == len) return NO;
  if (4 < len && 0 == strncmp("now-", s, 4)) {
    s += 4;
    len -= 4;
  }
  if (4 < len && 0 == strncmp("now+", s, 4)) {
    s += 4;
    len -= 4;
    return now_parse_nuns(now_range, s, len);
  }
  return NO;
}

void now_to_time_range(const a_now_range *now_range, const a_time *time, struct a_time_range *range)
{
  time_range_init(range, time, 0);
  if (0 != now_range->days)
    time_incr_days(&range->stop, now_range->days);
  time_incr(&range->stop, now_range->seconds);
}

void now_add_to_schedule(const a_now_range *now_range, const a_time *time, struct a_schedule *schedule)
{
  a_time_range range_, *range = &range_;
  now_to_time_range(now_range, time, range);
  schedule_insert(schedule, range);
}

#if RUN_TESTS
void test_now_init()
{
#define X(S, SECS)                                                    \
  do {                                                                \
    a_now_range now_range;                                            \
    if (OK != now_init(&now_range, S, sizeof(S)-1))                   \
      TFAIL();                                                        \
    if (now_range.seconds != SECS)                                    \
      TFAIL();                                                        \
  } while_0;
  X("now+1s",  1);
  X("now+1m",  60);
  X("now+1m1s",  61);
  X("now+1h1m1s",  3661);
  X("now+100h",  360000);
  X("now+1s1m1s",  62); /* undocumented feature */
  X("now-now+1s",  1);
#undef X
#define BAD(S)                                                        \
  do {                                                                \
    a_now_range now_range;                                            \
    if (OK == now_init(&now_range, S, sizeof(S)-1))                   \
      TFAIL();                                                        \
  } while_0;
  BAD("now");
  BAD("now+");
  BAD("now+1");
  BAD("now+s");
  BAD("now-");
  BAD("now-now");
  BAD("now-now+");
  BAD("now-now+1");
#undef BAD
}

void test_now_add_to_schedule()
{
  const a_time *now = time_now();
  char buffer[sizeof("20160902120000-20160902123000")];
  size_t size = sizeof(buffer);
#define X(S, SECONDS)                                                 \
  do {                                                                \
    a_time time_ = time_clone(now), *time = &time_;                   \
    a_schedule schedule_, *schedule = &schedule_;                     \
    schedule_init(schedule);                                          \
    a_now_range now_range;                                            \
    if (OK != now_init(&now_range, S, sizeof(S)-1))                   \
      TFAIL();                                                        \
    now_add_to_schedule(&now_range, time, schedule);                  \
    char *p = &buffer[0];                                             \
    size_t offset = 0;                                                \
    offset += strftime(&p[offset], size - offset, "%Y%m%d%H%M%S", time_tm(time)); \
    p[offset++] = '-';                                                \
    time_incr(time, SECONDS);                                         \
    offset += strftime(&p[offset], size - offset, "%Y%m%d%H%M%S", time_tm(time)); \
    char buf2[sizeof(buffer)] = { 0 };                                \
    offset = schedule_to_s(schedule, buf2, sizeof(buf2));             \
    buf2[offset] = 0;                                                 \
    if (0 != strcmp(buffer, buf2)) {                                  \
      TFAILF("%s != %s", buffer, buf2);                               \
    }                                                                 \
  } while_0;
  X("now+30m", 30 * 60);
  X("now+1h", 3600);
#undef X
}

PRE_INIT(test_now)
{
  test_now_init();
  test_now_add_to_schedule();
}
#endif

#if ONE_OBJ
#include "main.c"
#include "time.c"
#include "../hrs3.c"
#endif

/*
 * Local Variables:
 * compile-command: "gcc -Wall -DTEST -g -o now now.c && ./now"
 * End:
 */

#endif /* __now_c__ */
