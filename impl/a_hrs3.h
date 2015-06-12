#ifndef __a_hrs3_h__
#define __a_hrs3_h__

#include "daily.h"
#include "remaining.h"
#include "time_range.h"
#include "weekly.h"

/*
 * hrs3 - An intermediate form for parsed hrs3 strings.
 */

typedef enum a_hrs3_kind {
  Unknown,
  Invalid,
  Daily,
  Weekdaily,
  Weekly,
  Biweekly,
  Raw
} a_hrs3_kind;

typedef struct a_hrs3 {
  a_hrs3_kind kind;
  union {
    a_day day;
    a_week week;
    a_time_range time_range;
  };
} a_hrs3;

status hrs3_init(a_hrs3 *hrs3, const char *buffer, size_t len);
a_remaining_result hrs3_remaining(a_hrs3 *hrs3, const a_time *t);
void hrs3_destroy(a_hrs3 *);

#endif /* __a_hrs3_h__ */
