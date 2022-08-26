/**
Control-Surface code for fez9000 knob board. pretty simple stuff.
*/

#include <Control_Surface.h> // Include the Control Surface library

// Instantiate a MIDI over USB interface.
USBMIDI_Interface midi;

CCButton buttons[] {
  { 7,  {MIDI_CC::General_Purpose_Controller_1, CHANNEL_12 }},
  { 2,  {MIDI_CC::General_Purpose_Controller_1, CHANNEL_13 }},
  { 5,  {MIDI_CC::General_Purpose_Controller_1, CHANNEL_14 }},
  { 3,  {MIDI_CC::General_Purpose_Controller_1, CHANNEL_15 }},
};

// Instantiate an array of CCPotentiometer objects
CCPotentiometer potentiometers[] {
  {A0, 0},
  {A1, 1},
  {A2, 2},
  {A3, 3},
  {A4, 4},
  {A5, 5},
  {A6, 6},
  {A7, 7},
  {A8, 8},
  {A9, 9},
  {A10, 10},
  {A11, 11},

};

void setup() {
  Control_Surface.begin(); // Initialize Control Surface
}

void loop() {
  Control_Surface.loop(); // Update the Control Surface
}
