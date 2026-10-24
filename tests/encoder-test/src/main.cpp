#include <Arduino.h>

// Resistor encoder: two buttons share one ADC1 pin.
//   3V3 --[10k pull-up]--+-- GPIO34
//                        +--[BTN A]--[20k]-- GND
//                        +--[BTN B]--[10k]-- GND
// Expected voltages: none 3.30V, A 2.20V, B 1.65V, A+B 1.32V.
constexpr uint8_t ENCODER_PIN = 34;   // ADC1, input-only
constexpr uint8_t RED_LED_PIN = 18;   // Lit while button A is held (220R)
constexpr uint8_t GREEN_LED_PIN = 19; // Lit while button B is held (220R)
constexpr unsigned long DEBOUNCE_MS = 35;

// Midpoints between the expected levels, in millivolts.
constexpr uint32_t NONE_MIN_MV = 2750;
constexpr uint32_t A_MIN_MV = 1925;
constexpr uint32_t B_MIN_MV = 1485;

enum ButtonState : uint8_t {
  BUTTONS_NONE = 0,
  BUTTON_A = 1,
  BUTTON_B = 2,
  BUTTONS_BOTH = BUTTON_A | BUTTON_B,
};

ButtonState lastReading = BUTTONS_NONE;
ButtonState stableState = BUTTONS_NONE;
unsigned long lastChangeMs = 0;

ButtonState decode(uint32_t millivolts) {
  if (millivolts >= NONE_MIN_MV) return BUTTONS_NONE;
  if (millivolts >= A_MIN_MV) return BUTTON_A;
  if (millivolts >= B_MIN_MV) return BUTTON_B;
  return BUTTONS_BOTH;
}

const char *stateName(ButtonState state) {
  switch (state) {
    case BUTTON_A: return "A";
    case BUTTON_B: return "B";
    case BUTTONS_BOTH: return "A+B";
    default: return "none";
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(ENCODER_PIN, ADC_11db); // ~0-3.3V range
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  Serial.println("Hold button A (red) or B (green) to light its LED.");
  Serial.printf("Idle reading: %lu mV (expect ~3300)\n", analogReadMilliVolts(ENCODER_PIN));
}

void loop() {
  const uint32_t millivolts = analogReadMilliVolts(ENCODER_PIN);
  const ButtonState reading = decode(millivolts);

  if (reading != lastReading) {
    lastChangeMs = millis();
    lastReading = reading;
  }

  if ((millis() - lastChangeMs) >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    digitalWrite(RED_LED_PIN, (stableState & BUTTON_A) ? HIGH : LOW);
    digitalWrite(GREEN_LED_PIN, (stableState & BUTTON_B) ? HIGH : LOW);
    Serial.printf("Pressed: %s (%lu mV, raw %d)\n", stateName(stableState), millivolts,
                  analogRead(ENCODER_PIN));
  }

  delay(2);
}
