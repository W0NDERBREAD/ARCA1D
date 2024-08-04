// stacker logic fields. Soring in struct so variable names can be reused.

struct stackerFields {
  bool pressedAnimating;  // Stores if the blinking success/fail animation is playing
  bool pressedSuccess;    // When the button is pressed was it in the center or not
  int stackSize;          // Size of the stack
};
struct stackerFields stacker = {
  pressedAnimating: false,
  pressedSuccess: false,
  stackSize: 3,
};

void stackerRun() {
  unsigned long currentMillis = millis();

  if (!gameOver) {
    // Button was pressed. Check for success, start animation, and increase speed
    if (!stacker.pressedAnimating && (btnPressed(leftButton) || btnPressed(rightButton))) {
      stacker.pressedAnimating = true;
      if (ledPixelPos == LED_COUNT - (ledPixelPos + stacker.stackSize) || ledPixelPos == LED_COUNT - (ledPixelPos + stacker.stackSize) - 1) {
       stacker. pressedSuccess = true;
      } else {
        stacker.pressedSuccess = false;
      }
      ledSpeed = ledSpeed - ledSpeedup;
      if (ledSpeed < maxLedSpeed) {
        ledSpeed = maxLedSpeed;
      }
    }

    // No button press and not animating.  Move stack if we've waited the speed amount
    if (!stacker.pressedAnimating && currentMillis - ledPreviousMillis >= ledSpeed) {
      // save the last time we moved the LED to now
      ledPreviousMillis = currentMillis;
      ledPixelPos = ledPixelPos + ledDirection;

      // If the end of the stack is past the last LED, switch directions
      if (ledPixelPos + stacker.stackSize - 1 >= LED_COUNT) {
        ledDirection = -1;
        ledPixelPos = ledPixelPos - 2;  // The last shown position had the stack all the way to the left so the next position is actually one to the left
      }

      // If the start of the stack is past the first LED, switch directions
      if (ledPixelPos < 0) {
        ledDirection = 1;
        ledPixelPos = 1;  // The last shown position started at 0, so move one to the right
      }

      // Update the LEDs with the new stack position. LEDs aren't updated until show() is called so turning all off and then on won't flicker
      strip.fill(0, 0, LED_COUNT);                       // Clear all LEDs so we don't leave a trail
      strip.fill(playingColor, ledPixelPos, stacker.stackSize);  // Show the new stack
    }

    // Animating a button press. Use the LED speed so even the success/fail blinking adds stress
    if (stacker.pressedAnimating && currentMillis - blinkPreviousMillis >= ledSpeed) {
      int start = (LED_COUNT - stacker.stackSize) / 2;  // The position where the stack should start
      int end = start + stacker.stackSize - 1;          // The position where the stack should end
      currentBlinkCount++;
      blinkPreviousMillis = currentMillis;
      blinkOn = !blinkOn;
      if (blinkOn) {
        if (stacker.pressedSuccess) {
          strip.fill(successColor, ledPixelPos, stacker.stackSize);
        } else {
          strip.fill(failColor, ledPixelPos, stacker.stackSize);
        }
      } else {
        if (stacker.pressedSuccess) {
          strip.fill(0, 0, LED_COUNT);
        } else {
          // Always turn on the whole stack.  Turn off any LEDs outside the start/end position so the incorrect LEDs blink.
          strip.fill(failColor, ledPixelPos, stacker.stackSize);
          strip.fill(0, 0, start);
          strip.fill(0, end + 1, LED_COUNT - end - 1);
        }
      }

      // Done animating.  Reset animation values and calculate new stack size
      if (currentBlinkCount >= numBlinks * 2 + 1) {
        stacker.pressedAnimating = false;
        currentBlinkCount = 0;
        blinkOn = false;

        // New stack size is existing size - missed left - missed right. Since we're handling the left and right at
        // the same time, a negative value means the stack was off in the opposite direction so ignote it by max()'ing with 0
        stacker.stackSize = stacker.stackSize - max(0, start - ledPixelPos) - max(0, ledPixelPos + stacker.stackSize - end - 1);
        gameOver = stacker.stackSize <= 0;
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
          strip.fill(failColor, 0, stacker.stackSize);
        } else {
          strip.fill(0, 0, stacker.stackSize);
        }
      }
    } else {
      // Done blinking.  Display the level score in binary
      for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(LED_COUNT - 1 - i, (bitRead(level, i) == 1) ? failColor : offColor);
      }

      // If either button is press reset all fields to original values to restart the game
      if (btnPressed(leftButton) || btnPressed(rightButton)) {
        ledSpeed = 400;
        ledPixelPos = -1;
        ledDirection = 1;
        stacker.stackSize = 3;
        stacker.pressedAnimating = false;
        stacker.pressedSuccess = false;
        currentBlinkCount = 0;
        blinkOn = false;
        blinkPreviousMillis = 0;
        gameOver = false;
        level = 0;
      }
    }
  }
}