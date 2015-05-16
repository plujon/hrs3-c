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
