#include <Adafruit_NeoTrellisM4.h>

#include "controls.h"
#include "midiout.h"
#include "modulator.h"
#include "state.h"

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

State state;
Controls controls(&trellis, &state);
Modulator modulator(&trellis, &state);
MidiOut midiout(&trellis, &state);

unsigned long prevReadTime = 0;

bool animation() {
  static unsigned long t = millis();
  if (millis() - t > 1000) {
    return false;
  }
  trellis.tick();
  trellis.fill(0);
  trellis.setPixelColor(rand() % 32, rand() % 0xFFFFFF);
  trellis.show();
  return true;
}

void setup() {
  Serial.begin(115200);
  // while (!Serial);
  delay(100);

  controls.init();
  modulator.init();
  midiout.init();
}

const double kMilliTo120qp =  120.0 / 60000.0 ; //* 24.0;
double time_qp = 0.0;

unsigned long last_midi_clock_received = 0;

void loop() {
  if (animation()) {
    return;
  }

  unsigned long t = millis();
  unsigned long time_diff = t - prevReadTime;
  prevReadTime = t;

  trellis.processMIDIIn();
  int num_midi_clocks = trellis.midiClockTickReceived();
  bool midi_clock_blink = false;
  if (trellis.midiStartClockReceived()) {
    last_midi_clock_received = t;
    time_qp = 0.0;
  } else if (num_midi_clocks > 0) {
    last_midi_clock_received = t;
    const int qp_int = static_cast<int>(time_qp); 
    time_qp += static_cast<double>(num_midi_clocks) / 24.0;
    midi_clock_blink = qp_int!=static_cast<int>(time_qp); 
  } else if (t - last_midi_clock_received > 1000) {
    time_qp = static_cast<double>(t) * kMilliTo120qp;
  }

  controls.run(midi_clock_blink);
  modulator.run(time_qp);
  midiout.run();
  if (time_diff < 10) {
    delay(10 - time_diff);
  }
}
