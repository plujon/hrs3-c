#ifndef __daily_h__
#define __daily_h__

#include "remaining.h"
#include "military.h"
#include "time.h"

typedef struct a_day {
  int capacity;
  int n_ranges;
  a_military_range *ranges;
} a_day;

void day_add_to_schedule(const a_day *, const a_time *time, struct a_schedule *schedule);
status day_init(a_day *day, const char *s, size_t len);
void day_clone(a_day *dest, const a_day *src);
void day_copy(a_day *dest, const a_day *src);
void day_destroy(a_day *day);
void day_merge(a_day *dest, const a_day *src);

#endif /* __daily_h__ */
