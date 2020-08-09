#ifndef MIDI_IN_H
#define MIDI_IN_H

#include <stdint.h>
#include <Adafruit_NeoTrellisM4.h>

#include "flash.h"

class MidiIn {
  public:
    MidiIn(Adafruit_NeoTrellisM4 *trellis, FlashFs* flash_fs);
    void init();
    void process();
    long num_start_clocks();
    long num_clock_ticks();
  private:
  	long start_clocks_;
  	long clock_ticks_;
    Adafruit_NeoTrellisM4 *const trellis_;
    FlashFs* flash_fs_;
};

#endif // MIDI_IN_H
