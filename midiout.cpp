#include "midiout.h"
#include "state.h"
#include "midimapping.h"

#define MIN_MIDI_DELAY 1

MidiOut::MidiOut(Adafruit_NeoTrellisM4 *trellis, State *state)
  : trellis_(trellis), state_(state) {}
MidiOut::~MidiOut() {}

void MidiOut::init() {
  trellis_->enableUSBMIDI(true);
  trellis_->enableUARTMIDI(true);
}

void MidiOut::run() {
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    const int midi_channel = c;
    if (state_->channels[c].muted) {
      continue;
    }
    trellis_->setUSBMIDIchannel(midi_channel);
    trellis_->setUARTMIDIchannel(midi_channel);
    const MidiMapping* midi_mapping = kMidiMapping;

    for (int i = 0; i < NUM_CONTROLLERS; ++i) {
      const int idx = i;
      if (idx >= kNumMidiMapping) {
        continue;
      }
      Controller *const controller = &state_->channels[c].controller[i];
      if (!controller->active) {
        continue;
      }
      const uint16_t range =
        controller->accel_state * (midi_mapping[idx].MSBC>0 ? 0x3FFF : 0x7F);
      if (controller->last_midi_out == range) {
        continue;
      }
      unsigned long now_time = millis();
      if (now_time - controller->last_midi_time < MIN_MIDI_DELAY) {
        continue;
      }
      controller->last_midi_time = now_time;

     if (midi_mapping[idx].MSBC==0) {
        trellis_->controlChange(midi_mapping[idx].LSBC, range);
      } else {
        trellis_->controlChange(midi_mapping[idx].LSBC, range & 0x7F);
        trellis_->controlChange(midi_mapping[idx].MSBC, range >> 7);
      }
    }
  }
  trellis_->sendMIDI();
}
