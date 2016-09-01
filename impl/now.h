#ifndef __now_h__
#define __now_h__

typedef struct a_now_range {
  int seconds;
  int days;
} a_now_range;

status now_init(a_now_range *now_range, const char *s, size_t len);
void now_destroy(a_now_range *now_range);
void now_to_time_range(const a_now_range *now_range, const a_time *time, struct a_time_range *range);
void now_add_to_schedule(const a_now_range *now_range, const a_time *time, struct a_schedule *schedule);

#endif /* __now_h__ */
