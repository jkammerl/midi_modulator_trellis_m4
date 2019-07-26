#include "utils.h"

unsigned short Random::GetInt() {
  bit_ = ((lfsr_ >> 0) ^ (lfsr_ >> 2) ^ (lfsr_ >> 3) ^ (lfsr_ >> 5)) & 1;
  return lfsr_ = (lfsr_ >> 1) | (bit_ << 15);
}

float Random::GetFloat() { return static_cast<float>(GetInt()) / 0xFFFF; }

unsigned short Random::lfsr_ = 0xACE1u;
unsigned Random::bit_;
