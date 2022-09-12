
#include <FastLED.h>
#include <string.h>
#include <esp_task_wdt.h>



#define WDT_TIMEOUT 3

#define DEFAULT_CURRENT_DRAW (1000)
#define MAX_CURRENT_DRAW (4000)

// How many leds in your strip?
#define NUM_LEDS (4*60)
#define NUM_PARAMS (10)

#define BLAH_SECTION_START (60)

#define DATA_PIN 22
#define CLOCK_PIN 13

#define FRAMES_PER_SECOND  30
#define PARAM_UPDATE_ANIMATION_PERIOD (100)

bool On_the_one = false;

uint8_t Started = 0;

// Define the array of leds
CRGB leds[NUM_LEDS];

uint8_t Params[NUM_PARAMS];
uint8_t New_Params[NUM_PARAMS];
unsigned long Animate_Params[NUM_PARAMS];
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t BeatNumber = 0;

unsigned long LastCommandAt;

// layout to say "FEZ9000"
uint8_t FEZTABLE[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};

void ShowFez9000()
{
  for (int i = 0 ; i < NUM_LEDS; i++) {
    uint8_t p = FEZTABLE[i];
    if (p) {
      leds[i] = CRGB::Red;
    }
    else {
      leds[i] = CRGB::Black;
    }
  }
}

void Reset()
{
  ShowFez9000();
}

void setup()
{
  bzero(Params, sizeof(Params));
  bzero(New_Params, sizeof(New_Params));
  bzero(Animate_Params, sizeof(Animate_Params));
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  // Cap power usage to 1.5 A
  FastLED.setMaxPowerInVoltsAndMilliamps(5, DEFAULT_CURRENT_DRAW);
  Serial.begin(9600);

  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  Reset();
}

void handleBeat()
{
  reRandom();
}

void reRandom()
{
  CRGB on_c = CRGB::Red;
  CRGB off_c = CRGB::Red;
  off_c /= 2;
  for (int i = BLAH_SECTION_START ; i < NUM_LEDS ; i++) {
    int n = random8();
    if (n & 1) {
      leds[i] = On_the_one ? on_c : off_c;
    }
    else {
      leds[i] = CRGB::Black;
    }
  }
}

CRGB ParamColors[] = {
  CRGB::Green,
  CRGB::Purple,
  CRGB::Blue,
  CRGB::Yellow
};

void displayParams()
{
  unsigned long t = millis();

  for (int i = 0 ; i < BLAH_SECTION_START ; i++) {
    leds[i] = CRGB::Red;
  }
  for (int pn = 0 ; pn < NUM_PARAMS ; pn++) {
    uint8_t param = Params[pn];
    uint8_t base_led = pn * 6;
    bool is_updating = Animate_Params[pn] > t;
    CRGB c = ParamColors[pn % 4];

    if (is_updating) {
      c /= 4;
    }

    for (int i = 0 ; i < 5 ; i++) {
      if (param & (1 << i)) {
        leds[base_led + i] = c;
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
#define COMMAND_BRIGHTNESS (0x3)


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
      On_the_one = ((BeatNumber % 4) == 0);
      handleBeat();
      break;

    case COMMAND_BRIGHTNESS:
      FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_CURRENT_DRAW >> arg1);
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

void FlushSerial()
{
  if (Serial.available()) {
    Serial.readString();
  }
}

void loop() {
  unsigned long t = millis();
  ReadCommand();

  if (!Started) {
    EVERY_N_MILLISECONDS( 20 ) {
      // shamelessly copy-pasta'd from fastled examples
      gHue++;
      uint8_t BeatsPerMinute = 82;
      CRGBPalette16 palette = PartyColors_p;
      uint8_t beat = beatsin8( BeatsPerMinute, 90, 255);
      for ( int i = 0; i < NUM_LEDS; i++) {
        if (FEZTABLE[i]) {
          leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
      }
    }
  }
  else {
    EVERY_N_MILLISECONDS( 40 ) {
      for (int i = BLAH_SECTION_START; i < NUM_LEDS; i++) {
        if (random8() > 128) {
          leds[i] /= 2;
        }
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
      // if its been more than 10 seconds since we've had a command go to wait mode
      Reset();
      // flush out the serial buffer
      FlushSerial();
      Started = 0;
    }
  }
  esp_task_wdt_reset();

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  FastLED.show();
}
