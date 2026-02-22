#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 5;   // Active-low button (INPUT_PULLUP)
constexpr uint8_t RED_LED_PIN = 18; // External red LED through 220R
constexpr unsigned long DEBOUNCE_MS = 35;

bool ledOn = false;
int lastReading = HIGH;
int stableState = HIGH;
unsigned long lastChangeMs = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  Serial.println("Press the button to toggle the red LED.");
}

void loop() {
  const int reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    lastChangeMs = millis();
    lastReading = reading;
  }

  if ((millis() - lastChangeMs) >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;

    if (stableState == LOW) {
      ledOn = !ledOn;
      digitalWrite(RED_LED_PIN, ledOn ? HIGH : LOW);
      Serial.printf("LED is now: %s\n", ledOn ? "ON" : "OFF");
    }
  }

  delay(2);
}

