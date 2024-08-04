#include <Adafruit_NeoPixel.h>

// Define pins being used
#define LED_PIN 9
#define RIGHT_BUTTON_PIN 6
#define LEFT_BUTTON_PIN 2

// How many pixels in the LED strip
#define LED_COUNT 7

// Declare NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);

// Button fields
unsigned long debounceDelay = 50;  // The debounce time; increase if the output flickers
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
int ledPixelPos = -1;                                  // Keeps track of the current position of the stack
int ledDirection = 1;                                  // Which direction the stack is moving.  1 is right, -1 is left
int ledStackSize = 3;                                  // How many LEDs in the moving stack

// stacker logic fields
bool pressedAnimating = false;          // Stores if the blinking success/fail animation is playing
bool pressedSuccess = false;            // When the button is pressed was it in the center or not
const int numBlinks = 3;                // Number of times to blink on success/fail
int currentBlinkCount = 0;              // How many blinks we've done
bool blinkOn = false;                   // If we're in the on part of the blink or the off
unsigned long blinkPreviousMillis = 0;  // keeps track of the last time we switched between blink on and blink off
bool gameOver = false;                  // GAME OVER :(
int level = 0;                          // Current level

void setup() {
  Serial.begin(9600);
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

  unsigned long currentMillis = millis();

  if (!gameOver) {
    // Button was pressed. Check for success, start animation, and increase speed
    if (!pressedAnimating && (btnPressed(leftButton) || btnPressed(rightButton))) {
      pressedAnimating = true;
      if (ledPixelPos == LED_COUNT - (ledPixelPos + ledStackSize) || ledPixelPos == LED_COUNT - (ledPixelPos + ledStackSize) - 1) {
        pressedSuccess = true;
      } else {
        pressedSuccess = false;
      }
      ledSpeed = ledSpeed - ledSpeedup;
      if (ledSpeed < maxLedSpeed) {
        ledSpeed = maxLedSpeed;
      }
    }

    // No button press and not animating.  Move stack if we've waited the speed amount
    if (!pressedAnimating && currentMillis - ledPreviousMillis >= ledSpeed) {
      // save the last time we moved the LED to now
      ledPreviousMillis = currentMillis;
      ledPixelPos = ledPixelPos + ledDirection;

      // If the end of the stack is past the last LED, switch directions
      if (ledPixelPos + ledStackSize - 1 >= LED_COUNT) {
        ledDirection = -1;
        ledPixelPos = ledPixelPos - 2;  // The last shown position had the stack all the way to the left so the next position is actually one to the left
      }

      // If the start of the stack is past the first LED, switch directions
      if (ledPixelPos < 0) {
        ledDirection = 1;
        ledPixelPos = 1;  // The last shown position started at 0, so move one to the right
      }

      // Update the LEDs with the new stack position. LEDs aren't updated until show() is called so turning all off and then on won't flicker
      strip.fill(0, 0, LED_COUNT);                          // Clear all LEDs so we don't leave a trail
      strip.fill(playingColor, ledPixelPos, ledStackSize);  // Show the new stack
    }

    // Animating a button press. Use the LED speed so even the success/fail blinking adds stress
    if (pressedAnimating && currentMillis - blinkPreviousMillis >= ledSpeed) {
      int start = (LED_COUNT - ledStackSize) / 2;  // The position where the stack should start
      int end = start + ledStackSize - 1;          // The position where the stack should end
      currentBlinkCount++;
      blinkPreviousMillis = currentMillis;
      blinkOn = !blinkOn;
      if (blinkOn) {
        if (pressedSuccess) {
          strip.fill(successColor, ledPixelPos, ledStackSize);
        } else {
          strip.fill(failColor, ledPixelPos, ledStackSize);
        }
      } else {
        if (pressedSuccess) {
          strip.fill(0, 0, LED_COUNT);
        } else {
          // Always turn on the whole stack.  Turn off any LEDs outside the start/end position so the incorrect LEDs blink.
          strip.fill(failColor, ledPixelPos, ledStackSize);
          strip.fill(0, 0, start);
          strip.fill(0, end + 1, LED_COUNT - end - 1);
        }
      }

      // Done animating.  Reset animation values and calculate new stack size
      if (currentBlinkCount >= numBlinks * 2 + 1) {
        pressedAnimating = false;
        currentBlinkCount = 0;
        blinkOn = false;

        // New stack size is existing size - missed left - missed right. Since we're handling the left and right at
        // the same time, a negative value means the stack was off in the opposite direction so ignote it by max()'ing with 0
        ledStackSize = ledStackSize - max(0, start - ledPixelPos) - max(0, ledPixelPos + ledStackSize - end - 1);
        gameOver = ledStackSize <= 0;
        level++;
      }
    }
  } else {
    // Game over, blink all the lights
    if (currentBlinkCount < numBlinks * 2 + 1) {
        if (currentMillis - blinkPreviousMillis >= ledSpeed) {
          currentBlinkCount++;
          blinkPreviousMillis = currentMillis;
          blinkOn = !blinkOn;
          if (blinkOn) {
            strip.fill(failColor, 0, ledStackSize);
          } else {
            strip.fill(0, 0, ledStackSize);
          }
        }
      }
    else {
      // Done blinking.  Display the level score in binary
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(LED_COUNT - 1 - i, (bitRead(level, i) == 1) ? failColor : offColor);
      }

      // If either button is press reset all fields to original values to restart the game
      if (btnPressed(leftButton) || btnPressed(rightButton)) {
        ledSpeed = 400;
        ledPixelPos = -1;
        ledDirection = 1;
        ledStackSize = 3;
        pressedAnimating = false;
        pressedSuccess = false;
        currentBlinkCount = 0;
        blinkOn = false;
        blinkPreviousMillis = 0;
        gameOver = false;
        level = 0;
      }
    }
  }

  strip.show();
}


bool btnPressed(struct button& b) {
  // Read the state of the switch into a local variable
  int reading = digitalRead(b.pin);
  bool pressed = false;

  // check to see if you just pressed the button
  // (i.e. the input went from HIGH to LOW), and you've waited long enough
  // since the last press to ignore any noise

  // If the switch changed, due to noise or pressing
  if (reading != b.lastState) {
    // reset the debouncing timer
    b.lastDebounceTime = millis();
  }

  if ((millis() - b.lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != b.state) {
      pressed = true;
      b.state = reading;
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  b.lastState = reading;
  return pressed && b.state == LOW;
}
