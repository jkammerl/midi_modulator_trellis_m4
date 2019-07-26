/*
 * controls.cpp
 *
 *  Created on: Oct 22, 2018
 *      Author: dean
 */
#undef max
#undef min
#include <algorithm>

#include "controls.h"
#include "state.h"

// Configuration for the datalogging file:
#define FILE_NAME "/data.raw"

#define KEY_MUTE_MODE 0
#define KEY_SPEED_MODE 8
#define KEY_MODULATION_MODE 16
#define KEY_CHANNEL_MODE 24

#define BRIGHTNESS 0.2f
#define LONG_KEYPRESS_MS 1000

Adafruit_ADXL343 accel = Adafruit_ADXL343(123, &Wire1);

int controller_to_key_map[] = {1,  2,  3,  4,  5,  6,  7,  9,  10, 11,
                               12, 13, 14, 15, 17, 18, 19, 20, 21, 22,
                               23, 25, 26, 27, 28, 29, 30, 31};

Controls::Controls(Adafruit_NeoTrellisM4 *trellis, State *state)
    : trellis_(trellis), state_(state) {
  key_pressed_timestamp_ = 0;
  mode_button_down_ = false;
  edit_channel_ = 0;
}

void Controls::init() {
  Serial.println("Control initialization");
  trellis_->begin();
  trellis_->setBrightness(255); // 255
  trellis_->autoUpdateNeoPixels(false);
  trellis_->show();

  /* Initialise the sensor */
  if (!accel.begin()) {
    /* There was a problem detecting the ADXL343 ... check your
     * connections */
    Serial.println("Ooops, no ADXL343 detected");
    while (1)
      ;
  } else {
    Serial.println("ADXL343 detected");
  }

  accel.setRange(ADXL343_RANGE_16_G);

  Serial.println("Trellis started");
}

void Controls::ProcessMuteKeys() {
  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    if (trellis_->isPressed(controller_to_key_map[i])) {
      state_->channels[edit_channel_].controller[i].active =
          !state_->channels[edit_channel_].controller[i].active;
    }
  }
}

void Controls::ProcessModulationKeys() {
  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    if (trellis_->isPressed(controller_to_key_map[i])) {
      Controller *const controller =
          &state_->channels[edit_channel_].controller[i];
      switch (controller->mode) {
      case Controller::Mode::SINUS:
        controller->mode = Controller::Mode::PULSE;
        break;
      case Controller::Mode::PULSE:
        controller->mode = Controller::Mode::RAND;
        break;
      case Controller::Mode::RAND:
        controller->mode = Controller::Mode::WALK;
        break;
      case Controller::Mode::WALK:
        controller->mode = Controller::Mode::SINUS;
        break;
      default:
        break;
      }
    }
  }
}

void Controls::ProcessSpeedKeys() {
  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    if (trellis_->isPressed(controller_to_key_map[i])) {
      Controller *const controller =
          &state_->channels[edit_channel_].controller[i];
      switch (controller->speed) {
      case Controller::Speed::SLOW:
        controller->speed = Controller::Speed::MED;
        break;
      case Controller::Speed::MED:
        controller->speed = Controller::Speed::FAST;
        break;
      case Controller::Speed::FAST:
        controller->speed = Controller::Speed::ULTRA;
        break;
      case Controller::Speed::ULTRA:
        controller->speed = Controller::Speed::SLOW;
        break;
      default:
        break;
      }
    }
  }
}

void Controls::ProcessChannelKeys() {
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    for (int i = 0; i < 4; ++i) {
      if (trellis_->isPressed(c + 1 + i * 8)) {
        edit_channel_ = c;
        return;
      }
    }
  }
}

void Controls::ProcessKeys() {
  while (trellis_->available()) {
    keypadEvent e = trellis_->read();
    int keyindex = e.bit.KEY;
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      if (trellis_->isPressed(KEY_MUTE_MODE)) {
        state_->op_mode = State::OpMode::MUTE;
        if (!mode_button_down_) {
          key_pressed_timestamp_ = millis();
          mode_button_down_ = true;
        }
      } else if (trellis_->isPressed(KEY_MODULATION_MODE)) {
        state_->op_mode = State::OpMode::MODULATION;
        if (!mode_button_down_) {
          key_pressed_timestamp_ = millis();
          mode_button_down_ = true;
        }
      } else if (trellis_->isPressed(KEY_SPEED_MODE)) {
        state_->op_mode = State::OpMode::SPEED;
        if (!mode_button_down_) {
          key_pressed_timestamp_ = millis();
          mode_button_down_ = true;
        }
      } else if (trellis_->isPressed(KEY_CHANNEL_MODE)) {
        state_->op_mode = State::OpMode::CHANNEL;
        if (!mode_button_down_) {
          key_pressed_timestamp_ = millis();
          mode_button_down_ = true;
        }
      }
      switch (state_->op_mode) {
      case State::OpMode::MUTE:
        ProcessMuteKeys();
        break;
      case State::OpMode::MODULATION:
        ProcessModulationKeys();
        break;
      case State::OpMode::SPEED:
        ProcessSpeedKeys();
        break;
      case State::OpMode::CHANNEL:
        ProcessChannelKeys();
      default:
        break;
      }
    }
    if (trellis_->isPressed(KEY_CHANNEL_MODE) &&
        trellis_->isPressed(KEY_MUTE_MODE)) {
      ResetEverything();
    }
    if (e.bit.EVENT == KEY_JUST_RELEASED) {
      mode_button_down_ = false;
    }
  }
  if (mode_button_down_ &&
      millis() - key_pressed_timestamp_ > LONG_KEYPRESS_MS) {
    if (trellis_->isPressed(KEY_MUTE_MODE)) {
      RandMute();
    } else if (trellis_->isPressed(KEY_MODULATION_MODE)) {
      RandMode();
    } else if (trellis_->isPressed(KEY_SPEED_MODE)) {
      RandSpeed();
    }
    if (trellis_->isPressed(KEY_CHANNEL_MODE)) {
      RandEverything();
      state_->op_mode = State::OpMode::MUTE;
    }
    mode_button_down_ = false;
  }
}

