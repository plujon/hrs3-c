#ifndef __thyme_h__
#define __thyme_h__

#include <time.h>

#define THYME_STR_SIZE (sizeof("ccyymmddhhmmss") - 1)

/*
 * Sometimes we want a timestamp (time_t), and sometimes a struct tm
 * (in localtime).
 */
typedef struct a_time {
  time_t time;  /* 0 means unset */
  struct tm tm; /* a tm_year of 0 means unset */
} a_time;

int thyme_cmp(const a_time *a, const a_time *b);
int thyme_diff(const a_time *later, const a_time *earlier);
bool thyme_precedes(const a_time *earlier, const a_time *later);
void thyme_incr(a_time *t, int sec);
void thyme_next_day(a_time *t);
void thyme_next_week(a_time *t);
a_time thyme_plus(const a_time *t, int sec);
a_time *thyme_minimum(const a_time *a, const a_time *b);
a_time *thyme_maximum(const a_time *a, const a_time *b);
time_t thyme_time(const a_time *t);
const struct tm *thyme_tm(const a_time *t);
void thyme_init(a_time *t, time_t time);
a_time thyme_clone(const a_time *src);
void thyme_copy(a_time *dest, const a_time *src);
bool thyme_hms(a_time *t, int hour, int min, int sec);
bool thyme_whms(a_time *t, int wday, int hour, int min, int sec);
bool thyme_ymdhms(a_time *t, int year, int mon, int mday, int hour, int min, int sec);
status thyme_parse(const char *s, size_t len, a_time *out);
size_t thyme_to_s(char *buffer, const a_time *t);
const a_time *thyme_now();

#endif /* __thyme_h__ */
