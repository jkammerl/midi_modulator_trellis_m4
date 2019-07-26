/*
 * controls.h
 *
 *  Created on: Oct 22, 2018
 *      Author: dean
 */

#ifndef CONTROLS_H_
#define CONTROLS_H_

#include <Adafruit_ADXL343.h>
#include <Adafruit_NeoTrellisM4.h>

#include "state.h"
#include "utils.h"

class Controls {
public:
  Controls(Adafruit_NeoTrellisM4 *trellis, State *state);
  ~Controls() {}

  void init();
  void run();

private:
  void ProcessKeys();
  void ProcessMuteKeys();
  void ProcessSpeedKeys();
  void ProcessModulationKeys();
  void ProcessChannelKeys();

  void RenderView();
  void RenderSpeedView();
  void RenderMuteView();
  void RenderModulationView();
  void RenderChannelView();

  void RandMute();
  void RandMode();
  void RandSpeed();
  void RandEverything();
  void ResetEverything();

  void CalculateAccelState();
  float GetAccelFactor(const sensors_event_t &event, float val);

  Adafruit_NeoTrellisM4 *const trellis_;
  State *const state_;

  unsigned long key_pressed_timestamp_;
  bool mode_button_down_;

  int edit_channel_;

  Random random_;
};

#endif /* CONTROLS_H_ */
