#ifndef __time_h__
#define __time_h__

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

int time_cmp(const a_time *a, const a_time *b);
int time_diff(const a_time *later, const a_time *earlier);
bool time_precedes(const a_time *earlier, const a_time *later);
void time_incr(a_time *t, int sec);
void time_next_day(a_time *t);
void time_next_week(a_time *t);
a_time time_plus(const a_time *t, int sec);
a_time *time_minimum(const a_time *a, const a_time *b);
a_time *time_maximum(const a_time *a, const a_time *b);
time_t time_time(const a_time *t);
const struct tm *time_tm(const a_time *t);
void time_init(a_time *t, time_t time);
a_time time_clone(const a_time *src);
void time_copy(a_time *dest, const a_time *src);
bool time_hms(a_time *t, int hour, int min, int sec);
bool time_whms(a_time *t, int wday, int hour, int min, int sec);
bool time_ymdhms(a_time *t, int year, int mon, int mday, int hour, int min, int sec);
status time_parse(a_time *time, const char *s, size_t len);
size_t time_to_s(const a_time *t, char *buffer);
const a_time *time_now();

#endif /* __time_h__ */
