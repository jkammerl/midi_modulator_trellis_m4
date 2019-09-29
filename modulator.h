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
    void run(double time_qp);

  private:
    void UpdateController(Controller *controller, double prev_time_qp, double time_qp);
    float GetSpeed(Controller::Speed speed);
    float GetRandomWalkSpeed(Controller::Speed speed);
    void EtcDoProgramSwitch(double time_qp);   

    Adafruit_NeoTrellisM4 *const trellis_;
    State *const state_;

    double prev_time_qp_;

    Random random_;
};

#endif // MODULATOR_H_
