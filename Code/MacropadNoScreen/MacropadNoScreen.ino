// Risto Innovates
// Macropad No Screen v1

#include <Arduino.h>
#include "Keyboard.h"
#include "Mouse.h"
#include <Encoder.h>
#include <ResponsiveAnalogRead.h>



#define ENCODER_BUTTON_PIN 4 // Encoder button connected to pin 4


const int potPin = A0;                // Analog pin for potentiometer
const int maxVolume = 16;             // Volume range (0-100)
int lastVolumeLevel = -1;              // Track last set volume level to avoid repeated commands

// Initialize ResponsiveAnalogRead on potPin with smooth filtering enabled
ResponsiveAnalogRead pot(potPin, true);


int encoderPinA = 2;
int encoderPinB = 3;
Encoder myEncoder(encoderPinA, encoderPinB); // Use the Encoder library for easy handling
int oldEncoderPos = 0;       // To track changes in encoder position

// Function Prototypes
void updateEncoder();

// MUX
// Outputs
int muxChannel1 = 4;
int muxChannel2 = 5;
int muxChannel3 = 6;

// Inputs
int muxSwitchesInput1 = 7;
int muxSwitchesInput2 = 8;

// MULTIPLEXER 1 BUTTONS - 8 CH
const int NUMBER_MUX_1_BUTTONS = 8;
bool muxButtons1CurrentState[NUMBER_MUX_1_BUTTONS] = {0};
bool muxButtons1PreviousState[NUMBER_MUX_1_BUTTONS] = {0};

unsigned long lastDebounceTimeMUX1[NUMBER_MUX_1_BUTTONS] = {0};
unsigned long debounceDelayMUX1 = 5;

// MULTIPLEXER 2 BUTTONS - 8 CH
const int NUMBER_MUX_2_BUTTONS = 8;
bool muxButtons2CurrentState[NUMBER_MUX_2_BUTTONS] = {0};
bool muxButtons2PreviousState[NUMBER_MUX_2_BUTTONS] = {0};

unsigned long lastDebounceTimeMUX2[NUMBER_MUX_2_BUTTONS] = {0};
unsigned long debounceDelayMUX2 = 5;

void setup() {
  Serial.begin(9600);

  Keyboard.begin();
  Mouse.begin(); 
  
  // Set up encoder pins
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  pinMode(muxChannel1, OUTPUT);
  pinMode(muxChannel2, OUTPUT);
  pinMode(muxChannel3, OUTPUT);
  digitalWrite(muxChannel1, LOW);
  digitalWrite(muxChannel2, LOW);
  digitalWrite(muxChannel3, LOW);

  pinMode(muxSwitchesInput1, INPUT_PULLUP); // Digital input for the first set of switches
  pinMode(muxSwitchesInput2, INPUT_PULLUP); // Digital input for the second set of switches

}

void loop() {
  updateEncoder();

  updateMUXButtons(muxSwitchesInput1, NUMBER_MUX_1_BUTTONS, muxButtons1CurrentState, muxButtons1PreviousState, lastDebounceTimeMUX2, debounceDelayMUX1, 0);
  updateMUXButtons(muxSwitchesInput2, NUMBER_MUX_2_BUTTONS, muxButtons2CurrentState, muxButtons2PreviousState, lastDebounceTimeMUX2, debounceDelayMUX2, 1);
  updatePot();
}

void updatePot() {
  pot.update();

  if (pot.hasChanged()) {
    int potValue = pot.getValue();
    int targetVolume = map(potValue, 1023, 0, 0, maxVolume);

    if (targetVolume != lastVolumeLevel) {
      int volumeChange = targetVolume - lastVolumeLevel;
      lastVolumeLevel = targetVolume;  // Update last known volume level
      
      if (volumeChange > 0) {
        // Increase volume
        for (int i = 0; i < volumeChange; i++) {
          Keyboard.press(KEY_F12);  // Volume up
          delay(10);                // Short delay for smoother adjustment
          Keyboard.release(KEY_F12);
        }
      } else if (volumeChange < 0) {
        // Decrease volume

        for (int i = 0; i < abs(volumeChange); i++) {
          Keyboard.press(KEY_F11);  // Volume down
          delay(10);                // Short delay for smoother adjustment
          Keyboard.release(KEY_F11);
        }
      }
    }
  }

  delay(5);  // Small delay for stability
}

