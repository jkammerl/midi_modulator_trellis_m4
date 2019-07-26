#ifndef MODULATOR_H_
#define MODULATOR_H_

#include "state.h"
#include "utils.h"
#include <Adafruit_NeoTrellisM4.h>

class Modulator {
public:
  Modulator(Adafruit_NeoTrellisM4 *trellis, State *state);
  ~Modulator() {}

  void init();
  void run(unsigned long time_diff);

private:
  void UpdateController(Controller *controller, unsigned long time_diff);
  float GetSpeed(Controller::Speed speed);
  float GetRandomWalkSpeed(Controller::Speed speed);

  Adafruit_NeoTrellisM4 *const trellis_;
  State *const state_;

  Random random_;
};

#endif // MODULATOR_H_
