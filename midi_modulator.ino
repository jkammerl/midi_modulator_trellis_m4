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

void setup() {
	Serial.begin(115200);
	// while (!Serial);
	delay(100);

	controls.init();
	modulator.init();
	midiout.init();
}

void loop() {
	unsigned long t = millis();
	unsigned long time_diff = t - prevReadTime;
	prevReadTime = t;

	controls.run();
	modulator.run(time_diff);
	midiout.run();
	delay(50);
}
