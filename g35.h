#ifndef G35_H
#define G35_H
#include <stdlib.h> 
#include <stdint.h>
#include <Arduino.h>
#include "delay_x.h"

enum COLOR {
  RED = 0xf00,
  GREEN = 0x0f0,
  BLUE = 0x00f,
  BLACK = 0x000,
  WHITE = 0xddd,
  YELLOW = 0xfa0,
  AQUA = 0x0ff, 
  FUCHSIA = 0xf05,
  RED2 = 0x800,
  GREEN2 = 0x030,
  PURPLE = 0xa0a,
  TEAL = 0x088,
  NAVY = 0x008, // good
  ORANGE = 0xf20,
  NO_COLOR = 0xffff
};

extern uint16_t colors[];
extern uint8_t num_colors;
extern uint8_t num_bulbs;

void add_light_string(uint8_t pin, uint8_t num_bulbs);
void assign_bulb_addresses();
void write_strings();

// transformations
uint8_t dec_intensity();
uint8_t inc_intensity();
void transpose(uint8_t block_size);
void shift_left();

// assignments
void set_bulb(uint8_t i, uint16_t color);
void set_bulb_with_intensity(uint8_t i, uint16_t color, uint8_t intensity);
void global_intensity(uint8_t intensity);
void every_n_color(uint8_t offset, uint8_t step, uint8_t count, uint16_t color);
void fill_color(uint16_t color);
void repeat(uint16_t *colors, uint8_t len);



#endif