void updateMUXButtons(int muxInputPin, int numberOfButtons, bool *currentState, bool *previousState, unsigned long *lastDebounceTime, unsigned long debounceDelay, int muxNumber) {
  for (int i = 0; i < numberOfButtons; i++) {
    int A = bitRead(i, 0);
    int B = bitRead(i, 1);
    int C = bitRead(i, 2);
    digitalWrite(muxChannel1, A);
    digitalWrite(muxChannel2, B);
    digitalWrite(muxChannel3, C);
    delay(1);
    currentState[i] = digitalRead(muxInputPin);

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (currentState[i] != previousState[i]) {
        lastDebounceTime[i] = millis();

        if (currentState[i] == LOW) {
          pressButton(muxNumber, i, 1); 
        } else {
          pressButton(muxNumber, i, 0);
        }
        previousState[i] = currentState[i];
      }
    }
  }
}


// Function to update encoder and handle navigation and selection
void updateEncoder() {
  int newEncoderPos = myEncoder.read() / 4; // Adjust encoder sensitivity if needed
  if (newEncoderPos != oldEncoderPos) {
    int direction = (newEncoderPos > oldEncoderPos) ? 1 : -1; // Determine rotation direction
    if (direction == 1) {
      // GO TO NEXT TAB
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_TAB);
      delay(2);
      Keyboard.release(KEY_LEFT_CTRL);
      Keyboard.release(KEY_TAB);
    } else {
      // GO TO PREVIOUS TAB
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press(KEY_TAB);
      delay(2);
      Keyboard.release(KEY_LEFT_CTRL);
      Keyboard.release(KEY_LEFT_SHIFT);
      Keyboard.release(KEY_TAB);
    }
    oldEncoderPos = newEncoderPos; // Update encoder position
  }
}


void pressButton(int muxNumber, int buttonNumber, int state) {
  
  if (state){
    Serial.print("Button: ");
    Serial.println(buttonNumber);
  }
  
  if (muxNumber == 0) {
    // mux 0
    switch (buttonNumber) {
      case 0:
        // Display Brightness Down
        if (state) {
          Keyboard.press('a');
        } else {
          Keyboard.release('a');
        }
        break;
      case 1:
        // Display Brightness Up
        if (state) {
          Keyboard.press('b');
        } else {
          Keyboard.release('b');
        }
        break;
      case 2:
        // Mission Control
        if (state) {
          Keyboard.press('c');
        } else {
          Keyboard.release('c');
        }
        break;
      case 3:
        // Media Rewind
        if (state) {
          Keyboard.press('d');
        } else {
          Keyboard.release('d');
        }
        break;
      case 4:
        // Media Play/Pause
        if (state) {
          Keyboard.press('e');
        } else {
          Keyboard.release('e');
        }
        break;
      case 5:
        // Media Fast Forward
        if (state) {
          Keyboard.press('f');
        } else {
          Keyboard.release('f');
        }
        break;
      case 6:
        //////
        if (state) {
          Keyboard.press('g');
        } else {
          Keyboard.release('g');
        }
        break;
      case 7:
        // /////
        if (state) {
          Keyboard.press('h');
        } else {
          Keyboard.release('h');
        }
        break;
    }
    
  } else {
    // mux 1
    switch (buttonNumber) {
      case 0:
        // Spotlight
        if (state) {
          Keyboard.press('i');
        } else {
          Keyboard.release('i');
        }
        break;
      case 1:
        // New Finder Window
        if (state) {
          Keyboard.press('j');
        } else {
          Keyboard.release('j');
        }
        break;
      case 2:
        // Safari
        if (state) {
          Keyboard.press('k');
        } else {
          Keyboard.release('k');
        }
        break;
      case 3:
        // Mail
        if (state) {
          Keyboard.press('l');
        } else {
          Keyboard.release('l');
        }
        break;
      case 4:
        // Text Edit
        if (state) {
          Keyboard.press('m');
        } else {
          Keyboard.release('m');
        }
        break;
      case 5:
        // Music
        if (state) {
          Keyboard.press('n');
        } else {
          Keyboard.release('n');
        }
        break;
      case 6:
        // PASTE
        if (state) {
          Keyboard.press('o');
        } else {
          Keyboard.release('o');
        }
        break;
      case 7:
        // OPEN NEW TAB
        if (state) {
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press('t');
        } else {
          Keyboard.release(KEY_LEFT_GUI);
          Keyboard.release('t');
        }
        break;
    }
  }
  
}
