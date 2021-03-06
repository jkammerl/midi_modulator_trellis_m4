/*
   controls.cpp

    Created on: Oct 22, 2018
        Author: dean
*/

#include "controls.h"
#include "state.h"
#include "midimapping.h"

#undef max
#undef min
#include <algorithm>

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
                               23, 25, 26, 27, 28, 29, 30, 31
                              };
int controller_to_key_map_size = sizeof(controller_to_key_map) / sizeof(int);
int key_to_controller_map[32];


Controls::Controls(Adafruit_NeoTrellisM4 *trellis, State *state, FlashFs* flash_fs)
  : trellis_(trellis), state_(state), flash_fs_(flash_fs) {
  key_pressed_timestamp_ = 0;
  mode_button_down_ = false;
  edit_channel_ = 0;
  memset(key_to_controller_map, 0xFF, sizeof(key_to_controller_map));
  for (int i = 0; i < controller_to_key_map_size; ++i) {
    key_to_controller_map[controller_to_key_map[i]] = i;
  }
  ResetKeyRange();
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
       connections */
    Serial.println("Ooops, no ADXL343 detected");
    while (1)
      ;
  } else {
    Serial.println("ADXL343 detected");
  }

  accel.setRange(ADXL343_RANGE_16_G);

  Serial.println("Trellis started");
}

template<typename F>
void Controls::ApplyKeyRangeToControllers(F &function) {
  if (!key_range_.ops_range_defined) {
    return;
  }
  for (int x = key_range_.xmin; x <= key_range_.xmax; ++x) {
    for (int y = key_range_.ymin; y <= key_range_.ymax; ++y) {
      int controller_idx = key_to_controller_map[y * 8 + x];
      if (controller_idx >= 0 && controller_idx < 32) {
        function(controller_idx);
      }
    }
  }
}

void Controls::ProcessMuteKeys() {
  auto func = [this](int controller_idx) {
    state_->channels[edit_channel_].controller[controller_idx].active =
      !state_->channels[edit_channel_].controller[controller_idx].active;
  };
  ApplyKeyRangeToControllers(func);
}

void Controls::ProcessModulationKeys() {
  auto func = [this](int controller_idx) {
    Controller *const controller =
      &state_->channels[edit_channel_].controller[controller_idx];
    switch (controller->mode) {
      case Controller::Mode::SINUS:
        controller->mode = Controller::Mode::PULSE;
        break;
      case Controller::Mode::PULSE:
        controller->mode = Controller::Mode::WALK;
        controller->state = random_.GetFloat();
        break;
      case Controller::Mode::WALK:
        controller->mode = Controller::Mode::SINUS;
        break;
      default:
        break;
    }
  };
  ApplyKeyRangeToControllers(func);
}

void Controls::ProcessSpeedKeys() {
  auto func = [this](int controller_idx) {
    Controller *const controller =
      &state_->channels[edit_channel_].controller[controller_idx];
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
  };
  ApplyKeyRangeToControllers(func);
}

void Controls::ProcessChannelKeys() {
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    if (trellis_->isPressed(c + 1)) {
      state_->channels[c].muted = !state_->channels[c].muted;
    }
    for (int i = 1; i < 4; ++i) {
      if (trellis_->isPressed(c + 1 + i * 8)) {
        edit_channel_ = c;
        return;
      }
    }
  }
}

void Controls::ResetKeyRange() {
  key_range_.ops_range_defined = false;
  key_range_.xmin = key_range_.ymin = std::numeric_limits<int>::max();
  key_range_.xmax = key_range_.ymax = 0;
}

int Controls::UpdateKeyRange() {
  int num_key_pressed = 0;
  for (int i = 0; i < 32; ++i) {
    if (i % 8 == 0) {
      // Skip mode keys.
      continue;
    }
    if (trellis_->isPressed(i)) {
      ++num_key_pressed;
      key_range_.ops_range_defined = true;
      int xpos = i % 8;
      int ypos = i / 8;
      key_range_.xmin = std::min( key_range_.xmin, xpos);
      key_range_.ymin = std::min(key_range_.ymin, ypos);
      key_range_.xmax = std::max(key_range_.xmax, xpos);
      key_range_.ymax = std::max(key_range_.ymax, ypos);
    }
  }
  return num_key_pressed;
}

