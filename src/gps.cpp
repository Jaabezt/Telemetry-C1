#include "gps.h"
#include "config.h"
#include "telemetry.h"
#include "logger.h"
#include <HardwareSerial.h>

static HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

static unsigned long lastValidFixTime = 0;

void initGPS() {
    gpsSerial.begin(
        GPS_BAUD,
        SERIAL_8N1,
        GPS_RX_PIN,
        GPS_TX_PIN
    );
}
void updateGPS() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (gps.encode(c)) {
      if (gps.location.isValid() && gps.location.isUpdated()) {

        lastValidFixTime = millis();

        if (isLogging) {
          bool hdopValid = gps.hdop.isValid();
          updateDistance(gps.location.lat(), gps.location.lng(),
                         hdopValid, hdopValid ? gps.hdop.hdop() : 0.0);
        }
      }
    }
  }
}

bool isGpsLocked() {
  return gps.location.isValid() &&
         (millis() - lastValidFixTime < GPS_TIMEOUT_MS);
}


