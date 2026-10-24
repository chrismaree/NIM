#include <Arduino.h>

constexpr uint8_t MATRIX_SIZE = 4;
constexpr uint8_t ROW_PINS[MATRIX_SIZE] = {1, 2, 4, 5};
constexpr uint8_t COLUMN_PINS[MATRIX_SIZE] = {6, 7, 8, 9};
constexpr unsigned long DEBOUNCE_MS = 30;

bool lastReading[MATRIX_SIZE][MATRIX_SIZE] = {};
bool pressed[MATRIX_SIZE][MATRIX_SIZE] = {};
unsigned long lastChangeTime[MATRIX_SIZE][MATRIX_SIZE] = {};

uint8_t buttonNumber(uint8_t row, uint8_t column) {
  return row * MATRIX_SIZE + column + 1;
}

void printGrid() {
  Serial.println("        C1(6) C2(7) C3(8) C4(9)");
  for (uint8_t row = 0; row < MATRIX_SIZE; ++row) {
    Serial.printf("R%u(%u)  ", row + 1, ROW_PINS[row]);
    for (uint8_t column = 0; column < MATRIX_SIZE; ++column) {
      if (pressed[row][column]) {
        Serial.printf("  [%2u]", buttonNumber(row, column));
      } else {
        Serial.print("    . ");
      }
    }
    Serial.println();
  }
}

void printHeld() {
  uint8_t count = 0;
  Serial.print("Held:");
  for (uint8_t row = 0; row < MATRIX_SIZE; ++row) {
    for (uint8_t column = 0; column < MATRIX_SIZE; ++column) {
      if (pressed[row][column]) {
        Serial.printf(" BTN%u", buttonNumber(row, column));
        ++count;
      }
    }
  }
  Serial.println(count == 0 ? " none" : "");
}

void reportChange(uint8_t row, uint8_t column, bool isPressed) {
  Serial.printf("%s BTN%-2u  row %u (GPIO %u)  col %u (GPIO %u)  t=%lums\n",
                isPressed ? "PRESS  " : "RELEASE", buttonNumber(row, column),
                row + 1, ROW_PINS[row], column + 1, COLUMN_PINS[column],
                millis());
  printHeld();
  printGrid();
  Serial.println();
}

void scanMatrix() {
  const unsigned long now = millis();

  for (uint8_t row = 0; row < MATRIX_SIZE; ++row) {
    pinMode(ROW_PINS[row], OUTPUT);
    digitalWrite(ROW_PINS[row], LOW);
    delayMicroseconds(5);

    for (uint8_t column = 0; column < MATRIX_SIZE; ++column) {
      const bool reading = digitalRead(COLUMN_PINS[column]) == LOW;

      if (reading != lastReading[row][column]) {
        lastReading[row][column] = reading;
        lastChangeTime[row][column] = now;
      }

      if (now - lastChangeTime[row][column] >= DEBOUNCE_MS &&
          reading != pressed[row][column]) {
        pressed[row][column] = reading;
        reportChange(row, column, reading);
      }
    }

    pinMode(ROW_PINS[row], INPUT);
  }
}

void checkIdleColumns() {
  for (uint8_t column = 0; column < MATRIX_SIZE; ++column) {
    if (digitalRead(COLUMN_PINS[column]) == LOW) {
      Serial.printf("WARNING: column %u (GPIO %u) reads LOW with no row "
                    "driven. Check it for a short to GND.\n",
                    column + 1, COLUMN_PINS[column]);
    }
  }
}

void setup() {
  Serial.begin(115200);
  const unsigned long waitStart = millis();
  while (!Serial && millis() - waitStart < 3000) {
    delay(10);
  }

  for (const uint8_t pin : ROW_PINS) {
    pinMode(pin, INPUT);
  }
  for (const uint8_t pin : COLUMN_PINS) {
    pinMode(pin, INPUT_PULLUP);
  }
  delay(10);

  Serial.println();
  Serial.println("4x4 button matrix serial test");
  Serial.println("Rows (driven LOW one at a time): GPIO 1, 2, 4, 5");
  Serial.println("Columns (INPUT_PULLUP):          GPIO 6, 7, 8, 9");
  Serial.println("Press buttons; each press and release is printed.");
  checkIdleColumns();
  Serial.println();
  printGrid();
  Serial.println();
}

void loop() {
  scanMatrix();
  delay(1);
}
