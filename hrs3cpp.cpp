#ifndef __hrs3_cpp__
#define __hrs3_cpp__

#include "hrs3cpp.h"
#include "hrs3.h"
typedef int status;
#include "impl/test.h"
#include "impl/time.h"
#include "impl/os.h"
#ifndef RUN_TESTS
#define RUN_TESTS 1
#endif
#ifndef while_0
#define while_0 while ((void)0,0)
#endif
#include <iostream>

class AggTime {
public:
  AggTime() : _timeIn(0), _timeOut(0) { }
  void timeIn(int x) { cout << "+" << x << endl;  _timeIn += x; }
  void timeOut(int x) { cout << "-" << x << endl; _timeOut += x; }
  int timeIn() const { return _timeIn; }
  int timeOut() const { return _timeOut; }
private:
  int _timeIn;
  int _timeOut;
};

int Hrs3::remainingIn(time_t t) const
{
  return _inverted
    ? hrs3_remaining_out(_hrsss.c_str(), t)
    : hrs3_remaining_in(_hrsss.c_str(), t);
}

int Hrs3::remainingOut(time_t t) const
{
  return _inverted
    ? hrs3_remaining_in(_hrsss.c_str(), t)
    : hrs3_remaining_out(_hrsss.c_str(), t);
}

AggTime Hrs3::aggTime(time_t begin, int sand) const
{
  AggTime aggTime;
  time_t it = begin;
  time_t end = begin + sand;
  int seconds = 0;
  for (; 0 < sand; it += seconds, sand -= seconds) {
    seconds = remainingIn(it);
    if (0 < seconds) {
      if (sand < seconds) seconds = sand;
      aggTime.timeIn(seconds);
    } else {
      seconds = remainingOut(it);
      if (sand < seconds) seconds = sand;
      if (0 == seconds) seconds = sand;
      aggTime.timeOut(seconds);
    }
  }
  if (it != end) throw runtime_error("aggTime bug");
  return aggTime;
}

bool Hrs3::validate()
{
  return 0 <= hrs3_remaining_out(_hrsss.c_str(), 1473482239);
}

#if RUN_TESTS
void test_hrs3_remainingIn()
{
  struct tm ymdhms;
  time_t t = time(0);
  LOCALTIME_R(&t, &ymdhms);
#define X(ret, x, h, m, s) do {                                        \
    ymdhms.tm_hour = h;                                                \
    ymdhms.tm_min = m;                                                 \
    ymdhms.tm_sec = s;                                                 \
    t = mktime(&ymdhms);                                               \
    Hrs3 hrs3(x);                                                      \
    if (ret != hrs3.remainingIn(t))                                    \
      TFAILF("%d vs %d", ret, hrs3.remainingIn(t));                    \
  } while_0
  X(   1, "8-9",  8, 59, 59);
  X(   0, "9-10", 8, 59, 59);
  X(   53941, "MTWRFAU0-2359",  8, 59, 59);
#undef X
}

ostream& operator <<(ostream &out, const AggTime &aggTime)
{
  out << "timeIn : " << aggTime.timeIn() << endl;
  out << "timeOut: " << aggTime.timeOut() << endl;
  return out;
}

void test_hrs3_remainingInWithInversion()
{
  struct tm ymdhms;
  time_t t = time(0);
  LOCALTIME_R(&t, &ymdhms);
#define X(ret, x, h, m, s) do {                                        \
    ymdhms.tm_hour = h;                                                \
    ymdhms.tm_min = m;                                                 \
    ymdhms.tm_sec = s;                                                 \
    t = mktime(&ymdhms);                                               \
    Hrs3 hrs3(x);                                                      \
    hrs3.invert();                                                     \
    if (ret != hrs3.remainingIn(t))                                    \
      TFAILF("%d vs %d", ret, hrs3.remainingIn(t));                    \
  } while_0
  X(   0, "8-9",  8, 59, 59);
  X(   1, "9-10", 8, 59, 59);
  X(23 * 3600, "8-9",  9,  0,  0);
#undef X
}

static struct TestHrs3 {
  TestHrs3() {
    test_hrs3_remainingIn();
    test_hrs3_remainingInWithInversion();
    Hrs3 hrs3("UMTWRFA0-2359");
    cout << hrs3.aggTime(time(0), 3600 * 24 * 7);
    cout << hrs3.aggTime(1473577140, 3600);
    cout << hrs3.remainingOut(1473577140) << endl;
  }
} x;
#endif /* RUN_TESTS */

/*
 * Local Variables:
 * compile-command: "gcc -DTEST -g -c hrs3.c && g++ -Wall -DTEST -g -o hrs3cpp hrs3cpp.cpp hrs3.o && ./hrs3cpp"
 * End:
 */

#endif /* __hrs3_cpp__ */
