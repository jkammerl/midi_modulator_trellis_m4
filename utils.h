#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdint.h>

class Random {
public:
  static unsigned short GetInt();
  static float GetFloat();

private:
  static unsigned short lfsr_;
  static unsigned bit_;
};

#endif // RANDOM_H_
