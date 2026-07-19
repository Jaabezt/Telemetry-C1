#ifndef GPS_H
#define GPS_H

#include <TinyGPS++.h>

void initGPS();

// Call every loop() iteration. Reads any available bytes from the GPS
// module and feeds them into the parser. Whenever a fresh, valid fix
// arrives, updates the internal "last valid fix" timestamp and, if a
// logging session is active (per logger.h's isLogging), updates the
// distance telemetry.
void updateGPS();

// True if we have a valid location fix that hasn't gone stale
// (received within GPS_TIMEOUT_MS).
bool isGpsLocked();

extern TinyGPSPlus gps;

#endif