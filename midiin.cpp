#include "midiin.h"
#include "midimapping.h"

#include <MIDIUSB.h>

midiEventPacket_t rx;
int map_receiving_counter = 0xFF;

MidiIn::MidiIn(Adafruit_NeoTrellisM4 *trellis, FlashFs* flash_fs) : trellis_(trellis), flash_fs_(flash_fs) {}

void MidiIn::init() {
   trellis_->enableUARTMIDI(true);
   start_clocks_ = 0;
   clock_ticks_ = 0;
}

void MidiIn::process() {
  do {
    rx = MidiUSB.read();
    if (rx.header == 0) {
      return;
    }
    switch (rx.byte1) {
        case 0xF8:
           ++clock_ticks_;
           break;
        case 0xFA:
           ++start_clocks_ ;
           break;
        case 0xD0:
           {
             Serial.println("Midi Mapping Dump requested");
             const MidiMapping* midi_mapping = kMidiMapping;
             for (size_t i=0; i<kNumMidiMapping; ++i) {
              const midiEventPacket_t snc = {0x0B, 0xB1, 21, i};
              MidiUSB.sendMIDI(snc);
              const midiEventPacket_t snv_ml = {0x0B, 0xB2, max(0,min(midi_mapping[i].MSBC,0x7F)), max(0,min(midi_mapping[i].LSBC,0x7F))};
              MidiUSB.sendMIDI(snv_ml);
             } 
             MidiUSB.flush();
           }
           break;  
        case 0xD1:
             Serial.println("Begin map receiving");
             //map_receiving_counter = 0;
            break;
        case 0xB0:
           map_receiving_counter=rx.byte3;
           if (rx.byte3<kNumMidiMapping) {
              map_receiving_counter=rx.byte3;
           }
           break;
        case 0xB1:
           if (map_receiving_counter<kNumMidiMapping) {
              MidiMapping* midi_mapping = kMidiMapping;
              midi_mapping[map_receiving_counter].MSBC = rx.byte2;
              midi_mapping[map_receiving_counter].LSBC = rx.byte3;
           }
           break;
        case 0xD2:
           Serial.println("End map receiving, num buttons updated:");
           Serial.println(map_receiving_counter);
           map_receiving_counter = 0xFF;
           flash_fs_->WriteState(reinterpret_cast<const char*>(kMidiMapping), sizeof(MidiMapping)*kNumMidiMapping, MMAP_START_BLOCK);
           break;
        case 0xD3:
           Serial.println("Reset requested");            
           InitMidiMapping();
           flash_fs_->WriteState(reinterpret_cast<const char*>(kMidiMapping), sizeof(MidiMapping)*kNumMidiMapping, MMAP_START_BLOCK);
           flash_fs_->ReadState(reinterpret_cast<char*>(kMidiMapping), sizeof(MidiMapping)*kNumMidiMapping, MMAP_START_BLOCK);
           break;
        default:
           break;    
    }
    byte midi_status = rx.byte1 & 0xF0;
    if (midi_status!=0xF0) {
      Serial1.write(rx.byte1);
      Serial1.write(rx.byte2);
      if (midi_status!= 0xC0 &&
          midi_status!= 0xD0) { 
           Serial1.write(rx.byte3);
      }      
    }
  } while (true);
}

long MidiIn::num_start_clocks() {
  long ret = start_clocks_;
  start_clocks_ = 0;
  return ret;
}

long MidiIn::num_clock_ticks() {
  long ret = clock_ticks_;
  clock_ticks_ = 0;
  return ret;   
}
