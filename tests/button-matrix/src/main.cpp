#include <Arduino.h>

// 2x2 button matrix: rows are driven, columns are read.
// A pressed button bridges its row to its column.
constexpr uint8_t ROWS = 2;
constexpr uint8_t COLS = 2;
constexpr uint8_t ROW_PINS[ROWS] = {25, 26}; // Outputs, idle HIGH, active LOW
constexpr uint8_t COL_PINS[COLS] = {32, 33}; // INPUT_PULLUP, LOW = pressed

// One LED per button, indexed [row][col], each through a 220R resistor.
constexpr uint8_t LED_PINS[ROWS][COLS] = {
    {18, 19},
    {21, 22},
};

constexpr unsigned long DEBOUNCE_MS = 35;

bool ledOn[ROWS][COLS] = {};
bool lastReading[ROWS][COLS] = {};
bool stablePressed[ROWS][COLS] = {};
unsigned long lastChangeMs[ROWS][COLS] = {};

void setup() {
  Serial.begin(115200);

  for (uint8_t r = 0; r < ROWS; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH);
  }
  for (uint8_t c = 0; c < COLS; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP);
  }
  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      pinMode(LED_PINS[r][c], OUTPUT);
      digitalWrite(LED_PINS[r][c], LOW);
    }
  }

  Serial.println("Press a button to toggle its LED.");
}

void loop() {
  const unsigned long now = millis();

  for (uint8_t r = 0; r < ROWS; r++) {
    digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(10); // Let the line settle before reading

    for (uint8_t c = 0; c < COLS; c++) {
      const bool pressed = digitalRead(COL_PINS[c]) == LOW;

      if (pressed != lastReading[r][c]) {
        lastReading[r][c] = pressed;
        lastChangeMs[r][c] = now;
      }

      if ((now - lastChangeMs[r][c]) >= DEBOUNCE_MS && pressed != stablePressed[r][c]) {
        stablePressed[r][c] = pressed;

        if (pressed) {
          ledOn[r][c] = !ledOn[r][c];
          digitalWrite(LED_PINS[r][c], ledOn[r][c] ? HIGH : LOW);
          Serial.printf("Button R%uC%u -> LED %s\n", r, c, ledOn[r][c] ? "ON" : "OFF");
        }
      }
    }

    digitalWrite(ROW_PINS[r], HIGH);
  }

  delay(2);
}
