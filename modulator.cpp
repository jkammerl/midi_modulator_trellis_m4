/*
   controls.cpp

    Created on: Oct 22, 2018
        Author: dean
*/

#include "modulator.h"
#include <Adafruit_NeoTrellisM4.h>

#undef min
#undef max
#include <vector>

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
  const double prev_time_qp_bar = fmod(prev_time_qp, 4.0 / speed_hz);
  const double time_qp_bar = fmod(time_qp, 4.0 / speed_hz);
  const bool trigger =
    time_qp_bar < prev_time_qp_bar;
  const float rand_float = random_.GetFloat();

  const float sin_mod = (sinf((time_qp / 4.0) * M_PI * 2.0f * speed_hz + M_PI/2.0) + 1.0f) /
                        2.0f;

  controller->speed_visualization = sin_mod;

  switch (controller->mode) {
    case Controller::Mode::SINUS:
      controller->state = sin_mod;
      break;
    case Controller::Mode::PULSE:
      controller->state = fmod(time_qp, 4.0 / speed_hz) / (4.0 / speed_hz);
      break;
    case Controller::Mode::RAND:
      if (trigger) {
         controller->state = rand_float;
      }
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

void Modulator::EtcDoProgramSwitch(double time_qp) {
  static double last_time_qp = 0.0;
  double time_mod = fmod(time_qp / 4.0, state_->etc.switch_duration_val_applied);
  double last_time_mod = fmod(last_time_qp / 4.0, state_->etc.switch_duration_val_applied);
  last_time_qp = time_qp;
  if (last_time_mod<time_mod) {
    return;
  }

  if (state_->etc.switch_mode==ETC::SwitchMode::OFF) {
    return;
  }

  std::vector<int> available_programs;
  available_programs.reserve(2*NUM_CONTROLLERS);
  for (int i=0; i<2*NUM_CONTROLLERS; ++i) {
    int idx = (i+state_->etc.current_program) % (2*NUM_CONTROLLERS);
    if (state_->etc.enabled_program[idx]) {
      available_programs.push_back(idx);
    }
  }

  switch (state_->etc.switch_mode) {
    case ETC::SwitchMode::FORWARD:
        if (available_programs.size() > 0) 
           state_->etc.current_program = available_programs[1 % available_programs.size()];
        break;
    case ETC::SwitchMode::RAND_SWITCH:
        state_->etc.current_program = random_.GetInt() % 127;
        break;        
    default:
        break;
  }

  if (state_->etc.rand_duration) {
    state_->etc.switch_duration_val_applied = random_.GetFloat() * state_->etc.switch_duration_val ; 
  }

  trellis_->programChange(state_->etc.current_program);  
}

void Modulator::run(double time_qp) {
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    for (int i = 0; i < NUM_CONTROLLERS; ++i) {
      Controller *const controller = &state_->channels[c].controller[i];
      UpdateController(controller, prev_time_qp_, time_qp);
    }
  }

  // ETC
  for (int i = 0; i < 5; ++i) {
    Controller *const controller = &state_->etc.controller[i];
    UpdateController(controller, prev_time_qp_, time_qp);
  }  
  EtcDoProgramSwitch(time_qp);  
  
  prev_time_qp_ = time_qp;
}
