#ifndef MIDI_IN_H
#define MIDI_IN_H

#include <stdint.h>

#include "flash.h"

class MidiIn {
  public:
    MidiIn(FlashFs* flash_fs);
    void init();
    void process();
    long num_start_clocks();
    long num_clock_ticks();
  private:
  	long start_clocks_;
  	long clock_ticks_;
    FlashFs* flash_fs_;
};

#endif // MIDI_IN_H
