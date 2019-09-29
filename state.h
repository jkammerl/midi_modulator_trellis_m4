#ifndef STATE_H_
#define STATE_H_

#include "stdint.h"

#define NUM_CONTROLLERS (32 - 4)
#define NUM_CHANNELS 7
#define ETC_CHANNEL_IDX 6
#define NORDDRUM2
//#define MFB_TANZBAER

struct Controller {
  enum Mode { SINUS = 0, PULSE, RAND, WALK, NUM_MODES };

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

struct ETC {
  bool enabled_program[2*NUM_CONTROLLERS];
  Controller controller[5];
  enum SwitchMode { OFF = 0, FORWARD, RAND_SWITCH };
  SwitchMode switch_mode;
  enum SwitchDuration { SLOW = 0, MED, FAST };
  SwitchDuration switch_duration;
  double switch_duration_val;
  double switch_duration_val_applied;
  bool rand_duration;
  int current_program;
};

struct State {
  enum OpMode { MUTE = 0, MODULATION, SPEED, CHANNEL };
  OpMode op_mode;

  Channel channels[NUM_CHANNELS];
  ETC etc;
};

#endif // STATE_H_
