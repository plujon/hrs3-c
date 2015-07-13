#ifndef __remaining_c__
#define __remaining_c__

#include "impl.h"

a_remaining_result remaining_result(int is_in_schedule, int seconds)
{
  a_remaining_result result = { 1, is_in_schedule, seconds };
  return result;
}

a_remaining_result remaining_invalid()
{
  a_remaining_result result = { 0, 0, 0 };
  return result;
}

#if ONE_OBJ
#include "main.c"
#endif

#endif /* __remaining_c__ */
