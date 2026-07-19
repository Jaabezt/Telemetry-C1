#include "logger.h"
#include "config.h"
#include "gps.h"
#include "telemetry.h"
#include "display.h"
#include <SPI.h>
#include <SD.h>
#include <Preferences.h>

bool sdCardReady = false;
bool isLogging = false;

static Preferences prefs;
static File logFile;
static char currentLogFileName[24] = "";

static unsigned long logStartMillis = 0;
static unsigned long lastLogWriteTime = 0;

static void writeLogRow();
static void printDebugRow(double smoothedSpeed);

bool initLogger() {
  prefs.begin("gpslog", false);

  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

  Serial.println("================================");
  Serial.println("Initializing SD card...");
  Serial.printf("CS Pin   : %d\n", SD_CS_PIN);
  Serial.printf("MOSI Pin : %d\n", SD_MOSI_PIN);
  Serial.printf("MISO Pin : %d\n", SD_MISO_PIN);
  Serial.printf("SCK Pin  : %d\n", SD_SCK_PIN);

  sdCardReady = SD.begin(SD_CS_PIN);

  if (!sdCardReady) {
    Serial.println("SD.begin() FAILED");
  } else {
    Serial.println("SD.begin() SUCCESS");

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("Card Size: %llu MB\n", cardSize);

    uint64_t totalBytes = SD.totalBytes() / (1024 * 1024);
    uint64_t usedBytes = SD.usedBytes() / (1024 * 1024);

    Serial.printf("Total Space : %llu MB\n", totalBytes);
    Serial.printf("Used Space  : %llu MB\n", usedBytes);

    // For testing SD Card.
    File test = SD.open("/TEST.TXT", FILE_WRITE);

    if (test) {
      test.println("ESP32 SD card test successful!");
      test.close();
      Serial.println("TEST.TXT written successfully.");
    } else {
      Serial.println("Failed to create TEST.TXT");
    }
  }

  Serial.println("================================");

  if (!sdCardReady) {
    // Make this impossible to miss -- the whole point of this device
    // is to log data, so a failed SD card is a critical fault the
    // driver/operator needs to know about immediately.
    showStatusMessage("SD CARD ERROR", "Check card/wires");
    Serial.println("SD card init FAILED. Check wiring and formatting (FAT32).");
    delay(3000);
  } else {
    Serial.println("SD card ready.");
  }

  showBootMessage(sdCardReady);
  Serial.println("System booted. Waiting for GPS fix...");
  return sdCardReady;
}

// Start a new log file and write the CSV header.
void startLogging() {
  if (!sdCardReady) {
    Serial.println("Cannot start logging: SD card not ready.");
    showStatusMessage("SD CARD ERROR", "Cannot record");
    return;
  }

  unsigned int logNumber = prefs.getUInt("logNum", 0) + 1;
  prefs.putUInt("logNum", logNumber);

  snprintf(currentLogFileName, sizeof(currentLogFileName), "/LOG%04u.CSV", logNumber);

  logFile = SD.open(currentLogFileName, FILE_WRITE);
  if (!logFile) {
    Serial.println("Failed to create log file!");
    showStatusMessage("FILE ERROR", "Cannot record");
    return;
  }

  logFile.println("Elapsed_s,Timestamp_UTC,Latitude,Longitude,Speed_kmh,Distance_km,Satellites,HDOP");
  logFile.flush();

  // Start recording
  isLogging = true;

  // Reset trip data
  resetTelemetry();

  // Reset timers
  logStartMillis = millis();
  lastLogWriteTime = millis();

  // Serial output
  Serial.println("==============================");
  Serial.print("Recording Started: ");
  Serial.println(currentLogFileName);
  Serial.println("==============================");

  // LCD status message
  showStatusMessage("RECORDING", "STARTED");
}

// Close the current log file safely.
void stopLogging() {
  if (logFile) {
    logFile.flush();
    logFile.close();
  }

  isLogging = false;

  Serial.println("==============================");
  Serial.println("Recording Stopped");
  Serial.print("Distance : ");
  Serial.print(totalDistanceKm, 3);
  Serial.println(" km");
  Serial.println("==============================");
}

// Called once per LOG_INTERVAL_MS while a session is active: writes
// one CSV row and prints the matching Serial debug block.
void updateLogger() {
  unsigned long now = millis();
  if (!isLogging || (now - lastLogWriteTime < LOG_INTERVAL_MS)) return;

  lastLogWriteTime = now;

  writeLogRow();

  double speed = gps.speed.isValid() ? getSmoothedSpeed(gps.speed.kmph()) : 0;
  printDebugRow(speed);
}

static void writeLogRow() {
  if (!logFile || !isLogging) return;

  if (!isGpsLocked()) return;

  double elapsedSeconds = (millis() - logStartMillis) / 1000.0;

  char timestamp[24];
  if (gps.date.isValid() && gps.time.isValid()) {
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
             gps.date.year(), gps.date.month(), gps.date.day(),
             gps.time.hour(), gps.time.minute(), gps.time.second());
  } else {
    snprintf(timestamp, sizeof(timestamp), "N/A");
  }

  logFile.print(elapsedSeconds, 1);
  logFile.print(",");
  logFile.print(timestamp);
  logFile.print(",");
  logFile.print(gps.location.lat(), 6);
  logFile.print(",");
  logFile.print(gps.location.lng(), 6);
  logFile.print(",");
  logFile.print(gps.speed.isValid() ? gps.speed.kmph() : 0.0, 2);
  logFile.print(",");
  logFile.print(totalDistanceKm, 3);
  logFile.print(",");
  logFile.print(gps.satellites.isValid() ? gps.satellites.value() : 0);
  logFile.print(",");
  logFile.println(gps.hdop.isValid() ? gps.hdop.hdop() : 0.0, 1);

  // Flush every row so a sudden power loss (e.g. engine/battery cut)
  // only risks losing the last second of data, not the whole file.
  logFile.flush();
}

static void printDebugRow(double smoothedSpeed) {
  Serial.println("--------------------------------");

  Serial.print("Speed      : ");
  Serial.print(smoothedSpeed, 1);
  Serial.println(" km/h");

  Serial.print("Distance   : ");
  Serial.print(totalDistanceKm, 3);
  Serial.println(" km");

  Serial.print("Latitude   : ");
  Serial.println(gps.location.lat(), 6);

  Serial.print("Longitude  : ");
  Serial.println(gps.location.lng(), 6);

  Serial.print("Satellites : ");
  Serial.println(gps.satellites.value());

  Serial.print("HDOP       : ");
  Serial.println(gps.hdop.hdop());

  Serial.println("--------------------------------");
}