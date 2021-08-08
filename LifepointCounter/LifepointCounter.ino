/*
   YuGiOh Lifepoint Counter
   Written for a Pro Micro device

   The following hardware is needed:
    6 buttons
    1 Adafruit SSD1306 128X64 OLED display
    1 Piezo-electric buzzer

   Copyright Blattoid 2021
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Images.h"

#define SERIAL_DEBUG //Uncomment to enable serial debugging output

// Pin definitions
#define SPEAKER 6 // Beeper speaker
#define LEDACTIVE 17 // Activity light, uses the RX LED
#define LEDADD 30 // Indicates addition mode, uses the TX LED
#define BTN_PLUSMINUS 14 // Toggle between Add/Subtract mode
#define BTN_THOUSAND 15
#define BTN_FIVEHUNDRED 18
#define BTN_ONEHUNDRED 19
#define BTN_FIFTY 20
#define BTN_TEN 21

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_ADDRESS 0x3D // < See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Define a struct for storing previous button states
// This is used to filter for Pulldown events on I/O.
struct ButtonStates {
  bool PlusMinus = true;
  bool Thousand = true;
  bool FiveHund = true;
  bool OneHund = true;
  bool Fifty = true;
  bool Ten = true;
};
ButtonStates BtnPrevious;

// Initialise variables
bool Addition = false;
bool FastMatch = false;
int16_t Lifepoints = 8000; // Must be a signed integer to facilitate detection of a Death State

void DisplayFullRender() {
  // Draws every element, including ones that do not have the potential to change

  // Clear screen buffer so elements don't overlap the old screen contents
  display.clearDisplay();

  // Left element
  display.drawBitmap(
    0, 0,
    LeftGraphic,
    48, 64, 1);

  // Right chevron - the loops generate a 3px width
  for (uint8_t i = 0; i < 3; i++)
    display.drawLine(116 + i, 0, // Top half
                     125 + i, 31,
                     1);
  for (uint8_t i = 0; i < 3; i++)
    display.drawLine(116 + i, 63, // Bottom half
                     125 + i, 32,
                     1);
  // Bottom line
  display.drawFastHLine(28, 63,
                        88, 1);

  // Next, draw the dynamic components.
  DisplayUpdate();
}
uint16_t DisplayUpdate() {
  // Keep track of start time
  uint16_t startTime = millis();

  /* OLD UI CODE
    // Health value on top
    display.setCursor(15, 0);
    display.print(Lifepoints);

    // Health bar on bottom
    int16_t maxHealth = FastMatch ? 4000 : 8000;
    uint8_t fillWidth = map( // Calculate width of bar
                      min(Lifepoints, maxHealth), // Upper constraint
                      0, maxHealth,
                      0, 117);
    display.drawRect( // Border
    4, 38,
    119, 20,
    1);
    display.fillRect( // Fill
    5, 39,
    fillWidth, 18,
    1);
  */

  // Font size 2 for "LP", 3 for readout
  // We do not have the luxury of clearDisplay, so we must manually clear the areas we will be working with

  // Clear top bar
  display.fillRect(42, 0,
                   /*?*/74, 21,
                   0);
  // Top bar
  uint8_t x = min(109, // Calculate where the bar's right edge and where the triangle should go
                  map(Lifepoints,
                      0, FastMatch ? 4000 : 8000,
                      42, 109));
  display.fillRect(42, 0,
                   x - 42, 21,
                   1);
  // Top bar triangle
  display.fillTriangle(x, 0,
                       x + 6, 10,
                       x, 19,
                       1);

  // Top bar "LP"
  display.setCursor(42, 7);
  display.setTextSize(2);
  display.setTextColor(0); // Cut out part of the bar
  display.print("LP");
  display.setTextColor(1); // Reset to 1

  // Lifepoint readout
  display.fillRect(45, 40,
                   69, 21, 0);
  display.setCursor(45, 40);
  display.setTextSize(3);
  display.print(Lifepoints);

  // Update display
  display.display();
  return millis() - startTime; // Return execution time of function
}

uint16_t DisplayDeath(bool Visible) {
  // Keep track of start time
  uint16_t startTime = millis();

  // Clear screen buffer so elements don't overlap old ones
  display.clearDisplay();

  if (Visible) {
    display.setCursor(15, 0);
    display.println("GAME");
    display.print(" OVER"); // offset weirdness
  }

  // Update display
  display.display();

  return millis() - startTime; // Return execution time of function to maintain a consistent frame rate
}

