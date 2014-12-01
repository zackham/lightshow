#include "g35_programs.h"

// #define DEBUG

volatile boolean button_down = false;
void on_button_press() { button_down = true; }

void setup() {
  // right now i have one string with 25 bulbs
  pinMode(2, OUTPUT);
  add_light_string(2, 25);

  // debugging is cool

  // use the button to initialize the lights (easier this way, until they share power source)
  attachInterrupt(1, on_button_press, RISING);

  Serial.begin(9600);

  Serial.println("waiting");
  // wait for the initial button press to initialize the lights.
  while(!button_down) { delay(50); }
  Serial.println("assigning addresses");
  assign_bulb_addresses();
  Serial.println("assigned...");

  //  uint8_t program_i = 0;
  //  uint8_t num_programs = sizeof(demo) / sizeof(measure_t);
  //  Serial.print("Num programs: ");
  //  Serial.println(num_programs);
  //  measure_t program = demo[program_i];
  //program_type *programs[] = { &program1, &program2, &program3, &program4 };
  program_type *programs[] = { &program2, &program3, &program4 };
  uint8_t i = 0;
  program_type *program = programs[i];
  unsigned long time;
  uint16_t delayms;
  program->setup(program);
  Serial.println("running program");
  while(1) {
    if(button_down) {
      Serial.println("changing program");
      button_down = false;
      i = (i+1) % (sizeof(programs)/sizeof(programs[0]));
      program = programs[i];
      program->setup(program);
    }
    program_tick(&program, millis());
    delay(program->delay);
  }
}

void loop() {} 
