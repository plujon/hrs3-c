#ifndef __schedule_h__
#define __schedule_h__

#include "remaining.h"
#include "time_range.h"
#include <stddef.h>

typedef struct a_schedule {
  int capacity;
  int n_ranges;
  a_time_range *ranges;
} a_schedule;

void schedule_init(a_schedule *schedule);
void schedule_destroy(a_schedule *schedule);
void schedule_grow(a_schedule *schedule);
void schedule_insert(a_schedule *schedule, struct a_time_range *range);
a_remaining_result schedule_remaining(const a_schedule *schedule, const struct a_time *t);
size_t schedule_to_s(const a_schedule *schedule, char *buffer, size_t size);

#endif /* __schedule_h__ */
