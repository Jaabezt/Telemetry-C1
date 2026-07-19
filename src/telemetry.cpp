#include "telemetry.h"
#include "config.h"
#include <TinyGPS++.h>

double totalDistanceKm = 0.0;

static double lastLat = 0.0, lastLng = 0.0;
static bool hasPreviousFix = false;

static double speedSamples[SPEED_SAMPLE_COUNT] = {0};
static int speedSampleIndex = 0;
static bool speedBufferFilled = false;

// Accumulate distance using consecutive valid GPS fixes.
void updateDistance(double currentLat, double currentLng, bool hdopValid, double hdop) {
  bool fixIsTrusted = (!hdopValid) || (hdop <= MAX_TRUSTED_HDOP);

  if (hasPreviousFix && fixIsTrusted) {
    double distanceMeters = TinyGPSPlus::distanceBetween(
        lastLat, lastLng, currentLat, currentLng);

    if (distanceMeters >= MIN_MOVEMENT_METERS) {
      totalDistanceKm += distanceMeters / 1000.0;
    }
  }

  lastLat = currentLat;
  lastLng = currentLng;
  hasPreviousFix = true;
}

void resetTelemetry() {
  totalDistanceKm = 0.0;
  hasPreviousFix = false;
}

// Moving average speed smoother.
double getSmoothedSpeed(double newRawSpeed) {
  speedSamples[speedSampleIndex] = newRawSpeed;
  speedSampleIndex = (speedSampleIndex + 1) % SPEED_SAMPLE_COUNT;
  if (speedSampleIndex == 0) speedBufferFilled = true;

  int count = speedBufferFilled ? SPEED_SAMPLE_COUNT : speedSampleIndex;
  if (count == 0) return 0.0;

  double sum = 0.0;
  for (int i = 0; i < count; i++) sum += speedSamples[i];
  return sum / count;
}