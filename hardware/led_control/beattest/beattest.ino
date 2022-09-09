
#include <FastLED.h>
#include <string.h>

// How many leds in your strip?
#define NUM_LEDS (4*60)
#define NUM_PARAMS (10)

#define BLAH_SECTION_START (60)

#define DATA_PIN 22
#define CLOCK_PIN 13

#define FRAMES_PER_SECOND  30
#define PARAM_UPDATE_ANIMATION_PERIOD (500)

uint8_t Started = 0;

// Define the array of leds
CRGB leds[NUM_LEDS];

uint8_t Params[NUM_PARAMS];
uint8_t New_Params[NUM_PARAMS];
unsigned long Animate_Params[NUM_PARAMS];
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t BeatNumber = 0;

unsigned long LastCommandAt;

void Reset()
{
  for (int i = 0 ; i < NUM_LEDS ; i++) {
    int n = random8();
    if (n & 1) {
      leds[i] = CRGB::Red;
    }
    else {
      leds[i] = CRGB::Black;
    }
  }
}

void setup()
{
  bzero(Params, sizeof(Params));
  bzero(New_Params, sizeof(New_Params));
  bzero(Animate_Params, sizeof(Animate_Params));
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  // Cap power usage to 1.5 A
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  Serial.begin(9600);
  Reset();
}

void handleBeat()
{
  if ((BeatNumber % 4) == 0) {
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

void displayParams()
{
  for (int i = 0 ; i < BLAH_SECTION_START ; i++) {
    leds[i] = CRGB::White;
  }
  for (int pn = 0 ; pn < NUM_PARAMS ; pn++) {
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
  unsigned long m = millis();
  switch (cmd) {
    case COMMAND_SET_PARAM:
      Started = 1;
      LastCommandAt = m;
      // wrap around parameters, cap args to 5 bits
      New_Params[arg1 % NUM_PARAMS] = arg2 & 0x1F;
      Animate_Params[arg1 % NUM_PARAMS] = (m + PARAM_UPDATE_ANIMATION_PERIOD);
      break;
    case COMMAND_BEAT:
      LastCommandAt = m;
      Started = 1;
      // cap beats to 16
      BeatNumber = arg1 & 0xF;
      handleBeat();
      break;
  }
}

unsigned long target;
uint8_t Command;
uint8_t Arg1, Arg2;

#define STATE_WAIT_START (0)
#define STATE_WAIT_CMD (1)
#define STATE_WAIT_ARG1 (2)
#define STATE_WAIT_ARG2 (3)
#define STATE_WAIT_END (4)

uint8_t State = STATE_WAIT_START;


void ReadCommand()
{
  uint8_t ReadBuffer[5];

  if (Serial.available() > 4) {
    int num = Serial.readBytes(ReadBuffer, 5);
    for (int i = 0 ; i < 5; i++) {
      uint8_t b = ReadBuffer[i];

      // if we see a start byte, reset statemachine to start
      if (b == 0xFA) {
        State = STATE_WAIT_START;
      }

      switch (State) {
        case STATE_WAIT_START:
          if (b == 0xFA) {
            State = STATE_WAIT_CMD;
          }
          break;
        case STATE_WAIT_CMD:
          Command = b;
          State = STATE_WAIT_ARG1;
          break;
        case STATE_WAIT_ARG1:
          Arg1 = b;
          State = STATE_WAIT_ARG2;
          break;
        case STATE_WAIT_ARG2:
          Arg2 = b;
          State = STATE_WAIT_END;
          break;
        case STATE_WAIT_END:
          if (b == 255) {
            parseCommand(Command, Arg1, Arg2);
          }
          State = STATE_WAIT_START;
          break;
      }
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
        leds[i] -= CHSV( gHue & 0xF0, 255, 40);
      }
    }

    EVERY_N_MILLISECONDS( 40 ) {
      if (random8() < 70) {
        leds[ BLAH_SECTION_START + random16(NUM_LEDS - BLAH_SECTION_START - 1) ] += CRGB::Blue;
      }

      for (int i = 0; i < NUM_PARAMS; i++) {
        if (Animate_Params[i] > t) {
          Params[i] = random8() % 0x0F;
        }
        else {
          Params[i] = New_Params[i];
        }
      }
      displayParams();
    }

    if ((t - LastCommandAt) > (10 * 1000)) {
      Reset();
      Started = 0;
    }
  }

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  FastLED.show();
}
