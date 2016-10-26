#ifndef __os_h__
#define __os_h__

#if _WIN32
  #pragma section(".CRT$XCU", read)
#define PRE_INIT(f)                                                   \
  static void f(void);                                                \
  __declspec(allocate(".CRT$XCU")) void (*f##_preinit)(void) = f;     \
  static void f(void)
#else
#define PRE_INIT(f)                          \
  static void  __attribute__((constructor)) f(void)
#endif

#if _WIN32
#define LOCALTIME_R(time, tm) localtime_s(tm, time)
#else
#define LOCALTIME_R(time, tm) localtime_r(time, tm)
#endif

#endif /* __os_h__ */
