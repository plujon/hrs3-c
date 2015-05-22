#ifndef __test_h__
#define __test_h__

#include <stdio.h>
#include <stdlib.h>

#define TFAIL()                                         \
  do {                                                  \
    printf("failed at %s:%d\n", __FILE__, __LINE__);    \
    exit(1);                                            \
  } while(0)
#define TFAILF(fmt, ...)                                                \
  do {                                                                  \
    printf("failed at %s:%d" fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
    exit(1);                                                            \
  } while(0)

#endif /* __test_h__ */
