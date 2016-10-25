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

a_military_time military_midnight(void);
int military_time_cmp(const a_military_time *a, const a_military_time *b);
int military_time_diff(const a_military_time *later, const a_military_time *prior);
size_t military_time_to_s(const a_military_time *time, char *buffer);
bool military_time_to_time(const a_military_time *military_time, const struct a_time *date, struct a_time *t);
bool military_range_to_time_range(const a_military_range *military_range, const struct a_time *date, struct a_time_range *time_range);
status military_parse_time(a_military_time *time, const char *s, size_t len);
int military_range_cmp(const a_military_range *a, const a_military_range *b);
int military_range_in_seconds(const a_military_range *range);
size_t military_range_to_s(const a_military_range *range, char *buffer);
bool military_range_contains(const a_military_range *range, const a_military_time *time);
bool military_range_overlaps_or_abuts(const a_military_range *a, const a_military_range *b);
void military_range_merge(a_military_range *dest, const a_military_range *src);
status military_parse_range(a_military_range* range, const char *s, size_t len);

#endif /* __military_h__ */
