#include "midiout.h"
#include "state.h"

struct MidiMapping {
  bool bit14;
  int MSBC;
  int LSBC;
};

#ifdef NORDDRUM2
#define MIN_MIDI_DELAY 1
// Nord Drum 2
static const MidiMapping kMidiMapping[] = {
  {false, 0, 14}, // 1.1 Noise Filter Frequency
  {false, 0, 15}, // 1.2 Noise Filter Type
  {false, 0, 16}, // 1.3 Noise Filter Envelope
  {false, 0, 17}, // 1.4 Noise Filter Resonance
  {false, 0, 18}, // 1.5 Noise Attack/Rate
  {false, 0, 19}, // 1.6 Noise Atk Mode
  {false, 0, 20}, // 1.7 Noise Decay Type (E, L, G)
  {false, 0, 21}, // 2.1 Noise Decay
  {false, 0, 22}, // 2.2 Noise Decay Lo
  {false, 0, 23}, // 2.3 Dist Amount
  {false, 0, 24}, // 2.4 Dist Type
  {false, 0, 25}, // 2.5 EQ Frequency
  {false, 0, 26}, // 2.6 EQ Gain
  {false, 0, 27}, // 2.7 Echo Feedback
  {false, 0, 28}, // 3.1 Echo Amount
  {true, 29, 61}, // 3.2 Echo BPM MSB
  {false, 0, 30}, // 3.3 Tone Spectra
  {true, 31, 63}, // 3.4 Tone Pitch
  {false, 0, 46}, // 3.5 Tone Wave
  {false, 0, 47}, // 3.6 Tone Timbre Decay
  {false, 0, 48}, // 3.7 Tone Punch
  {false, 0, 49}, // 4.1 Tone Decay Type (L, E)
  {false, 0, 50}, // 4.2 Tone Decay
  {false, 0, 51}, // 4.3 Tone Dec Lo
  {false, 0, 52}, // 4.4 Tone Timbre
  {false, 0, 53}, // 4.5 Tone Timb Envelope
  {false, 0, 54}, // 4.6 Tone Bend Amount
  {false, 0, 55} // 4.7 Tone Bend Time
};

#endif

#ifdef MFB_TANZBAER
#define MIN_MIDI_DELAY 100
// MFB-Tanzbaer
static const MidiMapping kMidiMapping[] = {
  {false, 0, 2},
  {false, 0, 64},
  {false, 0, 65},
  {false, 0, 3},
  {false, 0, 4},
  {false, 0, 5},
  {false, 0, 6},
  {false, 0, 66},
  {false, 0, 8},
  {false, 0, 9},
  {false, 0, 10},
  {false, 0, 11},
  {false, 0, 12},
  {false, 0, 13},
  {false, 0, 67},
  {false, 0, 14},
  {false, 0, 68},
  {false, 0, 69},
  {false, 0, 88},
  {false, 0, 70},
  {false, 0, 15},
  {false, 0, 71},
  {false, 0, 72},
  {false, 0, 73},
  {false, 0, 74},
  {false, 0, 16},
  {false, 0, 17},
  {false, 0, 75},
  {false, 0, 18},
  {false, 0, 76},
  {false, 0, 77},
  {false, 0, 19},
  {false, 0, 20},
  {false, 0, 78},
  {false, 0, 79},
  {false, 0, 21},
  {false, 0, 22},
  {false, 0, 80},
  {false, 0, 81},
  {false, 0, 23},
  {false, 0, 24},
  {false, 0, 82},
  {false, 0, 83},
  {false, 0, 84},
  {false, 0, 85},
  {false, 0, 86},
  {false, 0, 87},
  {false, 0, 85}
};
#endif

static const int kNumMidiMapping = sizeof(kMidiMapping) / sizeof(MidiMapping);

MidiOut::MidiOut(Adafruit_NeoTrellisM4 *trellis, State *state)
  : trellis_(trellis), state_(state) {}
MidiOut::~MidiOut() {}

void MidiOut::init() {
  trellis_->enableUSBMIDI(true);
  trellis_->enableUARTMIDI(true);
}



void MidiOut::run() {
  for (int c = 0; c < NUM_CHANNELS; ++c) {
#ifdef MFB_TANZBAER
    const int midi_channel = 9;
#else
    const int midi_channel = (c == 6) ? 9 : c;
    if (state_->channels[c].muted) {
      continue;
    }
#endif
    trellis_->setUSBMIDIchannel(midi_channel);
    trellis_->setUARTMIDIchannel(midi_channel);

    for (int i = 0; i < NUM_CONTROLLERS; ++i) {
#ifdef MFB_TANZBAER
      const int idx = i + (c * NUM_CONTROLLERS);
#else
      const int idx = i;
#endif
      if (idx >= kNumMidiMapping) {
        continue;
      }
      Controller *const controller = &state_->channels[c].controller[i];
      if (!controller->active) {
        continue;
      }
      const uint16_t range =
        controller->accel_state * (kMidiMapping[idx].bit14 ? 0x3FFF : 0x7F);
      if (controller->last_midi_out == range) {
        continue;
      }
      unsigned long now_time = millis();
      if (now_time - controller->last_midi_time < MIN_MIDI_DELAY) {
        continue;
      }
      controller->last_midi_time = now_time;

     if (!kMidiMapping[idx].bit14) {
        trellis_->controlChange(kMidiMapping[idx].LSBC, range);
      } else {
        trellis_->controlChange(kMidiMapping[idx].LSBC, range & 0x7F);
        trellis_->controlChange(kMidiMapping[idx].MSBC, range >> 7);
      }
#ifdef MFB_TANZBAER
      //      delay(1);
#endif
    }
  }
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    const int midi_channel = 0;
    trellis_->setUSBMIDIchannel(midi_channel);
    trellis_->setUARTMIDIchannel(midi_channel);  
    for (int i = 0; i < 5; ++i) {
      Controller *const controller = &state_->etc.controller[i];
      if (!controller->active) {
        continue;
      }
      const uint16_t range =
        controller->accel_state * 0x7F;
      if (controller->last_midi_out == range) {
        continue;
      }
      unsigned long now_time = millis();
      if (now_time - controller->last_midi_time < MIN_MIDI_DELAY) {
        continue;
      }
      controller->last_midi_time = now_time;
      trellis_->controlChange(21+i, range);
    }
  }
}
