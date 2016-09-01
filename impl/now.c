#ifndef __now_c__
#define __now_c__

#include "impl.h"

/*
 * now_parse_nun gobbles 1 Number and UNit designator (nun) from s,
 * and returns the number of characters gobbled.
 *
 * Here are some example nuns:
 *
 *   1h - represents 1 hour
 *   2m - represents 2 minutes
 *   3s - represents 3 seconds
 */
static size_t now_parse_nun(const char *s, size_t len, int *acc)
{
  char *end = 0;
  int num = s_to_d(s, len, &end);
  if (0 == num && s == end)
    return 0;
  switch (*end++) {
  case 'h': num *= 3600; break;
  case 'm': num *= 60  ; break;
  case 's': num *= 1   ; break;
  default: return 0;
  }
  if (acc)
    *acc += num;
  return end - s;
}

static status now_parse_nuns(int *seconds, const char *s, size_t len)
{
  if (seconds)
    *seconds = 0;
  for (; len;) {
    size_t n_gobbled = now_parse_nun(s, len, seconds);
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
  if (!s) return NO;
  if (len < 6) return NO; /* now+1s */
  if ('n' != *s++ ||
      'o' != *s++ ||
      'w' != *s++ ||
      '+' != *s++)
    return NO;
  len -= 4;
  int *seconds = now_range ? &now_range->seconds : 0;
  return now_parse_nuns(seconds, s, len);
}

void now_destroy(a_now_range *now_range)
{
  return;
}

void now_to_time_range(const a_now_range *now_range, const a_time *time, struct a_time_range *range)
{
  time_range_init(range, time, now_range->seconds);
}

void now_add_to_schedule(const a_now_range *now_range, const a_time *time, struct a_schedule *schedule)
{
  a_time_range range_, *range = &range_;
  now_to_time_range(now_range, time, range);
  schedule_insert(schedule, range);
}

#if RUN_TESTS
#include <string.h>
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
