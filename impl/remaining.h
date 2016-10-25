#ifndef __remaining_h__
#define __remaining_h__

typedef struct a_remaining_result {
  unsigned int is_valid:1;               /* whether this result is valid */
  unsigned int time_is_in_schedule:1;    /* whether time is in the schedule */
  /* if time_is_in_schedule, then this is the number of seconds after
     time that are still known to be in the schedule. */
  unsigned int seconds:30;
} a_remaining_result;

a_remaining_result remaining_result(int is_in_schedule, int seconds);
a_remaining_result remaining_invalid(void);

#endif /* __remaining_h__ */
