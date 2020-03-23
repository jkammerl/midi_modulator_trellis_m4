#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

#define STATE_START_BLOCK (0)
#define MMAP_START_BLOCK (1025*64)

class FlashFs {
  public:
    void init();
    bool ReadState(char* ptr, long len, int byteoffset);
    void WriteState(const char* ptr, long len, int byteoffset);
};

#endif // FLASH_H
