/*
   controls.h

    Created on: Oct 22, 2018
        Author: dean
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
    void run(bool midi_clock_blink);

  private:
    template<typename F>
    void ApplyKeyRangeToControllers(F &function);

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
    void ResetChannel();

    void CalculateAccelState();
    float GetAccelFactor(const sensors_event_t &event, float val);

    void ResetKeyRange();
    int UpdateKeyRange();

    Adafruit_NeoTrellisM4 *const trellis_;
    State *const state_;

    unsigned long key_pressed_timestamp_;
    bool mode_button_down_;

    int edit_channel_;

    struct {
      bool ops_range_defined;
      int xmin;
      int ymin;
      int xmax;
      int ymax;
    } key_range_;

    Random random_;
};

#endif /* CONTROLS_H_ */
