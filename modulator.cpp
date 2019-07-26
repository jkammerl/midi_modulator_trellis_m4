/*
 * controls.cpp
 *
 *  Created on: Oct 22, 2018
 *      Author: dean
 */

#include "modulator.h"
#include <Adafruit_NeoTrellisM4.h>

#define FILTER_Q 0.1f

Modulator::Modulator(Adafruit_NeoTrellisM4 *trellis, State *state)
    : trellis_(trellis), state_(state) {}

void Modulator::init() {}

float Modulator::GetSpeed(Controller::Speed speed) {
  switch (speed) {
  case Controller::Speed::SLOW:
    return 0.2f;
    break;
  case Controller::Speed::MED:
    return 0.5f;
    break;
  case Controller::Speed::FAST:
    return 2.0f;
    break;
  case Controller::Speed::ULTRA:
    return 8.0f;
    break;
  default:
    break;
  }
}

float Modulator::GetRandomWalkSpeed(Controller::Speed speed) {
  switch (speed) {
  case Controller::Speed::SLOW:
    return 1.0f;
    break;
  case Controller::Speed::MED:
    return 2.0f;
    break;
  case Controller::Speed::FAST:
    return 4.0f;
    break;
  case Controller::Speed::ULTRA:
    return 8.0f;
    break;
  default:
    break;
  }
}

void Modulator::UpdateController(Controller *controller,
                                 unsigned long time_diff) {
  const float speed_hz = GetSpeed(controller->speed);
  const float interval_ms = 1000.0f / speed_hz;
  const float half_interval_ms = interval_ms / 2.0f;
  const float prev_phase = fmod(controller->phase, interval_ms);
  controller->phase = fmod(prev_phase + time_diff, interval_ms);
  const bool trigger =
      prev_phase < half_interval_ms && controller->phase >= half_interval_ms;
  const float rand_float = random_.GetFloat();

  controller->speed_visualization =
      controller->phase > half_interval_ms ? 1.0f : 0.0f;

  switch (controller->mode) {
  case Controller::Mode::SINUS:
    controller->state =
        (sinf((controller->phase / 1000.0f) * M_PI * 2.0f * speed_hz) + 1.0f) /
        2.0f;
    break;
  case Controller::Mode::PULSE:
    controller->state = controller->phase > half_interval_ms ? 1.0f : 0.0f;
    break;
  case Controller::Mode::RAND:
    if (trigger) {
      controller->state = rand_float;
    }
    break;
  case Controller::Mode::WALK:
    controller->state += (rand_float > 0.5f ? 0.03 : -0.03) *
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

void Modulator::run(unsigned long time_diff) {
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    for (int i = 0; i < NUM_CONTROLLERS; ++i) {
      Controller *const controller = &state_->channels[c].controller[i];
      UpdateController(controller, time_diff);
    }
  }
}
