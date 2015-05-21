#ifndef __remaining_c__
#define __remaining_c__

#include "remaining.h"

a_remaining_result remaining_result(int is_in_schedule, int seconds)
{
  a_remaining_result result = { 1, is_in_schedule, seconds };
  return result;
}

a_remaining_result remaining_invalid()
{
  a_remaining_result result = { 0, -1, -1 };
  return result;
}

#endif /* __remaining_c__ */
