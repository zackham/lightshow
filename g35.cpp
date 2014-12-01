#include "g35.h"

#define MAX_INTENSITY 0xcc

typedef struct {
  uint8_t intensity;
  uint16_t color;
  uint8_t modified;
} bulb_type;

typedef struct {
  uint8_t pin;
  uint8_t num_bulbs;
  bulb_type *bulbs;
  bulb_type global_intensity_bulb;
} light_string_type;



uint16_t colors[] = {
  WHITE,
  YELLOW,
  BLUE,
  RED,
  AQUA,
  GREEN,
  FUCHSIA,
  NAVY,
  GREEN2,
  TEAL,
  ORANGE,
  PURPLE,
  RED2
};
uint8_t num_colors = sizeof(colors) / sizeof(colors[0]);

static void write_string(light_string_type*);
static void write_bulb(uint8_t pin, uint8_t addr, bulb_type*);

// if light strings are physically connected end-to-end, 
// we use this array to address them as if they are one string
static bulb_type *vbulbs[50];
uint8_t num_bulbs = 0;

static light_string_type light_strings[1];
static uint8_t num_strings = 0;

/* 
 * Bring a new light string into the system. Do this before assigning addresses =)
 */
void add_light_string(uint8_t pin, uint8_t str_num_bulbs) {
  light_string_type *str = (light_string_type *) malloc(sizeof(light_string_type));
  str->pin = pin;
  str->num_bulbs = str_num_bulbs;
  str->bulbs = (bulb_type *) malloc(sizeof(bulb_type) * str_num_bulbs);
  light_strings[num_strings++] = *str;

  for(uint8_t i=0; i < str_num_bulbs; i++) {
    str->bulbs[i].intensity = 0xcc;
    str->bulbs[i].modified = false;
    vbulbs[num_bulbs+i] = &str->bulbs[i];
  }

  num_bulbs += str_num_bulbs;
}

void assign_bulb_addresses() {
  write_strings();
}

void set_bulb(uint8_t i, uint16_t color) {
  if(vbulbs[i]->color != color) {
    vbulbs[i]->color = color;
    vbulbs[i]->modified = true;
  }
}

void set_bulb_with_intensity(uint8_t i, uint16_t color, uint8_t intensity) {
  if(vbulbs[i]->color != color || (intensity <= MAX_INTENSITY && vbulbs[i]->intensity != intensity)) {
    vbulbs[i]->color = color;
    if(intensity <= MAX_INTENSITY)
      vbulbs[i]->intensity = intensity;
    vbulbs[i]->modified = true;
  }
}

void transpose(uint8_t block_size) {
  uint16_t tmp_c;
  uint8_t tmp_i, swap_i;
  for(uint8_t i=block_size; i < num_bulbs; i++) {
    swap_i = i - block_size;
    tmp_c = vbulbs[i]->color;
    tmp_i = vbulbs[i]->intensity;
    vbulbs[i]->color = vbulbs[swap_i]->color;
    vbulbs[i]->intensity = vbulbs[swap_i]->intensity;
    vbulbs[i]->modified = true;
    vbulbs[swap_i]->color = tmp_c;
    vbulbs[swap_i]->intensity = tmp_i;
    vbulbs[swap_i]->modified = true;
  }
}

uint8_t dec_intensity() {
  uint8_t intensity = light_strings[0].global_intensity_bulb.intensity;
  if(intensity > 0) {
    global_intensity(intensity - 1);
  }
  return intensity;
}

uint8_t inc_intensity() {
  uint8_t intensity = light_strings[0].global_intensity_bulb.intensity;
  if(intensity < MAX_INTENSITY) {
    global_intensity(intensity + 1);
  }
  return intensity;
}

void global_intensity(uint8_t intensity) {
  uint8_t i=0;
  for(; i < num_strings; i++) {
    light_strings[i].global_intensity_bulb.intensity = intensity;
    light_strings[i].global_intensity_bulb.modified = true;
  }
  for(i=0; i < num_bulbs; i++) {
    vbulbs[i]->intensity = intensity;
  }
}

void fill_color(uint16_t color) {
  for(uint8_t i=0; i < num_bulbs; i++) {
    vbulbs[i]->color = color;
    // TODO iff color changed
    vbulbs[i]->modified = true;
  }
}

void every_n_color(uint8_t offset, uint8_t skip, uint8_t fill_count, uint16_t color) {
  uint8_t do_color = 0;
  for(uint8_t i=0;i < num_bulbs; i++) {
    if(do_color) {
      vbulbs[i]->color = color;
      vbulbs[i]->modified = true;
      do_color--;
    } else {
      if(i >= offset && (i + offset) % skip == 0) do_color = fill_count;
    }
  }
}

void repeat(uint16_t *colors, uint8_t len) {
  uint8_t i=0, j;
  while(1) {
    for(j=0; j < len; j++) {
      if(i == num_bulbs) return;
      vbulbs[i]->color = colors[j];
      vbulbs[i]->modified = true;
      i++;
    }
  }
}

