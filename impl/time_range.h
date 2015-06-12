#ifndef __time_range_h__
#define __time_range_h__

#include "time.h"
#include "remaining.h"

#define TIME_RANGE_STR_SIZE (2 * THYME_STR_SIZE + 1)

typedef struct a_time_range {
  a_time start;
  a_time stop;
} a_time_range;

a_time_range time_range_empty();
bool time_range_contains(a_time_range *range, const a_time *t);
void time_range_init(a_time_range *range, const a_time *start, int seconds);
void time_range_copy(a_time_range *dest, a_time_range *src);
void time_range_merge(a_time_range *dest, a_time_range *src);
a_time_range *time_range_overlap_alloc(a_time_range *range,
                                       a_time_range *during);
bool time_range_overlaps_or_abuts(a_time_range *a, a_time_range *b);
status time_range_parse(a_time_range *range, const char *s, size_t len);
size_t time_range_to_s(const a_time_range *range, char *buffer);
a_remaining_result time_range_remaining(a_time_range *range, a_time *t);

#endif /* __time_range_h__ */
