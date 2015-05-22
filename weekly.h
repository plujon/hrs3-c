#ifndef __weekly_h__
#define __weekly_h__

#include "daily.h"
#include "remaining.h"
#include <time.h>

typedef struct a_week {
  a_day days[7];
} a_week;

a_remaining_result weekly_remaining(const char *s, a_time *t);

#endif /* __weekly_h__ */