void Controls::RenderModulationView() {
  uint32_t kModeColors[] = {0x00FFFF, 0xFF0000, 0x00FF00, 0x0000FF};

  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    uint32_t overlay =
        kModeColors[static_cast<size_t>(
                        state_->channels[edit_channel_].controller[i].mode) %
                    4];
    uint8_t state_byte =
        state_->channels[edit_channel_].controller[i].accel_state * 255.0f *
        BRIGHTNESS;
    uint32_t state_color = (state_byte << 16) + (state_byte << 8) + state_byte;
    trellis_->setPixelColor(controller_to_key_map[i], state_color & overlay);
  }
}

void Controls::RenderMuteView() {
  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    uint32_t overlay = 0xFFFFFF;
    if (!state_->channels[edit_channel_].controller[i].active) {
      overlay = 0XFF0000;
    }
    uint8_t state_byte =
        state_->channels[edit_channel_].controller[i].accel_state * 255.0f *
        BRIGHTNESS;
    uint32_t state_color = (state_byte << 16) + (state_byte << 8) + state_byte;
    trellis_->setPixelColor(controller_to_key_map[i], state_color & overlay);
  }
}

void Controls::RenderSpeedView() {
  uint32_t kSpeedColors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0x00FFFF};

  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    uint32_t overlay =
        kSpeedColors[static_cast<size_t>(
                         state_->channels[edit_channel_].controller[i].speed) %
                     4];
    uint8_t state_byte =
        state_->channels[edit_channel_].controller[i].speed_visualization *
        255.0f * BRIGHTNESS;
    uint32_t state_color = state_byte;
    trellis_->setPixelColor(controller_to_key_map[i], state_color);
  }
}

void Controls::RenderChannelView() {
  for (int i = 0; i < 4; ++i) {
    uint8_t state_byte = 255.0f * BRIGHTNESS;
    uint32_t state_color = state_byte << 8;
    trellis_->setPixelColor(edit_channel_ + 1 + i * 8, state_color);
  }
}

void Controls::RenderView() {
  uint8_t mode_byte = 255.0f * BRIGHTNESS;
  uint32_t mode_color = (mode_byte << 16) + (mode_byte << 8) + mode_byte;

  switch (state_->op_mode) {
  case State::OpMode::MUTE:
    trellis_->setPixelColor(KEY_MUTE_MODE, mode_color);
    RenderMuteView();
    break;
  case State::OpMode::MODULATION:
    trellis_->setPixelColor(KEY_MODULATION_MODE, mode_color);
    RenderModulationView();
    break;
  case State::OpMode::SPEED:
    trellis_->setPixelColor(KEY_SPEED_MODE, mode_color);
    RenderSpeedView();
    break;
  case State::OpMode::CHANNEL:
    trellis_->setPixelColor(KEY_CHANNEL_MODE, mode_color);
    RenderChannelView();
    break;
  default:
    break;
  }
}

void Controls::ResetEverything() { memset(state_, 0, sizeof(State)); }

void Controls::RandMute() {
  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    Controller *controller = &state_->channels[edit_channel_].controller[i];
    controller->active = random_.GetInt() & 1;
  }
}

void Controls::RandMode() {
  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    Controller *controller = &state_->channels[edit_channel_].controller[i];
    controller->mode = static_cast<Controller::Mode>(
        random_.GetInt() % Controller::Mode::NUM_MODES);
  }
}

void Controls::RandSpeed() {
  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    Controller *controller = &state_->channels[edit_channel_].controller[i];
    controller->speed = static_cast<Controller::Speed>(
        random_.GetInt() % Controller::Speed::NUM_SPEEDS);
  }
}

void Controls::RandEverything() {
  RandMute();
  RandMode();
  RandSpeed();
  for (int i = 0; i < NUM_CONTROLLERS; ++i) {
    Controller *controller = &state_->channels[edit_channel_].controller[i];
    controller->phase = random_.GetInt();
  }
}

float Controls::GetAccelFactor(const sensors_event_t &event, float val) {
  /* Get a new sensor event */
  float accel_val = event.acceleration.y * -(1.0f / 11.0f);
  accel_val = std::max<float>(std::min<float>(accel_val, 1.0f), -1.0f);
  if (accel_val < 0.0f) {
    return (1.0f + accel_val) * val;
  } else {
    return 1.0f - (1.0f - accel_val) * (1.0f - val);
  }
}

void Controls::CalculateAccelState() {
  sensors_event_t event;
  accel.getEvent(&event);

  for (int c = 0; c < NUM_CHANNELS; ++c) {
    for (int i = 0; i < NUM_CONTROLLERS; ++i) {
      Controller *controller = &state_->channels[c].controller[i];
      controller->accel_state = GetAccelFactor(event, controller->state);
    }
  }
}

void Controls::run() {
  trellis_->tick();
  trellis_->fill(0);

  CalculateAccelState();
  ProcessKeys();
  RenderView();

  trellis_->show();
}
