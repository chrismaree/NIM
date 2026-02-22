#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;

constexpr uint8_t BTN_INT_A_PIN = 34;
constexpr uint8_t BTN_INT_B_PIN = 35;

constexpr uint8_t NUM_CHANNELS = 27;

constexpr uint8_t BUTTON_EXP_A = 0x20;
constexpr uint8_t BUTTON_EXP_B = 0x21;
constexpr uint8_t LED_EXP_A = 0x22;
constexpr uint8_t LED_EXP_B = 0x23;

struct ExpandedPin {
  uint8_t address;
  uint8_t pin; // 0..15
};

ExpandedPin buttonMap[NUM_CHANNELS];
ExpandedPin ledMap[NUM_CHANNELS];

uint16_t buttonsA = 0xFFFF;
uint16_t buttonsB = 0xFFFF;
uint16_t lastLedWriteA = 0xFFFF;
uint16_t lastLedWriteB = 0xFFFF;

void buildMaps() {
  for (uint8_t i = 0; i < NUM_CHANNELS; ++i) {
    buttonMap[i] = (i < 16) ? ExpandedPin{BUTTON_EXP_A, i}
                            : ExpandedPin{BUTTON_EXP_B, static_cast<uint8_t>(i - 16)};
    ledMap[i] = (i < 16) ? ExpandedPin{LED_EXP_A, i}
                         : ExpandedPin{LED_EXP_B, static_cast<uint8_t>(i - 16)};
  }
}

// Custom chip protocol:
// write two bytes only: low port then high port.
bool writePorts(uint8_t address, uint16_t value) {
  Wire.beginTransmission(address);
  Wire.write(static_cast<uint8_t>(value & 0xFF));
  Wire.write(static_cast<uint8_t>((value >> 8) & 0xFF));
  return Wire.endTransmission() == 0;
}

uint16_t readPorts(uint8_t address) {
  if (Wire.requestFrom(static_cast<int>(address), 2) != 2) {
    return 0xFFFF;
  }
  const uint8_t low = Wire.read();
  const uint8_t high = Wire.read();
  return static_cast<uint16_t>(low) | (static_cast<uint16_t>(high) << 8);
}

bool isPressed(uint8_t index) {
  const ExpandedPin pin = buttonMap[index];
  const uint16_t mask = static_cast<uint16_t>(1U << pin.pin);
  const bool levelHigh = (pin.address == BUTTON_EXP_A) ? ((buttonsA & mask) != 0)
                                                        : ((buttonsB & mask) != 0);
  return !levelHigh; // pull-up input: LOW means pressed
}

void refreshButtonStatesIfNeeded() {
  // INTA is open-drain on the custom chip. Use ESP32 internal pull-ups.
  const bool needReadA = (digitalRead(BTN_INT_A_PIN) == LOW);
  const bool needReadB = (digitalRead(BTN_INT_B_PIN) == LOW);

  if (needReadA) {
    buttonsA = readPorts(BUTTON_EXP_A);
  }
  if (needReadB) {
    buttonsB = readPorts(BUTTON_EXP_B);
  }
}

void pushLedState(bool force = false) {
  // On this custom chip:
  // bit=0 => output low
  // bit=1 => input_pullup/high
  // With the current wiring (LED anode on expander pin, cathode to GND),
  // pressed button should light LED -> use bit=1 for ON.
  uint16_t ledA = 0;
  uint16_t ledB = 0;

  for (uint8_t i = 0; i < NUM_CHANNELS; ++i) {
    if (!isPressed(i)) {
      continue;
    }
    const ExpandedPin ledPin = ledMap[i];
    if (ledPin.address == LED_EXP_A) {
      ledA |= static_cast<uint16_t>(1U << ledPin.pin);
    } else {
      ledB |= static_cast<uint16_t>(1U << ledPin.pin);
    }
  }

  if (force || ledA != lastLedWriteA) {
    writePorts(LED_EXP_A, ledA);
    lastLedWriteA = ledA;
  }
  if (force || ledB != lastLedWriteB) {
    writePorts(LED_EXP_B, ledB);
    lastLedWriteB = ledB;
  }
}
} // namespace

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  buildMaps();

  pinMode(BTN_INT_A_PIN, INPUT_PULLUP);
  pinMode(BTN_INT_B_PIN, INPUT_PULLUP);

  // Buttons as inputs with pull-up on all 16 pins for each chip.
  writePorts(BUTTON_EXP_A, 0xFFFF);
  writePorts(BUTTON_EXP_B, 0xFFFF);

  // Prime initial state once (also clears initial interrupt flags in the chip).
  buttonsA = readPorts(BUTTON_EXP_A);
  buttonsB = readPorts(BUTTON_EXP_B);
  pushLedState(true);

  Serial.println("Port-expander test ready (27 buttons + 27 LEDs).");
}

void loop() {
  refreshButtonStatesIfNeeded();
  pushLedState(false);
  delay(2);
}

