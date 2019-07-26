#ifndef STATE_H_
#define STATE_H_

#define NUM_CONTROLLERS (32 - 4)
#define NUM_CHANNELS 7

struct Controller {
  enum Mode { SINUS = 0, PULSE, RAND, WALK, NUM_MODES };

  enum Speed { SLOW = 0, MED, FAST, ULTRA, NUM_SPEEDS };

  bool active;
  bool speed_visualization;
  Mode mode;
  Speed speed;
  float state;
  float phase;
  float accel_state;
  uint16_t last_midi_out;
};

struct Channel {
  Controller controller[NUM_CONTROLLERS];
};

struct State {
  enum OpMode { MUTE = 0, MODULATION, SPEED, CHANNEL };
  OpMode op_mode;

  Channel channels[NUM_CHANNELS];
};

#endif // STATE_H_
