
#include <FastLED.h>
#include <string.h>

// How many leds in your strip?
#define NUM_LEDS (4*60)

#define BLAH_SECTION_START (60)

#define DATA_PIN 22
#define CLOCK_PIN 13

#define FRAMES_PER_SECOND  30

uint8_t Started = 0;

// Define the array of leds
CRGB leds[NUM_LEDS];

uint8_t Params[10];
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t BeatNumber = 0;

unsigned long LastCommandAt;


void setup()
{
  bzero(Params, sizeof(Params));
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);

  Serial.begin(9600);
}



void handleBeat()
{
  if (BeatNumber == 0 || BeatNumber == 4 || BeatNumber == 8 || BeatNumber == 12) {
    for (int i = BLAH_SECTION_START ; i < NUM_LEDS ; i++) {
      leds[i] = 0x220000;
    }
  }
  else {
    reRandom();
  }
}

void reRandom()
{
  for (int i = BLAH_SECTION_START ; i < NUM_LEDS ; i++) {
    int n = random8();
    if (n & 1) {
      leds[i] = CRGB::Red;
    }
    else {
      leds[i] = CRGB::Black;
    }
  }
}

CRGB ParamColors[] = {
  CRGB::Green,
  CRGB::Red,
  CRGB::Blue,
  CRGB::Yellow
};

void updateParams()
{
  for (int i = 0 ; i < BLAH_SECTION_START ; i++) {
    leds[i] = CRGB::White;
  }
  for (int pn = 0 ; pn < 10 ; pn++) {
    uint8_t param = Params[pn];
    uint8_t base_led = pn * 6;
    for (int i = 0 ; i < 5 ; i++) {
      if (param & (1 << i)) {
        leds[base_led + i] = ParamColors[pn % 4];
      }
      else {
        leds[base_led + i] = CRGB::Black;
      }
    }
  }
}

// command: set_param, 0x1
// arg1: param# 1-12
// arg2: value# 0-31 (will mask off > 5 bits)
#define COMMAND_SET_PARAM (0x1)
// command: beat, 0x2
// arg1: beat# (0-15)
#define COMMAND_BEAT (0x2)

void parseCommand(uint8_t cmd, uint8_t arg1, uint8_t arg2)
{
  switch (cmd) {
    case COMMAND_SET_PARAM:
      Params[arg1 % 10] = arg2 & 0x1F;
      updateParams();
      break;
    case COMMAND_BEAT:
      LastCommandAt = millis();
      Started = 1;
      BeatNumber = arg1 & 0xF;
      handleBeat();
      break;
  }
}

unsigned long target;
uint8_t brightness = 0;

void ReadCommand()
{
  uint8_t ReadBuffer[4];

  if (Serial.available()) {
    int num = Serial.readBytes(ReadBuffer, 4);
    // byte 4 should be 'stop' byte 255
    if (ReadBuffer[3] == 255) {
      parseCommand(ReadBuffer[0], ReadBuffer[1], ReadBuffer[2]);
    }
  }
}

void loop() {
  unsigned long t = millis();

  ReadCommand();

  if (!Started) {
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 20);
    int pos = beatsin16( 3, 0, NUM_LEDS - 1 );
    leds[pos] += CHSV( gHue & 0xF0, 255, 192);
  }
  else {
    EVERY_N_MILLISECONDS( 20 ) {
      gHue++;
      for (int i = BLAH_SECTION_START; i < NUM_LEDS; i++) {
        leds[i] -= CHSV( gHue * 0xF0, 255, 40);
      }
    }

    EVERY_N_MILLISECONDS( 40 ) {
      if ( random8() < 70) {
        leds[ BLAH_SECTION_START + random16(NUM_LEDS - BLAH_SECTION_START - 1) ] += CRGB::Pink;
      }
    }

    if ((t - LastCommandAt) > (10 * 1000)) {
      Started = 0;
    }
  }

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  FastLED.show();
}
