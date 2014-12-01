#ifndef G35_PROGRAMS_H
#define G35_PROGRAMS_H
#include "g35.h"

typedef struct program_type {
  // setup needs to be able to run multiple times -- take care with memory!
  void (*setup)(program_type *program);
  void (*next_frame)(program_type *program);
  program_type *next_program;

  uint16_t delay;
  unsigned long last_time;
  uint16_t total_iterations;
  uint16_t iterations;
  void *data;
} program_type;



void program_tick(program_type **program, unsigned long time);

extern program_type program1;
extern program_type program2;
extern program_type program3;
extern program_type program4;

#endif