void setup() {
  // Initialise serial output
  Serial.begin(9600);
  Serial.println("Yugioh Lifepoint Counter v1.4");

  // Initialise pins
  pinMode(SPEAKER,         OUTPUT);
  pinMode(LEDACTIVE,       OUTPUT);
  pinMode(LEDADD,          OUTPUT);
  digitalWrite(LEDACTIVE,  HIGH);
  // NOTE: Since PULLUP is being used, all button readings must be inverted.
  pinMode(BTN_PLUSMINUS,   INPUT_PULLUP);
  pinMode(BTN_THOUSAND,    INPUT_PULLUP);
  pinMode(BTN_FIVEHUNDRED, INPUT_PULLUP);
  pinMode(BTN_ONEHUNDRED,  INPUT_PULLUP);
  pinMode(BTN_FIFTY,       INPUT_PULLUP);
  pinMode(BTN_TEN,         INPUT_PULLUP);

  // Initialise display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("OLED display initialisation failed!"));
    noTone(SPEAKER);
    for (;;)
    {
      // Error tone and flash LED
      tone(SPEAKER, 100, 500);
      digitalWrite(LEDACTIVE, LOW);
      delay(500);
      digitalWrite(LEDACTIVE, HIGH);
      delay(500);
    }
  }
  display.setTextColor(WHITE);

  // Display splash screen
  display.clearDisplay();
  display.drawBitmap(
    0, 0,
    Splashscreen,
    SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();



  // Startup tune
  tone(SPEAKER, 500, 100);
  delay(100);
  tone(SPEAKER, 1000, 100);
  delay(100);
  tone(SPEAKER, 1500, 100);
  delay(200);

  // Show the splash screen for a bit, while periodically checking if the PlusMinus button is pressed.
  for (uint8_t i = 0; i < 10; i++) {
    // Short battle mode
    if (!digitalRead(BTN_PLUSMINUS) && !FastMatch) {
      BtnPrevious.PlusMinus = false; // Remember the button was initially held
      FastMatch = true;
      tone(SPEAKER, 500, 200);
    }
    delay(100);
  }
  if (FastMatch)  {
    Lifepoints = 4000;
  }

  DisplayFullRender(); // Render the initial screen state
}

void loop() {
  uint16_t Amount = 0; // Reset Amount to 0

  // Ensure LEDs have correct state
  digitalWrite(LEDACTIVE, HIGH); // Inverted, turn it off.
  digitalWrite(LEDADD, !Addition); // Inverted, turn it on in addition mode

  // Read all button states, while applying filtering to only trigger on pull down events.
  bool state = !digitalRead(BTN_PLUSMINUS);
  if (state && !BtnPrevious.PlusMinus) Addition ^= true;
  BtnPrevious.PlusMinus = state;
  state = !digitalRead(BTN_THOUSAND);
  if (state && !BtnPrevious.Thousand) Amount += 1000;
  BtnPrevious.Thousand = state;
  state = !digitalRead(BTN_FIVEHUNDRED);
  if (state && !BtnPrevious.FiveHund) Amount += 500;
  BtnPrevious.FiveHund = state;
  state = !digitalRead(BTN_ONEHUNDRED);
  if (state && !BtnPrevious.OneHund) Amount += 100;
  BtnPrevious.OneHund = state;
  state = !digitalRead(BTN_FIFTY);
  if (state && !BtnPrevious.Fifty) Amount += 50;
  BtnPrevious.Fifty = state;
  state = !digitalRead(BTN_TEN);
  if (state && !BtnPrevious.Ten) Amount += 10;
  BtnPrevious.Ten = state;

  // Damage loop
  while (Amount != 0) {
    // Update the counters in steps of 10
    Lifepoints += Addition ? 10 : -10;
    Amount -= 10;

    // Lifepoint caps
    if (Lifepoints <= 0) {
      Lifepoints = 0;

      DisplayUpdate();

      // Death tune
      for (uint16_t freq = 2000; freq >= 100; freq -= 100)
      {
        // Descending short beeps
        tone(SPEAKER, freq);
        delay(30);
      }
      noTone(SPEAKER); //Reset it
      display.setTextSize(4);
      for (uint16_t i = 0; i < 5; i++)
      {
        // Longer 3 beeps of same low frequency
        DisplayDeath(false); // Screen death flash off
        delay(300);
        tone(SPEAKER, 100);
        DisplayDeath(true); // Screen death flash on
        delay(300);
        noTone(SPEAKER);
      }
      // Easiest way to proceed after a Game Over state is to reset to normal
      Lifepoints = FastMatch ? 4000 : 8000;
      DisplayFullRender();
      break;
    }

    else if (Lifepoints > 9990) {
      Lifepoints = 9990;
      // Error beep
      tone(SPEAKER, 100, 50);
      delay(60);
      break;
    }

    int16_t deltaT = DisplayUpdate();

#ifdef SERIAL_DEBUG
    Serial.println();
    Serial.println(Addition ? "true" : "false");
    Serial.println(Amount);
    Serial.println(Lifepoints);
#endif

    // Beep
    digitalWrite(LEDACTIVE, LOW);
    tone(SPEAKER, Addition ? 3000 : 2000, 20);
    delay(max(0, 30 - deltaT));
    digitalWrite(LEDACTIVE, HIGH);
  }
}
