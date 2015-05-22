#ifndef __time_range_h__
#define __time_range_h__

#include "tm_time.h"

typedef struct a_time_range {
  a_time start;
  a_time stop;
} a_time_range;

a_time_range time_range_empty();
bool time_range_contains(a_time_range *range, a_time *t);
void time_range_init_tm(a_time_range *range, struct tm *tm, int seconds);
void time_range_init_time(a_time_range *range, time_t time, int seconds);
void time_range_copy(a_time_range *dest, a_time_range *src);
a_time_range *time_range_overlap_alloc(a_time_range *range,
                                       a_time_range *during);
bool time_range_overlap_or_abut(a_time_range *a, a_time_range *b);

#endif /* __time_range_h__ */
