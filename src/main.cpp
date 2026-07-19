#include "gps.h"
#include "display.h"
#include "logger.h"
#include "button.h"

void setup() {
  Serial.begin(115200);
  delay(100);

  initGPS();
  initButton();
  initDisplay();
  initLogger();
}

void loop() {
  updateGPS();
  updateButton();
  updateLogger();
  updateDisplay();
}