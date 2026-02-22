#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;

constexpr uint8_t BTN_INT_A_PIN = 34;
constexpr uint8_t BTN_INT_B_PIN = 35;

constexpr uint8_t BUTTON_EXP_A = 0x20;
constexpr uint8_t BUTTON_EXP_B = 0x21;
constexpr uint8_t LED_EXP_A = 0x22;
constexpr uint8_t LED_EXP_B = 0x23;

constexpr uint8_t NUM_ROWS = 4;
constexpr uint8_t ROW_LENGTHS[NUM_ROWS] = {1, 3, 5, 7};
constexpr uint8_t ROW_STARTS[NUM_ROWS] = {0, 1, 4, 9};
constexpr uint8_t NUM_TOKENS = 16; // 1 + 3 + 5 + 7

constexpr uint8_t START_BUTTON_PIN = 0; // BUTTON_EXP_B GPA0
constexpr uint8_t PLAYER1_LED_PIN = 0;  // LED_EXP_B GPA0
constexpr uint8_t PLAYER2_LED_PIN = 1;  // LED_EXP_B GPA1
constexpr unsigned long WINNER_BLINK_MS = 700;

bool tokenAlive[NUM_TOKENS];
bool gameActive = false;
uint8_t currentPlayer = 0; // 0 => P1, 1 => P2
uint8_t winner = 255;

uint16_t buttonsA = 0xFFFF;
uint16_t buttonsB = 0xFFFF;
uint16_t lastLedsA = 0xFFFF;
uint16_t lastLedsB = 0xFFFF;
bool winnerBlinkOn = true;
unsigned long lastWinnerBlinkMs = 0;

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

uint8_t rowForToken(uint8_t tokenIndex) {
  for (uint8_t row = 0; row < NUM_ROWS; ++row) {
    const uint8_t start = ROW_STARTS[row];
    const uint8_t end = static_cast<uint8_t>(start + ROW_LENGTHS[row] - 1);
    if (tokenIndex >= start && tokenIndex <= end) {
      return row;
    }
  }
  return 0;
}

bool noTokensLeft() {
  for (uint8_t i = 0; i < NUM_TOKENS; ++i) {
    if (tokenAlive[i]) {
      return false;
    }
  }
  return true;
}

void pushLeds(bool force = false) {
  // Custom chip behavior:
  // bit=1 => INPUT_PULLUP (drives LED anode high in this wiring, LED ON)
  // bit=0 => OUTPUT_LOW (LED OFF)
  uint16_t ledTokens = 0;
  for (uint8_t i = 0; i < NUM_TOKENS; ++i) {
    if (tokenAlive[i]) {
      ledTokens |= static_cast<uint16_t>(1U << i);
    }
  }

  uint16_t ledTurns = 0;
  if (gameActive) {
    ledTurns |= static_cast<uint16_t>(1U << (currentPlayer == 0 ? PLAYER1_LED_PIN : PLAYER2_LED_PIN));
  } else if (winner <= 1) {
    if (winnerBlinkOn) {
      ledTurns |= static_cast<uint16_t>(1U << (winner == 0 ? PLAYER1_LED_PIN : PLAYER2_LED_PIN));
    }
  }

  if (force || ledTokens != lastLedsA) {
    writePorts(LED_EXP_A, ledTokens);
    lastLedsA = ledTokens;
  }
  if (force || ledTurns != lastLedsB) {
    writePorts(LED_EXP_B, ledTurns);
    lastLedsB = ledTurns;
  }
}

void startNewGame() {
  for (uint8_t i = 0; i < NUM_TOKENS; ++i) {
    tokenAlive[i] = true;
  }
  gameActive = true;
  currentPlayer = 0;
  winner = 255;
  winnerBlinkOn = true;
  lastWinnerBlinkMs = millis();
  pushLeds(true);
  Serial.println("New game started. Player 1 turn.");
}

void applyMove(uint8_t tokenIndex) {
  if (tokenIndex >= NUM_TOKENS || !tokenAlive[tokenIndex] || !gameActive) {
    return;
  }

  const uint8_t row = rowForToken(tokenIndex);
  const uint8_t rowStart = ROW_STARTS[row];
  const uint8_t rowEnd = static_cast<uint8_t>(rowStart + ROW_LENGTHS[row] - 1);

  // Nim move with one click: remove selected token and all tokens to its right in same row.
  for (uint8_t i = tokenIndex; i <= rowEnd; ++i) {
    tokenAlive[i] = false;
  }

  if (noTokensLeft()) {
    gameActive = false;
    winner = currentPlayer;
    winnerBlinkOn = true;
    lastWinnerBlinkMs = millis();
    Serial.printf("Player %u wins. Press START for new game.\n", static_cast<unsigned>(winner + 1));
  } else {
    currentPlayer = static_cast<uint8_t>(1 - currentPlayer);
    Serial.printf("Player %u turn.\n", static_cast<unsigned>(currentPlayer + 1));
  }

  pushLeds(true);
}

void updateWinnerBlink() {
  if (gameActive || winner > 1) {
    return;
  }

  const unsigned long now = millis();
  if (now - lastWinnerBlinkMs < WINNER_BLINK_MS) {
    return;
  }

  lastWinnerBlinkMs = now;
  winnerBlinkOn = !winnerBlinkOn;
  pushLeds(false);
}

void pollButtonsAndHandleEdges() {
  const uint16_t oldA = buttonsA;
  const uint16_t oldB = buttonsB;

  // INTA is open-drain. LOW means there was an input change.
  if (digitalRead(BTN_INT_A_PIN) == LOW) {
    buttonsA = readPorts(BUTTON_EXP_A);
  }
  if (digitalRead(BTN_INT_B_PIN) == LOW) {
    buttonsB = readPorts(BUTTON_EXP_B);
  }

  const uint16_t pressedA = static_cast<uint16_t>(oldA & ~buttonsA); // HIGH -> LOW
  const uint16_t pressedB = static_cast<uint16_t>(oldB & ~buttonsB); // HIGH -> LOW

  if (pressedB & static_cast<uint16_t>(1U << START_BUTTON_PIN)) {
    startNewGame();
    return;
  }

  if (!gameActive) {
    return;
  }

  // One click = one move.
  for (uint8_t i = 0; i < NUM_TOKENS; ++i) {
    if (pressedA & static_cast<uint16_t>(1U << i)) {
      applyMove(i);
      return;
    }
  }
}
} // namespace

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  pinMode(BTN_INT_A_PIN, INPUT_PULLUP);
  pinMode(BTN_INT_B_PIN, INPUT_PULLUP);

  // Button chips: all pins input/pull-up.
  writePorts(BUTTON_EXP_A, 0xFFFF);
  writePorts(BUTTON_EXP_B, 0xFFFF);

  // Prime button state once (also clears interrupt flags).
  buttonsA = readPorts(BUTTON_EXP_A);
  buttonsB = readPorts(BUTTON_EXP_B);

  // LEDs off before game start.
  writePorts(LED_EXP_A, 0x0000);
  writePorts(LED_EXP_B, 0x0000);
  lastLedsA = 0x0000;
  lastLedsB = 0x0000;

  startNewGame();
}

void loop() {
  pollButtonsAndHandleEdges();
  updateWinnerBlink();
  delay(3);
}
