#ifndef TELEMETRY_H
#define TELEMETRY_H

// Running total distance for the current logging session (km)
extern double totalDistanceKm;

// Feed a new GPS fix (lat/lng) into the distance accumulator.
// Only call this while a logging session is active.
void updateDistance(double currentLat, double currentLng, bool hdopValid, double hdop);

// Reset trip telemetry (called when a new logging session starts).
void resetTelemetry();

// Moving-average speed smoother. Feed it the raw GPS speed (km/h), get back a jitter-reduced value.
double getSmoothedSpeed(double newRawSpeed);

#endif