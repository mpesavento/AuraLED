#include <FastLED.h>
#include "btn.h"

#define DATA_PIN   9 // SPI MOSI pin
#define CLOCK_PIN  8 //13 //SPI  SCK

#define RESET_PIN  1

#define COLOR_ORDER BGR  // most of the 10mm black APA102

#define CHIPSET     APA102
#define NUM_LEDS    20

#define FPS 100

//CRGB leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
int brightness = 255;

int MAX_BRIGHT = 100;
int MIN_BRIGHT = 20;

float desat_freq = 0.1;

Btn btn_reset(RESET_PIN);

// set the number of pixels we want on
uint8_t NUM_ON = 4;

uint8_t values[NUM_LEDS];

// ***********************************************
void setup() {
  pinMode(RESET_PIN, INPUT_PULLUP);
  
  randomSeed(analogRead(0));
  delay(500); // sanity delay
  
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(8)>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( brightness );

  Serial.begin(9600);
  Serial.print("Start ");
  currentBlending = LINEARBLEND;
}

void loop()
{
  // check reset button state
  btn_reset.poll(
    // button pressed
    [](){
      set_aura_colors();
    },
    // button held
    [](){
      set_aura_multicolor();
    }
  );

  // FOR TESTING
  EVERY_N_MILLISECONDS( 2000 ) { set_aura_colors(); }
//  set_aura_colors();
//  EVERY_N_MILLISECONDS( 2000 ) { set_aura_multicolor(); }
//  set_aura_multicolor();

//  static uint8_t paletteIndex = 0; // the number of pixels that the pattern phase shifts each time
//  // decouple the rate from the index by waiting for some time
//  EVERY_N_MILLISECONDS( 80 ) { paletteIndex++; }  
//  fill_mirror_from_palette(paletteIndex);
  FastLED.show();
  delayToSyncFrameRate(FPS);
}

float random_float() {
  float f = float(random(10000) / 10000.0);
  Serial.println(f);
  return f;
}

void print_color(CHSV color) {
  Serial.print(color.h);
  Serial.print(",");
  Serial.print(color.s);
  Serial.print(",");
  Serial.println(color.v);
}

CHSV get_random_color() {
  //want to get random hue, saturation towards fully saturated
  CHSV color = CHSV(random8(), 255, random8(MIN_BRIGHT, MAX_BRIGHT));
  // we want to have a white aura be a rare thing, so overwrite saturation if we happen to roll the dice well
//  if(random_float() < desat_freq) {
//    Serial.println("*");
//    color.saturation = random8(100, 240);
//  }
  print_color(color);
}

void set_aura_colors() {
  // set the random color 
  CHSV color = get_random_color();
  // fill all to black
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  // except our target pixels
  for(int i=0; i<NUM_ON; i++) {
    CHSV set_color = color;
    // randomize the brightness
    set_color.value = random8(MIN_BRIGHT, MAX_BRIGHT);
    leds[random8(NUM_LEDS)] = set_color; 
  }
}


void set_aura_multicolor() {
  // fill all to black
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  // except our target pixels
  for(int i=0; i<NUM_ON; i++) {
    leds[random8(NUM_LEDS)] = get_random_color(); 
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