void Controls::ProcessKeys() {
  while (trellis_->available()) {
    keypadEvent e = trellis_->read();
    int keyindex = e.bit.KEY;
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      if (trellis_->isPressed(KEY_MUTE_MODE)) {
        state_->op_mode = State::OpMode::MUTE;
        flash_fs_->WriteState(reinterpret_cast<const char*>(state_), sizeof(State), STATE_START_BLOCK);
        if (!mode_button_down_) {
          key_pressed_timestamp_ = millis();
          mode_button_down_ = true;
        }
      } else if (trellis_->isPressed(KEY_MODULATION_MODE)) {
        state_->op_mode = State::OpMode::MODULATION;
        flash_fs_->WriteState(reinterpret_cast<const char*>(state_), sizeof(State), STATE_START_BLOCK);
        if (!mode_button_down_) {
          key_pressed_timestamp_ = millis();
          mode_button_down_ = true;
        }
      } else if (trellis_->isPressed(KEY_SPEED_MODE)) {
        state_->op_mode = State::OpMode::SPEED;
        flash_fs_->WriteState(reinterpret_cast<const char*>(state_), sizeof(State), STATE_START_BLOCK);
        if (!mode_button_down_) {
          key_pressed_timestamp_ = millis();
          mode_button_down_ = true;
        }
      } else if (trellis_->isPressed(KEY_CHANNEL_MODE)) {
        state_->op_mode = State::OpMode::CHANNEL;
        flash_fs_->WriteState(reinterpret_cast<const char*>(state_), sizeof(State), STATE_START_BLOCK);
        if (!mode_button_down_) {
          key_pressed_timestamp_ = millis();
          mode_button_down_ = true;
        }
      }
      if (state_->op_mode == State::OpMode::CHANNEL) {
        ProcessChannelKeys();
      }
    }
    const int num_keys_pressed = UpdateKeyRange();
    if (num_keys_pressed == 0 && key_range_.ops_range_defined) {
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
        default:
          break;
      }
      ResetKeyRange();
    }

    if (trellis_->isPressed(KEY_CHANNEL_MODE) &&
        trellis_->isPressed(KEY_MUTE_MODE)) {
      ResetChannel();
    }
    if (trellis_->isPressed(KEY_CHANNEL_MODE) &&
        trellis_->isPressed(KEY_MUTE_MODE) &&
        trellis_->isPressed(KEY_MODULATION_MODE) &&
        trellis_->isPressed(KEY_MODULATION_MODE)) {
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
  uint32_t kModeColors[] = {0x00FFFF, 0x0000FF, 0x00FF00, 0xFF0000};

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
  const uint8_t state_byte = 255.0f * BRIGHTNESS;
  for (int i = 1; i < 4; ++i) {
    const uint32_t state_color = state_byte << 8;
    trellis_->setPixelColor(edit_channel_ + 1 + i * 8, state_color);
  }
  for (int c = 0; c < NUM_CHANNELS; ++c) {
    if (state_->channels[c].muted) {
      const uint32_t state_color = state_byte << 16;
      trellis_->setPixelColor(c + 1, state_color);
    }
  }
}

void Controls::RenderView() {
  const bool muted = state_->channels[edit_channel_].muted;
  uint8_t mode_byte = 255.0f * BRIGHTNESS;
  uint32_t mode_color = (mode_byte << 16) + (mode_byte << 8) + mode_byte;
  if (muted) {
    mode_color &= 0xFF0000;
  }

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

void Controls::ResetChannel() {
  memset(&state_->channels[edit_channel_], 0, sizeof(Channel));
  flash_fs_->WriteState(reinterpret_cast<const char*>(state_), sizeof(State), STATE_START_BLOCK);
}
void Controls::ResetEverything() {
  Serial.println("Full reset");
  memset(state_, 0, sizeof(State));
  flash_fs_->WriteState(reinterpret_cast<const char*>(state_), sizeof(State), STATE_START_BLOCK);
  InitMidiMapping();
  flash_fs_->WriteState(reinterpret_cast<char*>(kMidiMapping), sizeof(MidiMapping)*kNumMidiMapping, MMAP_START_BLOCK);
  delay(200);
}

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

void Controls::run(bool midi_clock_blink) {
  trellis_->tick();
  trellis_->fill(0);


  if (midi_clock_blink) {
    const uint8_t mode_byte = 255.0f * BRIGHTNESS;
    uint32_t mode_color = (mode_byte << 16) + (mode_byte << 8) + mode_byte;
    trellis_->setPixelColor(0, mode_color);
    trellis_->setPixelColor(8, mode_color);
    trellis_->setPixelColor(16, mode_color);
    trellis_->setPixelColor(24, mode_color);
  }

  CalculateAccelState();
  ProcessKeys();
  RenderView();

  trellis_->show();
}
