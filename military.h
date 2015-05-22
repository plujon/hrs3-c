#ifndef __military_h__
#define __military_h__

typedef struct a_military_time {
  unsigned char hour;   /* 0 through 24 */
  unsigned char minute; /* 0 through 59 */
} a_military_time;

typedef struct a_military_range {
  a_military_time start;
  a_military_time stop;
} a_military_range;

a_military_time military_midnight();
int military_time_cmp(a_military_time *a, a_military_time *b);
int military_time_diff(a_military_time *later, a_military_time *prior);
size_t military_time_to_s(char *buffer, a_military_time *time);
status military_parse_time(const char *s, int len, a_military_time* time);
int military_range_cmp(a_military_range *a, a_military_range *b);
int military_range_in_seconds(a_military_range *range);
size_t military_range_to_s(char *buffer, a_military_range *range);
bool military_range_contains(a_military_range *range, a_military_time *time);
bool military_range_overlaps_or_abuts(a_military_range *a, a_military_range *b);
void military_range_merge(a_military_range *dest, a_military_range *src);
status military_parse_range(const char *s, int len, a_military_range* range);

#endif /* __military_h__ */
