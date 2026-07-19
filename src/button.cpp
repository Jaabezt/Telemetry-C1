#include "button.h"
#include "config.h"
#include "logger.h"
#include "Arduino.h"

static bool lastButtonReading = HIGH;
static bool stableButtonState = HIGH;
static unsigned long lastButtonChangeTime = 0;

void initButton() {
  pinMode(LOG_BUTTON_PIN, INPUT_PULLUP);
}

void updateButton() {
  bool reading = digitalRead(LOG_BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastButtonChangeTime = millis();
  }

  if ((millis() - lastButtonChangeTime) > DEBOUNCE_MS) {
    if (reading != stableButtonState) {
      stableButtonState = reading;
      if (stableButtonState == LOW) {   // press detected
        if (isLogging) {
          stopLogging();
        } else {
          startLogging();
        }
      }
    }
  }

  lastButtonReading = reading;
}