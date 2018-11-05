#include <FastLED.h>
#include "btn.h"

#define DATA_PIN   9 // SPI MOSI pin
#define CLOCK_PIN  8 //13 //SPI  SCK

#define RESET_PIN  1

#define COLOR_ORDER BGR  // most of the 10mm black APA102

#define CHIPSET     APA102
#define NUM_LEDS    40

#define FPS 100

CRGBArray<NUM_LEDS> leds;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
int brightness = 255;

// set the number of pixels we want on
uint8_t NUM_ON = NUM_LEDS;

uint8_t NUM_OUTSIDE = 20;
uint8_t NUM_INSIDE = 20;

float INNER_SCALE = 0.6;
int MAX_BRIGHT = 255;
int MIN_BRIGHT = 20;

int TEST_RATE_MS = 10000; // testing delay between iterations

// what probability (from uniform dist) do we want to have a desaturated pixel?
// NOTE: do we want a desaturated pixel, or desaturated color set?
float desat_freq = 0.1 / NUM_ON;  // prob that is desaturated

Btn btn_reset(RESET_PIN);


// ***********************************************
void setup() {
  pinMode(RESET_PIN, INPUT_PULLUP);
  
  randomSeed(analogRead(0));
  delay(500); // sanity delay
  
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(8)>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( brightness );

  Serial.begin(9600);
  Serial.println("Start ");
  Serial.println("using HSV");
  currentBlending = LINEARBLEND;
}

void loop()
{
//  // check reset button state
//  btn_reset.poll(
//    // button pressed
//    [](){
//      set_aura_colors(0, NUM_LEDS);
//    },
//    // button held
//    [](){
//      set_aura_multicolor(0, NUM_LEDS);
//    }
//  );

  // update the pixel array
  EVERY_N_MILLISECONDS( TEST_RATE_MS ) { write_colors(); }
  
  FastLED.show();
  delayToSyncFrameRate(FPS);
}

// ************************************************************
float random_float() {
  float f = float(random(10000) / 10000.0);
  return f;
}

void print_color(CHSV color) {
  Serial.print(color.h);
  Serial.print(",");
  Serial.print(color.s);  
  Serial.print(",");
  Serial.println(color.v);
}

void write_colors() {
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
//  set_aura_colors(0, NUM_LEDS, false);
  set_aura_primary_color(0, NUM_OUTSIDE, false);
  set_aura_multicolor(NUM_OUTSIDE, NUM_LEDS, true);
  
}

CHSV get_random_color() {
  //want to get random hue, saturation towards fully saturated
  CHSV color = CHSV(random8(), random8(200, 255), random8(MIN_BRIGHT, MAX_BRIGHT));
  return color;
}


void set_aura_colors(int start_i, int end_i, int dim_half) {
  // set the random color 
  CHSV color = get_random_color();
  for(int i=start_i; i<end_i; i++) {
    CHSV set_color = color;
    // we want to have a white aura be a rare thing, so overwrite saturation if we happen to roll the dice well
    if(random_float() < desat_freq) {
      Serial.print("*");
      set_color.sat = random8(50, 100);
    }
    // drop the intensity for the inner circle
    if (dim_half) {
      if(i >= start_i && i < start_i + floor((end_i - start_i)/2)) {
        set_color.val = uint8_t(set_color.value * INNER_SCALE);
      }
    }
    leds[i] = set_color; 
  }
  print_color(color);
}


void set_aura_multicolor(int start_i, int end_i, int dim_half) {
  for(int i=start_i; i<end_i; i++) {
    CHSV set_color = get_random_color();
    // set all cool colors
    set_color.hue = random8(80, 240); // yellow-green to pink
    // drop the intensity for the inner circle
    if (dim_half) {
      if(i >= start_i && i < start_i + floor((end_i - start_i)/2)) {
//        Serial.print("dim:");
//        Serial.print(set_color.value );
//        Serial.print("->");
//        Serial.println(set_color.value * INNER_SCALE);
        set_color.val = uint8_t(set_color.value * INNER_SCALE);
      }
    }
    leds[i] = set_color;
  }
}

void set_aura_primary_color(int start_i, int end_i, int dim_half) {
  CHSV color = get_random_color();
  for(int i=start_i; i<end_i; i++) {
    CHSV set_color = color;
    uint8_t hue = set_color.hue;
//    Serial.print(hue);
//    Serial.print(":");
//    Serial.print((hue-30) % 256);
//    Serial.print(",");
//    Serial.println((hue+30) % 256);
    set_color.hue = random8((hue-30) % 256, (hue+30) % 256);
    set_color.value = set_color.value * 0.8;
    // drop the intensity for the inner circle
    if (dim_half) {
      if(i >= start_i && i < start_i + floor((end_i - start_i)/2)) {
        set_color.val = uint8_t(set_color.value * INNER_SCALE);
      }
    }
    leds[i] = set_color;
  }
}



void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        // the shift along color index changes the frequency of the pattern oscillations
        // higher numbers increase the frequency, shortening the period
        colorIndex += 1; // from PaletteTrace.ino
    }
}


void fill_mirror_from_palette(uint8_t colorIndex)
{
  for(int i=0; i < int(NUM_LEDS/2); i++) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
    leds[NUM_LEDS - i - 1] = leds[i];
    colorIndex += 1; // the color shift frequency
  }
}


// delayToSyncFrameRate - delay how many milliseconds are needed
//   to maintain a stable frame rate.
static void delayToSyncFrameRate( uint8_t framesPerSecond) {
  static uint32_t msprev = 0;
  uint32_t mscur = millis();
  uint16_t msdelta = mscur - msprev;
  uint16_t mstargetdelta = 1000 / framesPerSecond;
  // Serial.print("frame dt: ");
  // Serial.println(msdelta);
  if ( msdelta < mstargetdelta) {
    FastLED.delay( mstargetdelta - msdelta);
  }
  msprev = mscur;
}
