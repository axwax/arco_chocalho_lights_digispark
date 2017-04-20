// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

MPU6050 accelgyro;
bool lights = false;

// the real exponent function is too slow, so I created an approximation (only for x < 0)
float myexp(float x) {
  return (1.0/(1.0-(0.634-1.344*x)*x));
}

#include <WS2812.h>
#define PIXEL_PIN   4 // (pin 1)
#define NUMPIXELS   30
#define BRIGHTNESS  40
#define FOCUS       65
#define DELAY       3000
#define ALPHA 0
WS2812 LED(NUMPIXELS); 
cRGB value;


void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();

    // initialize device
    accelgyro.initialize();

  // initialize LED strip
  LED.setOutput(PIXEL_PIN);
  LED.setColorOrderRGB();
}

long prev = 0;

void loop() {
    long t = millis();
    
    // these methods (and a few others) are also available
    int16_t ax, ay, az;
    accelgyro.getAcceleration(&ax, &ay, &az);

    float d = sqrt((ax/6000) * (ax/6000) + (ay/6000) * (ay/6000) + (az/6000) * (az/6000));


    if(d>5 && t-prev > 20){
      lights = true;
      prev = t;
    }
    else if(t-prev > 50){
      lights = false;
      prev = t;
    }

  float m = 94.68 + (float)t/DELAY;
  m = m - 42.5*cos(m/552.0) - 6.5*cos(m/142.0);
  
  // recalculate position of each spot (measured on a scale of 0 to 1)
  float posr = 0.5 + 0.55*sin(m*1.14);
  float posg = 0.5 + 0.55*sin(m*1.54);
  float posb = 0.5 + 0.55*sin(m*2.05);

  for (int i=0; i<NUMPIXELS; i++) {
    // pixel position on a scale from 0.0 to 1.0
    float ppos = (float)i / NUMPIXELS;
 
    // distance from this pixel to the center of each color spot
    float dr = ppos-posr;
    float dg = ppos-posg;
    float db = ppos-posb;

    if(lights){
      if(d<5.1){
        value.r = 100; // green
        value.g = 0;
        value.b = 0;
      }
      else if(d<6){ // yellow
        value.r = 100;
        value.g = 125;
        value.b = 0;
      }
      else{
        value.r = 0; // r= g
        value.g = 255; // g=r
        value.b = 0;      
      }
      LED.set_crgb_at(i, value);
    }
    else{
      value.r = constrain(BRIGHTNESS*myexp(-FOCUS*dr*dr),1,BRIGHTNESS);
      value.g = constrain(BRIGHTNESS*myexp(-FOCUS*dg*dg),1,BRIGHTNESS);
      value.b = constrain(BRIGHTNESS*myexp(-FOCUS*db*db),1,BRIGHTNESS);
      LED.set_crgb_at(i, value);      
    }
  }
  LED.sync();
  if(lights) delay(10);
  else delay(5);  
}
