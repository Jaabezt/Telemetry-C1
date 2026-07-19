#include "display.h"
#include "config.h"
#include "gps.h"
#include "telemetry.h"
#include "logger.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// Temporary confirmation message shown after button presses
static bool showingStatusMessage = false;
static String statusMessageLine1 = "";
static String statusMessageLine2 = "";
static unsigned long statusMessageStartTime = 0;

static unsigned long lastDisplayUpdate = 0;

static void clearRow(int row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
}

void initDisplay() {
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void showStatusMessage(const String &line1, const String &line2) {
  showingStatusMessage = true;
  statusMessageLine1 = line1;
  statusMessageLine2 = line2;
  statusMessageStartTime = millis();
}

void showBootMessage(bool sdReady) {
  showStatusMessage("Acquiring GPS...", sdReady ? "SD OK - Press btn" : "SD FAIL - no log");
}

// Format seconds as mm:ss for compact LCD display.
String formatElapsed(double totalSeconds) {
  unsigned long totalSecs = (unsigned long)totalSeconds;
  unsigned long mins = totalSecs / 60;
  unsigned long secs = totalSecs % 60;
  char buf[8];
  snprintf(buf, sizeof(buf), "%02lu:%02lu", mins, secs);
  return String(buf);
}

void updateDisplay() {
  unsigned long now = millis();
  if (now - lastDisplayUpdate < DISPLAY_INTERVAL_MS) return;
  lastDisplayUpdate = now;

  if (showingStatusMessage) {
    if (millis() - statusMessageStartTime < STATUS_MESSAGE_DURATION_MS) {
      clearRow(0);
      clearRow(1);
      lcd.setCursor(0, 0);
      lcd.print(statusMessageLine1);
      lcd.setCursor(0, 1);
      lcd.print(statusMessageLine2);
      return;
    } else {
      showingStatusMessage = false;
    }
  }

  if (!isLogging) {
    clearRow(0);
    clearRow(1);

    lcd.setCursor(0, 0);
    lcd.print("Press Button");

    lcd.setCursor(0, 1);
    lcd.print("Start Record");

    return;
  }

  bool gpsLocked = isGpsLocked();

  clearRow(0);
  clearRow(1);

  if (!gpsLocked) {
    lcd.setCursor(0, 0);
    lcd.print("No GPS signal");
    if (isLogging) {
      lcd.setCursor(13, 0);
      lcd.print("REC");
    }
    lcd.setCursor(0, 1);
    if (!sdCardReady) {
      lcd.print("SD CARD FAILED");
    } else {
      lcd.print("Sats: ");
      lcd.print(gps.satellites.isValid() ? gps.satellites.value() : 0);
    }
    return;
  }

  // --- Line 1: speed, with REC tag if logging ---
  double rawSpeedKmh = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
  bool fixIsTrusted = (!gps.hdop.isValid()) || (gps.hdop.hdop() <= MAX_TRUSTED_HDOP);

  double speedKmh = getSmoothedSpeed(fixIsTrusted ? rawSpeedKmh : 0.0);

  if (speedKmh < MIN_TRUSTED_SPEED_KMH)
    speedKmh = 0.0;

  lcd.setCursor(0, 0);
  lcd.print("SPD:");
  lcd.print(speedKmh, 1);
  lcd.print("km/h ");

  lcd.setCursor(0, 1);
  lcd.print("DST:");
  lcd.print(totalDistanceKm, 3);
  lcd.print("km   ");
}