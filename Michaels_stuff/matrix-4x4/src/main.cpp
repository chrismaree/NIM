#include <Arduino.h>

constexpr uint8_t LIGHT_COUNT = 16;
constexpr uint8_t MATRIX_SIZE = 4;
constexpr uint8_t MATRIX_ROW_PINS[MATRIX_SIZE] = {1, 2, 4, 5};
constexpr uint8_t MATRIX_COLUMN_PINS[MATRIX_SIZE] = {6, 7, 8, 9};
constexpr uint8_t LED_PINS[LIGHT_COUNT] = {
    10, 11, 12, 13, 14, 15, 16, 17, 18, 21, 37, 47, 35, 36, 43, 44};
constexpr uint8_t PLAYER_INDICATOR_PINS[2] = {38, 39};
constexpr uint8_t MODE_SWITCH_PIN = 40;
constexpr uint8_t WINNER_LED_PINS[2] = {41, 42};
constexpr uint8_t ROW_STARTS[MATRIX_SIZE] = {0, 1, 4, 9};
constexpr uint8_t ROW_ENDS[MATRIX_SIZE] = {1, 4, 9, LIGHT_COUNT};
constexpr unsigned long DEBOUNCE_MS = 30;
constexpr unsigned long AI_THINK_MS = 500;
constexpr unsigned long WINNER_BLINK_INTERVAL_MS = 300;
constexpr unsigned long WINNER_DISPLAY_MS = 3000;
constexpr unsigned long RESTART_BLINK_INTERVAL_MS = 250;

bool lightIsOn[LIGHT_COUNT] = {};
bool lastReading[LIGHT_COUNT] = {};
bool buttonState[LIGHT_COUNT] = {};
unsigned long lastChangeTime[LIGHT_COUNT] = {};
bool gameOver = false;
bool aiMode = false;
bool aiMovePending = false;
unsigned long aiMoveScheduledAt = 0;
uint8_t currentPlayer = 0;
bool lastModeReading = false;
unsigned long lastModeChangeTime = 0;
bool restartBlinkActive = false;
bool restartBlinkOff = false;
unsigned long restartBlinkChangedAt = 0;
uint8_t winnerLedIndex = 0;
bool winnerLedOn = false;
unsigned long winnerDisplayStartedAt = 0;
unsigned long winnerLedToggledAt = 0;

void updatePlayerIndicators() {
  for (uint8_t player = 0; player < 2; ++player) {
    digitalWrite(PLAYER_INDICATOR_PINS[player],
                 !gameOver && player == currentPlayer ? HIGH : LOW);
  }
}

void setAllLeds(uint8_t state) {
  for (const uint8_t pin : LED_PINS) {
    digitalWrite(pin, state);
  }
  for (const uint8_t pin : PLAYER_INDICATOR_PINS) {
    digitalWrite(pin, state);
  }
  for (const uint8_t pin : WINNER_LED_PINS) {
    digitalWrite(pin, state);
  }
}

void startNewGame() {
  const unsigned long now = millis();
  for (uint8_t light = 0; light < LIGHT_COUNT; ++light) {
    lightIsOn[light] = true;
    digitalWrite(LED_PINS[light], HIGH);
    lastReading[light] = false;
    buttonState[light] = false;
    lastChangeTime[light] = now;
  }
  currentPlayer = 0;
  gameOver = false;
  aiMovePending = false;
  winnerLedOn = false;
  restartBlinkActive = true;
  restartBlinkOff = false;
  restartBlinkChangedAt = now;
  digitalWrite(WINNER_LED_PINS[0], LOW);
  digitalWrite(WINNER_LED_PINS[1], LOW);
  updatePlayerIndicators();
  setAllLeds(HIGH);
  Serial.println("New game. Player 1 starts.");
}

