#include "midimapping.h"

// Default mapping for ND2.
const MidiMapping kMidiND2Mapping[] = {
  {0, 14}, // 1.1 Noise Filter Frequency
  {0, 15}, // 1.2 Noise Filter Type
  {0, 16}, // 1.3 Noise Filter Envelope
  {0, 17}, // 1.4 Noise Filter Resonance
  {0, 18}, // 1.5 Noise Attack/Rate
  {0, 19}, // 1.6 Noise Atk Mode
  {0, 20}, // 1.7 Noise Decay Type (E, L, G)
  {0, 21}, // 2.1 Noise Decay
  {0, 22}, // 2.2 Noise Decay Lo
  {0, 23}, // 2.3 Dist Amount
  {0, 24}, // 2.4 Dist Type
  {0, 25}, // 2.5 EQ Frequency
  {0, 26}, // 2.6 EQ Gain
  {0, 27}, // 2.7 Echo Feedback
  {0, 28}, // 3.1 Echo Amount
  {29, 61}, // 3.2 Echo BPM MSB
  {0, 30}, // 3.3 Tone Spectra
  {31, 63}, // 3.4 Tone Pitch
  {0, 46}, // 3.5 Tone Wave
  {0, 47}, // 3.6 Tone Timbre Decay
  {0, 48}, // 3.7 Tone Punch
  {0, 49}, // 4.1 Tone Decay Type (L, E)
  {0, 50}, // 4.2 Tone Decay
  {0, 51}, // 4.3 Tone Dec Lo
  {0, 52}, // 4.4 Tone Timbre
  {0, 53}, // 4.5 Tone Timb Envelope
  {0, 54}, // 4.6 Tone Bend Amount
  {0, 55} // 4.7 Tone Bend Time
};

const int kNumMidiMapping = sizeof(kMidiND2Mapping) / sizeof(MidiMapping);

MidiMapping kMidiMapping[kNumMidiMapping];

void InitMidiMapping() {
   memcpy(kMidiMapping, kMidiND2Mapping, sizeof(kMidiND2Mapping));
}
