// Mostly copied from https://gist.github.com/kriegsman/062e10f7f07ba8518af6

#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include "credentials.h"
#include <fauxmoESP.h>        //https://bitbucket.org/xoseperez/fauxmoesp

#define NUM_LEDS 8
#define DATA_PIN 3
#define FRAMES_PER_SECOND  120

CRGB leds[NUM_LEDS];

fauxmoESP fauxmo;

void wifiSetup() {

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.hostname("Kristalllampe");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  WiFi.setAutoReconnect(1);

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

uint8_t gCurrentPatternNumber = 6; // Index number of which pattern is current

void setup() { 
  Serial.begin(115200);
  LEDS.addLeds<WS2812,DATA_PIN,GRB>(leds,NUM_LEDS);
  LEDS.setBrightness(255);

  wifiSetup();

  fauxmo.addDevice("Dom Rainbow");                //device id 0
  fauxmo.addDevice("Dom RainbowWithGlitter");     //device id 1
  fauxmo.addDevice("Dom Confetti");               //device id 2
  fauxmo.addDevice("Dom Sinelon");                //device id 3
  fauxmo.addDevice("Dom Juggle");                 //device id 4
  fauxmo.addDevice("Dom Bpm");                    //device id 5

  fauxmo.onMessage([](unsigned char device_id, const char * device_name, bool state) {
    Serial.println(device_id);
    Serial.println(state);
    
    if (state == 1) {   // On
      gCurrentPatternNumber = device_id;
    } else {     // RF Off
      gCurrentPatternNumber = 6;
    }
  });
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, black };

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  fauxmo.handle();
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 40 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}


void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 30);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void black(){
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