void shift_left() {
  uint16_t last_c, first_c;
  first_c = vbulbs[0]->color;
  for(uint8_t i=0; i < num_bulbs-1; i++) {
    last_c = vbulbs[i]->color;
    set_bulb(i, vbulbs[i+1]->color);
  }
  set_bulb(num_bulbs-1, first_c);
}

void write_strings() {
  for(uint8_t i=0; i < num_strings; i++) {
    write_string(&light_strings[i]);
  }
}

static void write_string(light_string_type *str) {
  for(uint8_t i=0; i < str->num_bulbs; i++) {
    if(str->bulbs[i].modified) {
      write_bulb(str->pin, i, &str->bulbs[i]);
    }
  }
  if(str->global_intensity_bulb.modified) {
    write_bulb(str->pin, 63, &str->global_intensity_bulb);
  }
}

/* 
 * Idle bus state: Low
 * Start Bit: High for 10µSeconds
 * 0 Bit: Low for 10µSeconds, High for 20µSeconds
 * 1 Bit: Low for 20µSeconds, High for 10µSeconds
 * Minimum quiet-time between frames: 30µSeconds
 *
 * Each frame is 26 bits long and has the following format:
 * 
 * Start bit
 * 6-Bit Bulb Address, MSB first
 * 8-Bit Brightness, MSB first
 * 4-Bit Blue, MSB first
 * 4-Bit Green, MSB first
 * 4-Bit Red, MSB first
 *
 * Source: http://www.deepdarc.com/2010/11/27/hacking-christmas-lights/
 */
#define DELAYLONG 20 
#define DELAYSHORT 10 
#define DELAYEND 30 
#define ZERO(__pin) digitalWrite(__pin,LOW); _delay_us(DELAYSHORT); digitalWrite(__pin,HIGH); _delay_us(DELAYLONG);
#define ONE(__pin) digitalWrite(__pin,LOW); _delay_us(DELAYLONG); digitalWrite(__pin,HIGH); _delay_us(DELAYSHORT);
static void write_bulb(uint8_t pin, uint8_t addr, bulb_type *bulb) {
  uint8_t R, G, B, intensity;
  R = (bulb->color & 0xf00) >> 8;
  G = (bulb->color & 0x0f0) >> 4;
  B = bulb->color & 0x00f;
  intensity = bulb->intensity > MAX_INTENSITY ? MAX_INTENSITY : bulb->intensity;

  bulb->modified = false;

  cli();

  // start bit
  digitalWrite(pin,HIGH);
  _delay_us(DELAYSHORT);

  // addr - 6 bits
  if (addr & 0x20) { ONE(pin) } else { ZERO(pin) };
  if (addr & 0x10) { ONE(pin) } else { ZERO(pin) };
  if (addr & 0x08) { ONE(pin) } else { ZERO(pin) };
  if (addr & 0x04) { ONE(pin) } else { ZERO(pin) };
  if (addr & 0x02) { ONE(pin) } else { ZERO(pin) };
  if (addr & 0x01) { ONE(pin) } else { ZERO(pin) };

  // intensity - 8 bits
  if (intensity & 0x80) { ONE(pin) } else { ZERO(pin) };
  if (intensity & 0x40) { ONE(pin) } else { ZERO(pin) };
  if (intensity & 0x20) { ONE(pin) } else { ZERO(pin) };
  if (intensity & 0x10) { ONE(pin) } else { ZERO(pin) };
  if (intensity & 0x08) { ONE(pin) } else { ZERO(pin) };
  if (intensity & 0x04) { ONE(pin) } else { ZERO(pin) };
  if (intensity & 0x02) { ONE(pin) } else { ZERO(pin) };
  if (intensity & 0x01) { ONE(pin) } else { ZERO(pin) };

  // blue - 4 bits
  if (B & 0x8) { ONE(pin) } else { ZERO(pin) };
  if (B & 0x4) { ONE(pin) } else { ZERO(pin) };
  if (B & 0x2) { ONE(pin) } else { ZERO(pin) };
  if (B & 0x1) { ONE(pin) } else { ZERO(pin) };

  // green - 4 bits
  if (G & 0x8) { ONE(pin) } else { ZERO(pin) };
  if (G & 0x4) { ONE(pin) } else { ZERO(pin) };
  if (G & 0x2) { ONE(pin) } else { ZERO(pin) };
  if (G & 0x1) { ONE(pin) } else { ZERO(pin) };

  // red - 4 bits
  if (R & 0x8) { ONE(pin) } else { ZERO(pin) };
  if (R & 0x4) { ONE(pin) } else { ZERO(pin) };
  if (R & 0x2) { ONE(pin) } else { ZERO(pin) };
  if (R & 0x1) { ONE(pin) } else { ZERO(pin) }; 

  digitalWrite(pin,LOW);
  _delay_us(DELAYEND);

  sei();
}