void finishTurn(uint8_t player) {
  for (const bool isOn : lightIsOn) {
    if (isOn) {
      currentPlayer = 1 - player;
      updatePlayerIndicators();
      if (aiMode && currentPlayer == 1) {
        aiMovePending = true;
        aiMoveScheduledAt = millis();
        Serial.println("Agent is thinking...");
      } else {
        Serial.print(currentPlayer == 0 ? "Player 1" : "Player 2");
        Serial.println("'s turn.");
      }
      return;
    }
  }

  gameOver = true;
  updatePlayerIndicators();
  winnerLedIndex = 1 - player;
  winnerLedOn = true;
  winnerDisplayStartedAt = millis();
  winnerLedToggledAt = winnerDisplayStartedAt;
  digitalWrite(WINNER_LED_PINS[winnerLedIndex], HIGH);
  Serial.print(aiMode && winnerLedIndex == 1 ? "Agent" : "Player ");
  if (!aiMode || winnerLedIndex == 0) {
    Serial.print(winnerLedIndex + 1);
  }
  Serial.println(" wins! Taking the last light loses.");
}

void applyMove(uint8_t light) {
  uint8_t row = 0;
  while (light >= ROW_ENDS[row]) {
    ++row;
  }

  uint8_t removedCount = 0;
  for (uint8_t index = light; index < ROW_ENDS[row]; ++index) {
    if (lightIsOn[index]) {
      lightIsOn[index] = false;
      digitalWrite(LED_PINS[index], LOW);
      ++removedCount;
    }
  }

  Serial.print("Player ");
  Serial.print(currentPlayer + 1);
  Serial.print(" removed ");
  Serial.print(removedCount);
  Serial.print(" light(s) from row ");
  Serial.println(row + 1);
  finishTurn(currentPlayer);
}

void scanButtons() {
  const unsigned long now = millis();
  int8_t newlyPressedLight = -1;

  for (uint8_t matrixRow = 0; matrixRow < MATRIX_SIZE; ++matrixRow) {
    pinMode(MATRIX_ROW_PINS[matrixRow], OUTPUT);
    digitalWrite(MATRIX_ROW_PINS[matrixRow], LOW);
    delayMicroseconds(5);

    for (uint8_t matrixColumn = 0; matrixColumn < MATRIX_SIZE;
         ++matrixColumn) {
      const uint8_t light = matrixRow * MATRIX_SIZE + matrixColumn;
      const bool reading =
          digitalRead(MATRIX_COLUMN_PINS[matrixColumn]) == LOW;

      if (reading != lastReading[light]) {
        lastReading[light] = reading;
        lastChangeTime[light] = now;
      }

      if (now - lastChangeTime[light] >= DEBOUNCE_MS &&
          reading != buttonState[light]) {
        buttonState[light] = reading;
        if (reading && newlyPressedLight < 0) {
          newlyPressedLight = light;
        }
      }
    }

    pinMode(MATRIX_ROW_PINS[matrixRow], INPUT);
  }

  if (!gameOver && (!aiMode || currentPlayer == 0) && newlyPressedLight >= 0 &&
      lightIsOn[static_cast<uint8_t>(newlyPressedLight)]) {
    applyMove(static_cast<uint8_t>(newlyPressedLight));
  }
}

bool chooseAgentButton(uint8_t &buttonIndex) {
  uint8_t heaps[MATRIX_SIZE] = {};
  uint8_t nonemptyCount = 0;
  uint8_t largeHeapCount = 0;
  uint8_t firstNonemptyRow = 0;
  uint8_t largeHeapRow = 0;
  uint8_t singleHeapCount = 0;
  uint8_t nimSum = 0;

  for (uint8_t row = 0; row < MATRIX_SIZE; ++row) {
    for (uint8_t light = ROW_STARTS[row]; light < ROW_ENDS[row]; ++light) {
      if (lightIsOn[light]) {
        ++heaps[row];
      }
    }

    if (heaps[row] > 0) {
      if (nonemptyCount == 0) {
        firstNonemptyRow = row;
      }
      ++nonemptyCount;
      nimSum ^= heaps[row];
      if (heaps[row] > 1) {
        ++largeHeapCount;
        largeHeapRow = row;
      } else {
        ++singleHeapCount;
      }
    }
  }

  if (nonemptyCount == 0) {
    return false;
  }

  uint8_t selectedRow = firstNonemptyRow;
  uint8_t removeCount = 1;
  if (largeHeapCount == 1) {
    const uint8_t target = singleHeapCount % 2 == 0 ? 1 : 0;
    removeCount = heaps[largeHeapRow] - target;
    selectedRow = largeHeapRow;
  } else if (largeHeapCount > 1 && nimSum != 0) {
    for (uint8_t row = 0; row < MATRIX_SIZE; ++row) {
      const uint8_t target = heaps[row] ^ nimSum;
      if (target < heaps[row]) {
        selectedRow = row;
        removeCount = heaps[row] - target;
        break;
      }
    }
  }

  buttonIndex =
      ROW_STARTS[selectedRow] + heaps[selectedRow] - removeCount;
  return true;
}

