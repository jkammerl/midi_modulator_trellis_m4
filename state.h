#ifndef STATE_H_
#define STATE_H_

#include "stdint.h"

#define NUM_CONTROLLERS (32 - 4)
#define NUM_CHANNELS 7
#define NORDDRUM2
//#define MFB_TANZBAER

struct Controller {
  enum Mode { SINUS = 0, PULSE, WALK, NUM_MODES };

  enum Speed {  SLOW = 0, MED, FAST, ULTRA, NUM_SPEEDS };

  bool active;
  float speed_visualization;
  Mode mode;
  Speed speed;
  float state;
  float accel_state;
  uint16_t last_midi_out;
  unsigned long last_midi_time;
};

struct Channel {
  Controller controller[NUM_CONTROLLERS];
  bool muted;
};

struct State {
  enum OpMode { MUTE = 0, MODULATION, SPEED, CHANNEL };
  OpMode op_mode;

  Channel channels[NUM_CHANNELS];
};


#endif // STATE_H_
