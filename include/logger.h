#ifndef LOGGER_H
#define LOGGER_H

// True once SD.begin() has succeeded.
extern bool sdCardReady;

// True while a logging session is active.
extern bool isLogging;

// Call once from setup(). Initializes SPI + SD card, runs the startup
// self-test, and shows an LCD/Serial warning if the card failed.
// Returns the same value it stores in sdCardReady.
bool initLogger();

// Call every loop() iteration. Internally throttles to one CSV row
// (plus a Serial debug printout) per LOG_INTERVAL_MS, and only writes
// while a logging session is active with a current GPS lock.
void updateLogger();

// Begin a new logging session: opens a new /LOGxxxx.CSV file, writes
// the CSV header, and resets trip telemetry. Shows a status message
// on the LCD indicating success or failure. Called by button.cpp.
void startLogging();

// Close out the current logging session and flush/close the file.
// Called by button.cpp.
void stopLogging();

#endif