# hrs3-c
C library for hrs3 (human readable small schedule string) schedules

## What is hrs3?

hrs3 is a notation for describing periods of time using short strings.
Here are some examples:

* 10-12: 10am to 12pm
* MWF10-12: Monday, Wednesday, and Friday, 10am to 12pm
* M1330-1400: Monday 13:00 to 14:00 (1:30pm to 2:00pm)

Daily, week-daily, weekly, biweekly, and raw are all part of hrs3.

See hrs3.bnf or hrs3.ebnf for official grammar.

This library does not implement biweekly schedules.

## How to use

A time is said to be "in" or "out" of a schedule, depending on whether
or not it occurs during the relevant schedule.

    time_t now = time(0);
    int seconds = hrs3_remaining_in("MWF10-12", now);
    if (seconds) {
      /* The time 'now' through 'now + seconds' falls within MWF10-12. */
    } else {
      /* The time 'now' does not fall within MWF10-12. */
    }
    seconds = hrs3_remaining_out("MWF10-12", now);
    if (seconds) {
      /* The time 'now' is not in MWF10-12, and 'now + seconds' is when the next scheduled period starts. */
    } else {
      /* The time 'now' falls within MWF10-12. */
    }

## Canonical representation

Every hrs3 string can be converted to a canonical representation with
the same meaning.  A canonical representation:

1. has no overlapping or abutting shifts, except shifts that abut at
   midnight.  So, "10-12&13-14" and "M23-24&T0-1" are canonical, and
   "10-12&11-13" and "10-12&12-13" are not.

2. is as chronologically ordered as possible.  When grouping
   designators are used, such as "MWF", the earliest chronological
   designator shall control the relative ordering with other similar
   groupings in the same hrs3.  So, "MWF10-12.T8-9" is canonical, and
   "T8-9.MWF10-12" is not.

3. does not contain repetitions of the same day designator.  So,
   "M10-12&13-15.WF10-12" is canonical, and "MWF10-12.M13-15" is not.

4. uses the shortest means to express the schedule while observing the
   above restrictions. The shortest equivalent unambiguous string is
   canonical.  When two or more alternative representations have the
   same length, the representation with fewest highens shall be
   canonical.  So, "10-12" is canonical and "1000-1200" is not.

Canonical representations should be used whenever possible because
they can be used for equality comparison, and many implementations
convert to canonical representation internally.
