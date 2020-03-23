#ifndef MIDI_MAPPING_H
#define MIDI_MAPPING_H

#include "state.h"
#include <string.h>

struct MidiMapping {
  uint8_t MSBC;
  uint8_t LSBC;
};

// Default mapping for ND2.
extern const MidiMapping kMidiND2Mapping[];

extern const int kNumMidiMapping;

extern MidiMapping kMidiMapping[];

void InitMidiMapping();



#endif // MIDI_MAPPING_H
