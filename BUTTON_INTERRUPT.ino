#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN 6                                        // Datapin rings are connected to
#define PIXEL_COUNT 40                                     // Number of pixels

#define MIC_PIN A0

#define BRIGHTNESS_MAX 255                                 // max brightness
#define MIC_ANIMATION_MODES 2                              // number of mic based animation modes
#define ANIMATION_MODES 13                                  // number of "normal" animation modes

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800); // init neo pixel rings

volatile bool oldStateButton0 = HIGH;
volatile bool oldStateButton1 = HIGH;
volatile bool oldStateButton2 = HIGH;
volatile float brightness = 32;
volatile bool micMode = LOW;
volatile uint8_t currentAnimationMode = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Startup");
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  for(int i = 10; i< 13;i++) {
    digitalWrite(i,HIGH);
  }
  pciSetup(10);
  pciSetup(11);
  pciSetup(12);
  strip.begin();
  strip.show();
}

void loop() {
  if(micMode) {
    switch(currentAnimationMode) {
      case 0:
        equalizer();
        break;
      case 1:
        Serial.println("Mic Animation Mode 1");
        delay(1000);
        break;        
      default:
        break;
    }
  } else {
    switch(currentAnimationMode) {
      case 0: colorWipe(strip.Color(0, 0, 0), 50);    // Black/off
            break;
      case 1: colorWipe(strip.Color((uint8_t)(255*(brightness/255)), 0, 0), 50);  // Red
              colorWipe(strip.Color(0, 0, 0), 50);
              break;
      case 2: colorWipe(strip.Color(0, (uint8_t)(255*(brightness/255)), 0), 50);  // Green
              colorWipe(strip.Color(0, 0, 0), 50);
              break;
      case 3: colorWipe(strip.Color(0, 0, (uint8_t)(255*(brightness/255))), 50);  // Blue
              colorWipe(strip.Color(0, 0, 0), 50);
              break;
      case 4: theaterChase(strip.Color((uint8_t)(127*(brightness/255)), (uint8_t)(127*(brightness/255)), (uint8_t)(255*(brightness/127))), 50); // White
              break;
      case 5: theaterChase(strip.Color((uint8_t)(127*(brightness/255)),   0,   0), 50); // Red
              break;
      case 6: theaterChase(strip.Color(  0,   0, (uint8_t)(127*(brightness/255))), 50); // Blue
              break;
      case 7: rainbow(20);
              break;
      case 8: rainbowCycle(20);
              break;
      case 9: theaterChaseRainbow(50);
              break;
      case 10: colorChaser(20);
              break;
      case 12: rainbow2(20);
              break;
      case 13: rainbowCycle2(20);
              break;
    }
  }
}

void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group 
}

ISR(PCINT0_vect) {
  bool newStateButton0 = digitalRead(10);
  bool newStateButton1 = digitalRead(11);
  bool newStateButton2 = digitalRead(12);
  if(newStateButton0 == LOW && oldStateButton0 == HIGH) {
    brightness = brightness / 2;
    if(brightness < 30) brightness = BRIGHTNESS_MAX;
    Serial.print("Brightness: ");
    Serial.println(brightness);
  }
  if(newStateButton1 == LOW && oldStateButton1 == HIGH) {
    currentAnimationMode = 0;
    micMode = !micMode;
    Serial.print("Microphone mode: ");
    Serial.println(micMode);
  }
  if(newStateButton2 == LOW && oldStateButton2 == HIGH) {
    currentAnimationMode++;
    if(micMode && currentAnimationMode >= MIC_ANIMATION_MODES) {
      currentAnimationMode = 0;
    } else if(currentAnimationMode >= ANIMATION_MODES) {
      currentAnimationMode = 0;
    }
    Serial.print("Animation mode: ");
    Serial.println(currentAnimationMode);
  }
  oldStateButton0 = newStateButton0;
  oldStateButton1 = newStateButton1;
  oldStateButton2 = newStateButton2;
  delay(100);
}

void equalizer() {
  for(uint8_t ii = 0; ii < 10; ii++) {
    int val = ((1024-analogRead(A0))/25)+1;
    Serial.println(val);
    for(uint16_t i = 0; i<strip.numPixels(); i++) {
      if(i >= val) {
        strip.setPixelColor(i,strip.Color(0,0,0));
      } else {
        strip.setPixelColor(i,strip.Color((uint8_t)(255*(brightness/255)),0,0));
      }
    }
    strip.show();
    delay(100);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void colorChaser(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & (uint8_t)(255*(brightness/255))));
    }
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbow2(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel2((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbowCycle2(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel2(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color( - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos* 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel2(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color( - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, ((uint8_t)(WheelPos * 3)*(brightness/255)), ((uint8_t)(255 - WheelPos * 3)*(brightness/255)));
  } else {
   WheelPos -= 170;
   return strip.Color(((uint8_t)(WheelPos * 3)*(brightness/255)), ((uint8_t)(255 - WheelPos * 3)*(brightness/255)), 0);
  }
}
