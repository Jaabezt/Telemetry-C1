#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void initDisplay();

// Call every loop() iteration. Internally throttles to one redraw per
// DISPLAY_INTERVAL_MS. Priority order:
//   1. Temporary confirmation message (button feedback)
//   2. "No GPS signal" fallback
//   3. Normal telemetry screen: speed / distance
// Reads current logging/SD state from logger.h.
void updateDisplay();

// Show a temporary 2-line confirmation message (e.g. button feedback)
// for STATUS_MESSAGE_DURATION_MS, after which the normal screen
// resumes automatically.
void showStatusMessage(const String &line1, const String &line2);

// Show the "Acquiring GPS..." boot message, with a second line that
// reflects whether the SD card initialized successfully.
void showBootMessage(bool sdReady);

// Format seconds as mm:ss for compact LCD display.
String formatElapsed(double totalSeconds);

#endif