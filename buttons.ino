// Button fields
unsigned long debounceDelay = 50;  // The debounce time; increase if the output flickers

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