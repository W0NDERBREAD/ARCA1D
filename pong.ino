// pong logic fields. Storing in struct so variable names can be reused.
struct pongFields {
  bool pressedAnimating;  // Stores if the blinking success/fail animation is playing
  bool scoreAnimating;
  int p1Score;  // Player 1 score
  bool p1Pressed;
  bool p1CurvePressed;
  uint32_t p1Color;
  int p2Score;  // Player 2 score
  bool p2Pressed;
  bool p2CurvePressed;
  uint32_t p2Color;
  bool curving;
  int curveStep;
  int ballPos;    // Keeps track of the current position of the ball
  int direction;  // The direction of the ball.  1 is right, -1 is left
};
struct pongFields pong = {
  pressedAnimating: false,
  scoreAnimating: false,
  p1Score: 0,
  p1Pressed: false,
  p1CurvePressed: false,
  p1Color: strip.Color(0, 0, 255),
  p2Score: 0,
  p2Pressed: false,
  p2CurvePressed: false,
  p2Color: strip.Color(255, 0, 255),
  curving: false,
  curveStep: 10,
  ballPos: 3,
  direction: randomDirection()
};

void pongRun() {
  unsigned long currentMillis = millis();

  if (!gameOver) {
    // Button was pressed. Check for success, start animation, and increase speed
    if (!pong.pressedAnimating && !pong.scoreAnimating) {
      if (btnPressed(leftButton) && !pong.p1Pressed) {
        pong.p1Pressed = true;
        if (pong.ballPos == 0) {
          pong.p1Pressed = false;
          pong.direction *= -1;
          ledSpeed = ledSpeed - ledSpeedup;
          if (ledSpeed < maxLedSpeed) {
            ledSpeed = maxLedSpeed;
          }
          level += 1;
        }
      }

      if (btnPressed(rightButton) && !pong.p2Pressed) {
        pong.p2Pressed = true;
        if (pong.ballPos == LED_COUNT - 1) {
          pong.p2Pressed = false;
          pong.direction *= -1;
          ledSpeed = ledSpeed - ledSpeedup;
          if (ledSpeed < maxLedSpeed) {
            ledSpeed = maxLedSpeed;
          }
          level += 1;
        }
      }
    }

    if (!pong.pressedAnimating && !pong.scoreAnimating && currentMillis - ledPreviousMillis >= ledSpeed) {
      // save the last time we moved the ball to now
      ledPreviousMillis = currentMillis;

      pong.ballPos += pong.direction;

      strip.fill(0, 0, LED_COUNT);                      // Clear all LEDs so we don't leave a trail
      strip.setPixelColor(pong.ballPos, playingColor);  // Show the new stack

      // If the ball is past the last LED, p1 scores
      if (pong.ballPos >= LED_COUNT) {
        pong.p1Score += 1;
        pong.pressedAnimating = true;
        ledSpeed = 400;
      } else if (pong.ballPos < 0) {
        // If the ball is past the first LED, p2 scores
        pong.p2Score += 1;
        pong.pressedAnimating = true;
        ledSpeed = 400;
      }
    }

    // Animating a button press
    if (pong.pressedAnimating && currentMillis - blinkPreviousMillis >= ledSpeed) {
      currentBlinkCount++;
      blinkPreviousMillis = currentMillis;
      blinkOn = !blinkOn;
      if (blinkOn) {
        if (pong.ballPos >= LED_COUNT) {
          strip.setPixelColor(LED_COUNT - 1, failColor);
        } else {
          strip.setPixelColor(0, failColor);
        }
      } else {
        strip.fill(0, 0, LED_COUNT);
      }

      // Done animating.  Reset animation values and start score
      if (currentBlinkCount >= numBlinks * 2 + 1) {
        pong.scoreAnimating = true;
        currentBlinkCount = 0;
        blinkOn = false;

        pong.pressedAnimating = false;
      }
    }

    // Animating the score
    if (pong.scoreAnimating && currentMillis - blinkPreviousMillis >= ledSpeed) {
      currentBlinkCount++;
      blinkPreviousMillis = currentMillis;
      blinkOn = !blinkOn;
      if (blinkOn) {
        strip.clear();
        if (pong.p1Score > 0) {
          strip.fill(pong.p1Color, 0, pong.p1Score);
        }
        if (pong.p2Score) {
          strip.fill(pong.p2Color, LED_COUNT - pong.p2Score);
        }
      } else {
        strip.clear();
      }

      // Done animating.  Reset animation values and reset values to start new round.  If either player has > 4 points then game over
      if (currentBlinkCount >= numBlinks * 2 + 1) {
        pong.scoreAnimating = false;
        pong.pressedAnimating = false;
        currentBlinkCount = 0;
        blinkOn = false;

        pong.p1Pressed = false;
        pong.p2Pressed = false;
        pong.ballPos = 3;
        pong.direction = randomDirection();

        if (pong.p1Score >= 4 || pong.p2Score >= 4) {
          gameOver = true;
        }
      }
    }
  } else {
    const uint32_t winnerColor = pong.p1Score >= 4 ? pong.p1Color : pong.p2Color;
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(LED_COUNT - 1 - i, (bitRead(level, i) == 1) ? winnerColor : offColor);
    }

    // If either button is press reset all fields to original values to restart the game
    if (btnPressed(leftButton) || btnPressed(rightButton)) {
      gameOver = false;
      level = 0;
      pong.p1Score = 0;
      pong.p2Score = 0;
    }
  }
}

// Returns a random direction
int randomDirection() {
  if (random(2) == 0) {
    return -1;
  }
  return 1;
}