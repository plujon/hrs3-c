#ifndef __weekly_h__
#define __weekly_h__

#include "daily.h"
#include "remaining.h"
#include <time.h>

typedef struct a_week {
  a_day days[7];
} a_week;

status week_parse(const char *s, size_t size, a_week *week);
void week_destroy(a_week *week);
void week_add_to_schedule(const a_week *week, const a_time *t, struct a_schedule *schedule);

#endif /* __weekly_h__ */
