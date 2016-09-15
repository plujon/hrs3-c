#ifndef __hrs3cpp_h__
#define __hrs3cpp_h__

#include <string>
#include <stdexcept>
using namespace std;

class AggTime;

class Hrs3 {
public:
  static Hrs3 nullHrs3() {
    return Hrs3("");
  }
  Hrs3(string hrsss = "") : _hrsss(hrsss), _inverted(false) {
    if (!validate())
      _hrsss = "";
  }
  void invert() { _inverted = !_inverted; }
  bool inverted() const { return _inverted; }
  bool empty() const { return _hrsss.empty(); }
  bool valid() const { return !empty(); }
  string str() const { return _hrsss; }
  bool operator ==(const Hrs3 &other) const { return _hrsss == other._hrsss; }
  int remainingIn(time_t t) const;
  int remainingOut(time_t t) const;
  AggTime aggTime(time_t begin, int sand) const;
private:
  bool validate();
  string _hrsss;
  bool _inverted;
};

#endif // __hrs3cpp_h__
