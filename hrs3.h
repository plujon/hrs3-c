#ifndef __hrs3_h__
#define __hrs3_h__

#include <time.h>
#include "extern_c.h"

EXTERN_C
int hrs3_remaining_in(const char *s, time_t time);
EXTERN_C
int hrs3_remaining_out(const char *s, time_t time);

#endif /* __hrs3_h__ */
