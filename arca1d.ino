#include <Adafruit_NeoPixel.h>

// Define pins being used
#define LED_PIN 2
#define RIGHT_BUTTON_PIN 10
#define LEFT_BUTTON_PIN 3

// How many pixels in the LED strip
#define LED_COUNT 7

// Declare NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);

// Buttons
struct button {
  int pin;                         // The pin the button is connected to
  int state;                       // The HIGH/LOW state of the button.  Because we're using pinMode INPUT_PULLUP HIGH is unpressed, LOW is pressed.
  int lastState;                   // The last state of the button.  Used for debounce calculations.
  unsigned long lastDebounceTime;  // Last time debounce was checked
};
struct button leftButton = { LEFT_BUTTON_PIN, HIGH, HIGH, 0 };    // Initial leftButton state
struct button rightButton = { RIGHT_BUTTON_PIN, HIGH, HIGH, 0 };  // Initial rightButton state

// LED fields
const uint32_t playingColor = strip.Color(255, 0, 0);  // Red
const uint32_t failColor = strip.Color(255, 0, 0);     // Red
const uint32_t successColor = strip.Color(0, 255, 0);  // Green
const uint32_t offColor = strip.Color(0, 0, 0);        // Black
unsigned long ledPreviousMillis = 0;                   // Will store last time strip was updated
int ledSpeed = 400;                                    // How fast to move the LEDs
const int ledSpeedup = 25;                             // How quickly to increase the LED speed
int maxLedSpeed = 75;                                  // Max speed the LEDs will move
int ledDirection = 1;                                  // Which direction the stack is moving.  1 is right, -1 is left

// Menu fields
bool playStacker = false;
bool playPong = true;

// Blinking logic
const int numBlinks = 3;                // Number of times to blink on success/fail
int currentBlinkCount = 0;              // How many blinks we've done
bool blinkOn = false;                   // If we're in the on part of the blink or the off
unsigned long blinkPreviousMillis = 0;  // keeps track of the last time we switched between blink on and blink off

// Common fields used by all games
bool gameOver = false;  // GAME OVER :(
int level = 0;          // Current level

void setup() {
  Serial.begin(9600);
  Serial.println("starting");

  // analog input pin 0 is unconnected so random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(millis());

  // Set all unused pins to INPUT_PULLUP to conserve power consumption
  for (int i = 2; i <= 19; i++) {
    if (i != LED_PIN || i != RIGHT_BUTTON_PIN || i != LEFT_BUTTON_PIN || i != 13) {  // ignore the pins we define and the internal LED pin
      pinMode(i, INPUT_PULLUP);
    }
  }

  // Setup buttons.  Using INPUT_PULLUP so we don't need an extra resistor
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);

  // Setup LED strip
  strip.begin();            // Initialize NeoPixel strip object
  strip.show();             // Turn off all pixels
  strip.setBrightness(10);  // Set brightness of LEDs (max = 255)
}

void loop() {
  if (playStacker) {
    stackerRun();
  } else if (playPong) {
    pongRun();
  } else {
    //Menu logic
  }

  strip.show();
}
