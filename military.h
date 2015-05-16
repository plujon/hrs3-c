#ifndef __military_h__
#define __military_h__

/*
 *
 */

typedef struct a_military_time {
  unsigned char hour;   /* 0 through 24 */
  unsigned char minute; /* 0 through 59 */
} a_military_time;

typedef struct a_military_range {
  a_military_time start;
  a_military_time stop;
} a_military_range;

int military_parse_range(const char *s, int len, a_military_range* range);
int military_parse_time(const char *s, int len, a_military_time* time);
int military_range_cmp(a_military_range *a, a_military_range *b);
int military_range_in_seconds(a_military_range *range);
int military_range_in_seconds(a_military_range *range);
int military_time_cmp(a_military_time *a, a_military_time *b);
int military_time_diff(a_military_time *later, a_military_time *prior);

#endif /* __military_h__ */