void updateRestartBlink(unsigned long now) {
  if (!restartBlinkActive ||
      now - restartBlinkChangedAt < RESTART_BLINK_INTERVAL_MS) {
    return;
  }

  restartBlinkOff = !restartBlinkOff;
  restartBlinkChangedAt = now;
  setAllLeds(restartBlinkOff ? LOW : HIGH);
  if (!restartBlinkOff) {
    restartBlinkActive = false;
    digitalWrite(WINNER_LED_PINS[0], LOW);
    digitalWrite(WINNER_LED_PINS[1], LOW);
    updatePlayerIndicators();
  }
}

void updateWinnerDisplay(unsigned long now) {
  if (!gameOver) {
    return;
  }

  if (now - winnerDisplayStartedAt >= WINNER_DISPLAY_MS) {
    startNewGame();
    return;
  }

  if (now - winnerLedToggledAt >= WINNER_BLINK_INTERVAL_MS) {
    winnerLedOn = !winnerLedOn;
    digitalWrite(WINNER_LED_PINS[winnerLedIndex],
                 winnerLedOn ? HIGH : LOW);
    winnerLedToggledAt = now;
  }
}

void setup() {
  Serial.begin(115200);

  for (const uint8_t pin : MATRIX_ROW_PINS) {
    pinMode(pin, INPUT);
  }
  for (const uint8_t pin : MATRIX_COLUMN_PINS) {
    pinMode(pin, INPUT_PULLUP);
  }
  for (const uint8_t pin : LED_PINS) {
    pinMode(pin, OUTPUT);
  }

  for (const uint8_t pin : PLAYER_INDICATOR_PINS) {
    pinMode(pin, OUTPUT);
  }
  for (const uint8_t pin : WINNER_LED_PINS) {
    pinMode(pin, OUTPUT);
  }
  pinMode(MODE_SWITCH_PIN, INPUT);
  aiMode = digitalRead(MODE_SWITCH_PIN) == HIGH;
  lastModeReading = aiMode;
  lastModeChangeTime = millis();
  startNewGame();
  Serial.print("Nim ready in ");
  Serial.print(aiMode ? "AI mode" : "two-player mode");
  Serial.println(". Player 1 goes first.");
}

void loop() {
  const unsigned long now = millis();
  const bool modeReading = digitalRead(MODE_SWITCH_PIN) == HIGH;
  if (modeReading != lastModeReading) {
    lastModeReading = modeReading;
    lastModeChangeTime = now;
  } else if (modeReading != aiMode &&
             now - lastModeChangeTime >= DEBOUNCE_MS) {
    aiMode = modeReading;
    startNewGame();
    Serial.print("Mode changed to ");
    Serial.print(aiMode ? "AI mode" : "two-player mode");
    Serial.println(". Restarting game.");
  }

  updateWinnerDisplay(now);
  updateRestartBlink(now);
  if (gameOver || restartBlinkActive) {
    return;
  }

  scanButtons();

  if (aiMovePending && now - aiMoveScheduledAt >= AI_THINK_MS) {
    aiMovePending = false;
    uint8_t agentButton = 0;
    if (!chooseAgentButton(agentButton)) {
      Serial.println("Agent could not find a legal move.");
      gameOver = true;
      updatePlayerIndicators();
      return;
    }
    applyMove(agentButton);
  }
}
