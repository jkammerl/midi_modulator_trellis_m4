/*
   controls.cpp

    Created on: Oct 22, 2018
        Author: dean
*/

#include "modulator.h"
#include <Adafruit_NeoTrellisM4.h>

#define FILTER_Q 0.1f

Modulator::Modulator(Adafruit_NeoTrellisM4 *trellis, State *state)
  : trellis_(trellis), state_(state) {}

void Modulator::init() {
  prev_time_qp_ = 0.0;
}

float Modulator::GetSpeed(Controller::Speed speed) {
  switch (speed) {
    case Controller::Speed::SLOW:
      return 0.25f;
      break;
    case Controller::Speed::MED:
      return 0.5f;
      break;
    case Controller::Speed::FAST:
      return 1.0f;
      break;
    case Controller::Speed::ULTRA:
      return 2.0f;
      break;
    default:
      break;
  }
}

float Modulator::GetRandomWalkSpeed(Controller::Speed speed) {
  switch (speed) {
    case Controller::Speed::SLOW:
      return 0.001f;
      break;
    case Controller::Speed::MED:
      return 0.005f;
      break;
    case Controller::Speed::FAST:
      return 0.02f;
      break;
    case Controller::Speed::ULTRA:
      return 0.08f;
      break;
    default:
      break;
  }
}

void Modulator::UpdateController(Controller *controller, double prev_time_qp, double time_qp) {
  const float speed_hz = GetSpeed(controller->speed);
  const double prev_time_qp_bar = fmod(prev_time_qp, 4.0);
  const double time_qp_bar = fmod(time_qp, 4.0);
  const bool trigger =
    time_qp_bar < prev_time_qp_bar;
  const float rand_float = random_.GetFloat();

  const float sin_mod = (sinf((time_qp / 4.0) * M_PI * 2.0f * speed_hz) + 1.0f) /
                        2.0f;

  controller->speed_visualization = sin_mod;

  switch (controller->mode) {
    case Controller::Mode::SINUS:
      controller->state = sin_mod;
      break;
    case Controller::Mode::PULSE:
      controller->state = fmod(time_qp, 4.0 / speed_hz) / (4.0 / speed_hz);
      break;
    case Controller::Mode::WALK:
      controller->state += (rand_float > 0.5f ? 1.0 : -1.0) *
                           GetRandomWalkSpeed(controller->speed);
      if (controller->state > 1.0) {
        controller->state = 1.0f - (controller->state - 1.0f);
      } else if (controller->state < 0.0) {
        controller->state *= -1.0f;
      }
      break;
    default:
      break;
  }
}

void Modulator::run(double time_qp) {
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    for (int i = 0; i < NUM_CONTROLLERS; ++i) {
      Controller *const controller = &state_->channels[c].controller[i];
      UpdateController(controller, prev_time_qp_, time_qp);
    }
  }
  prev_time_qp_ = time_qp;
}
