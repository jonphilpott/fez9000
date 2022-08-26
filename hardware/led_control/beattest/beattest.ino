
#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 252

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 22
#define CLOCK_PIN 13

#define FRAMES_PER_SECOND  30


// Define the array of leds
CRGB leds[NUM_LEDS];


void setup() {

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);

  Serial.begin(9600);
  reRandom();
}


char readBuffer[3];

void reRandom()
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

unsigned long t;
unsigned long target;
void loop() {
  t = millis();

  if (Serial.available()) {
    int num = Serial.readBytes(readBuffer, 3);
    Serial.print("read 1:");
    Serial.print((int) readBuffer[0]);
    Serial.print(", read 2:");
    Serial.print((int) readBuffer[1]);
    Serial.print(", read 3:");
    Serial.println((int) readBuffer[2]);


    if (readBuffer[2] == 255) {
      if (readBuffer[0] == 1) {
        reRandom();
      }

      if (readBuffer[0] == 2) {
        fill_rainbow( leds, NUM_LEDS, 0, 7);
      }
    }

    readBuffer[0] = readBuffer[1] = readBuffer[2] = 0;
  }


  if ( random8() < 80) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  FastLED.show();
}
