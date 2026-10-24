// Wokwi custom chip: models the two-button resistor encoder.
// Wokwi cannot solve resistor dividers, so this chip computes the node voltage:
//   VCC --[r_pullup]--+-- OUT
//                     +--[A to GND]--[r_a]-- GND
//                     +--[B to GND]--[r_b]-- GND
// Buttons connect A or B to GND; the chip holds them high with pull-ups.

#include "wokwi-api.h"
#include <stdlib.h>

typedef struct {
  pin_t pin_a;
  pin_t pin_b;
  pin_t pin_out;
  float vcc;
  float r_pullup;
  float r_a;
  float r_b;
} chip_state_t;

static void update_output(chip_state_t *chip) {
  float conductance = 0.0f;
  if (pin_read(chip->pin_a) == LOW) conductance += 1.0f / chip->r_a;
  if (pin_read(chip->pin_b) == LOW) conductance += 1.0f / chip->r_b;
  pin_dac_write(chip->pin_out, chip->vcc / (1.0f + chip->r_pullup * conductance));
}

static void on_button_change(void *user_data, pin_t pin, uint32_t value) {
  update_output((chip_state_t *)user_data);
}

void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  chip->pin_a = pin_init("A", INPUT_PULLUP);
  chip->pin_b = pin_init("B", INPUT_PULLUP);
  chip->pin_out = pin_init("OUT", ANALOG);
  chip->vcc = attr_read_float(attr_init_float("vcc", 3.3f));
  chip->r_pullup = attr_read_float(attr_init_float("r_pullup", 10000.0f));
  chip->r_a = attr_read_float(attr_init_float("r_a", 20000.0f));
  chip->r_b = attr_read_float(attr_init_float("r_b", 10000.0f));

  const pin_watch_config_t watch = {
    .user_data = chip,
    .edge = BOTH,
    .pin_change = on_button_change,
  };
  pin_watch(chip->pin_a, &watch);
  pin_watch(chip->pin_b, &watch);

  update_output(chip);
}
