#ifndef MIDI_H_
#define MIDI_H_

#include "state.h"
#include "utils.h"
#include <Adafruit_NeoTrellisM4.h>

class MidiOut {
  public:
    MidiOut(Adafruit_NeoTrellisM4 *trellis, State *state);
    ~MidiOut();

    void init();
    void run();

  private:
    Adafruit_NeoTrellisM4 *const trellis_;
    State *const state_;
};

#endif // MIDI_H_
