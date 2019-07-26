#include "midiout.h"

struct MidiMapping {
  bool bit14;
  int MSBC;
  int LSBC;
};

static const MidiMapping kMidiMapping[] = {
    {true, 0, 32},  {false, 0, 14}, {false, 0, 15}, {false, 0, 16},
    {false, 0, 17}, {false, 0, 18}, {false, 0, 19}, {false, 0, 20},
    {false, 0, 21}, {false, 0, 22}, {false, 0, 23}, {false, 0, 24},
    {false, 0, 25}, {false, 0, 26}, {false, 0, 27}, {false, 0, 28},
    {true, 29, 61}, {false, 0, 30}, {true, 31, 63}, {false, 0, 46},
    {false, 0, 47}, {false, 0, 48}, {false, 0, 49}, {false, 0, 50},
    {false, 0, 51}, {false, 0, 52}, {false, 0, 53}, {false, 0, 54},
    {false, 0, 55}};

MidiOut::MidiOut(Adafruit_NeoTrellisM4 *trellis, State *state)
    : trellis_(trellis), state_(state) {}
MidiOut::~MidiOut() {}

void MidiOut::init() {
  trellis_->enableUSBMIDI(true);
  trellis_->enableUARTMIDI(true);
}

void MidiOut::run() {
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    trellis_->setUSBMIDIchannel(c);
    trellis_->setUARTMIDIchannel(c);

    for (int i = 0; i < NUM_CONTROLLERS; ++i) {
      Controller *const controller = &state_->channels[c].controller[i];
      if (!controller->active) {
        continue;
      }
      const uint16_t range =
          controller->accel_state * (kMidiMapping[i].bit14 ? 0x3FFF : 0x7F);
      if (controller->last_midi_out == range) {
        continue;
      }
      if (!kMidiMapping[i].bit14) {
        trellis_->controlChange(kMidiMapping[i].LSBC, range);
      } else {
        trellis_->controlChange(kMidiMapping[i].MSBC, range >> 7);
        trellis_->controlChange(kMidiMapping[i].LSBC, range & 0x7F);
      }
    }
  }
}
