#ifndef __daily_h__
#define __daily_h__

#include "remaining.h"
#include "military.h"
#include "tm_time.h"

typedef struct a_day {
  int capacity;
  int n_ranges;
  a_military_range *ranges;
} a_day;

a_remaining_result daily_remaining(const char *s, a_time *t);

status day_parse(const char *s, size_t len, a_day *day);
void day_clone(a_day *dest, a_day *src);
void day_copy(a_day *dest, a_day *src);
void day_destroy(a_day *day);
void day_merge(a_day *dest, a_day *src);

#endif /* __daily_h__ */
