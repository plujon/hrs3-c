#ifndef __base_h__
#define __base_h__

#define DIM(x) sizeof(x)/sizeof(x[0])
#define CRASH() do { char *p = 0; *p = 'a'; } while(0)
#define NOD(x) do { status _x = x; if (OK != _x) return _x; } while (0)
#if TEST
#  ifndef CHECK
#  define CHECK 1
#  endif
#  define RUN_TESTS 1
#endif
#if CHECK
#  define BUG() CRASH()
#else
#  define BUG()
#endif
#define ONE_OBJ 1
#define OK 0
#define true 1
#define false 0

typedef int status;
typedef int bool;

struct a_day;
struct a_schedule;
struct a_time;
struct a_time_range;

#include <stddef.h>

#include "os.h"

#endif /* __base_h__ */
