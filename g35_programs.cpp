#include "g35_programs.h"
#define WHITE_PTR &colors[0]
#define YELLOW_PTR &colors[1]
#define BLUE_PTR &colors[2]
#define RED_PTR &colors[3]
#define AQUA_PTR &colors[4]
#define GREEN_PTR &colors[5]
#define FUCHSIA_PTR &colors[6]
#define NAVY_PTR &colors[7]
#define GREEN2_PTR &colors[8]
#define TEAL_PTR &colors[9]
#define ORANGE_PTR &colors[10]
#define PURPLE_PTR &colors[11]
#define RED2_PTR &colors[12]
void program1_setup(program_type*);
void program2_setup(program_type*);
void program3_setup(program_type*);
void program4_setup(program_type*);

void program_tick(program_type **program, unsigned long time) {
  program_type *program_ptr = *program;
  if(program_ptr->iterations == 0) {
    *program = program_ptr->next_program;
    (*program)->iterations = (*program)->total_iterations;
    (*program)->setup(*program);
    return;
  }
  if(time - program_ptr->last_time >= program_ptr->delay) {
    program_ptr->next_frame(program_ptr);
    program_ptr->last_time = time;
    write_strings();
  }
}

#define GET_DATA(PNAME) struct PNAME##_data *data = (PNAME##_data*) program->data

#define delay_DEFAULT_CFG {1000}
struct delay_data {
  uint16_t duration;
};
void delay_setup(program_type *program) {
  GET_DATA(delay);
  program->iterations = 0;
  delay(data->duration);
}
void delay_next_frame(program_type *program) {}

#define chase_DEFAULT_CFG {NO_COLOR, &colors[1], 2, 24, 1, 0xCC, 0, 0}
struct chase_data {
  uint16_t initial_fill;
  uint16_t *pattern;
  uint8_t pattern_len;
  uint8_t speed;
  int8_t direction;
  uint8_t intensity;
  uint8_t linear;
  uint8_t clear_behind;
  uint16_t last_color;
  uint8_t i;
};
void chase_setup(program_type *program) {
  GET_DATA(chase);
  data->i = data->direction == 1 ? 0 : num_bulbs-1;
  program->delay = 20;
  if(data->initial_fill != NO_COLOR) {
    fill_color(data->initial_fill);
    global_intensity(data->intensity);
  }
}
void chase_next_frame(program_type *program) {
  GET_DATA(chase);
  uint16_t color = data->pattern[data->i % data->pattern_len];
  set_bulb_with_intensity(data->i, color, data->intensity);
  if(data->clear_behind) {
    uint8_t last_i;
    if(data->direction == 1 && data->i > 0) {
      last_i = data->i - 1;
      set_bulb(last_i, data->last_color);
    } else if(data->direction == -1 && data->i < num_bulbs-1) {
      last_i = data->i + 1;
      set_bulb(last_i, data->last_color);
    }
  }

  if(data->i == 0 && data->direction == -1 || data->i+1 == num_bulbs && data->direction==1) {
    data->i = data->direction == 1 ? 0 : num_bulbs-1;
    program->iterations--;
    if(program->iterations > 0)
      shift_left();
  } else {
    data->i += data->direction;
    if(!data->linear)
      program->delay = (data->speed<<2) * data->i / num_bulbs;
  }
}

#define cycle_DEFAULT_CFG {colors, num_colors, 0}
struct cycle_data {
  uint16_t *pattern;
  uint8_t pattern_len;
  uint8_t i;
};
void cycle_setup(program_type *program) {
  program->delay = 700;
}
void cycle_next_frame(program_type *program) {
  GET_DATA(cycle);
  data->i++;
  if(data->i == data->pattern_len) data->i = 0;
  fill_color(data->pattern[data->i]);
  program->iterations--;
}

void fadeout_setup(program_type *program) {
  program->delay = 6;
  program->last_time = 0;
}
void fadeout_next_frame(program_type *program) {
  uint8_t high_intensity = dec_intensity();
  if(high_intensity == 0) {
    program->iterations = 0;
  }
}

#define fadein_to_DEFAULT_CFG {WHITE}
struct fadein_to_data {
  uint16_t color;
};
void fadein_to_setup(program_type *program) {
  GET_DATA(fadein_to);
  program->delay = 6;
  fill_color(data->color);
}
void fadein_to_next_frame(program_type *program) {
  uint8_t high_intensity = inc_intensity();
  if(high_intensity == 0xCC) {
    program->iterations = 0;
  }
}

struct blink_data {
  uint16_t active_c;
  uint16_t other_c;
};
void blink_setup(program_type *program) {
  struct blink_data *data;
  if(!program->data) {
    data = (struct blink_data *) malloc(sizeof(blink_data));
    program->data = data;
  } else {
    data = (blink_data *) program->data;
  }
  data->active_c = RED;
  data->other_c = WHITE;
  program->delay = 1000;
  program->last_time = 0;
  fill_color(data->active_c);
}
void blink_next_frame(program_type *program) {
  blink_data *data = (struct blink_data *) program->data;
  uint16_t tmp_c = data->active_c;
  data->active_c = data->other_c;
  data->other_c = tmp_c;
  fill_color(data->active_c);
  program->iterations--;
}

void twinkle_setup(program_type *program) {
  program->delay = 200;
  program->last_time = 0;
  repeat(colors, num_colors);
}
void twinkle_next_frame(program_type *program) {
  shift_left();
  program->iterations--;
}

#define wrap_DEFAULT_CFG {colors, num_colors}
struct wrap_data {
  uint16_t *colors;
  uint8_t colors_len;
};
void wrap_setup(program_type *program) {
  GET_DATA(wrap);
  program->delay = 700;
  program->last_time = 0;
  repeat(data->colors, data->colors_len);
}
void wrap_next_frame(program_type *program) {
  every_n_color(program->iterations % 2, 1, 1, RED);
  every_n_color(program->iterations % 2 == 0 ? 1 : 0, 1, 1, GREEN);
  program->iterations--;
}

#define throb_DEFAULT_CFG { 0xC0, 1, 25 }
struct throb_data {
  uint8_t amount;
  uint8_t do_shift;
  uint8_t speed;
  uint8_t count;
  uint8_t dir;
};
void throb_setup(program_type *program) {
  GET_DATA(throb);
  data->count = 0;
  data->dir = -1;
}
void throb_next_frame(program_type *program) {
  GET_DATA(throb);
  uint8_t intensity;
  if(data->dir == 1) {
    intensity = inc_intensity();
  } else {
    intensity = dec_intensity();
  }
  data->count++;
  unsigned long offset_intensity = 10 + intensity;
  program->delay = (uint8_t) ((unsigned long) 800 * data->speed / (offset_intensity*offset_intensity*offset_intensity));

  if(data->count == data->amount) {
    data->count = 0;
    if(data->dir == 1) {
      data->dir = -1;
      program->iterations--;
    } else {
      data->dir = 1;
      if(data->do_shift) {
        delay(100);
        shift_left();
        program->delay = 100;
      }
    }
  }
}



program_type program1 = {program1_setup};
program_type program2 = {program2_setup};
program_type program3 = {program3_setup};
program_type program4 = {program4_setup};
#define _P_(PNAME, ITER)                                                \
  struct PNAME##_data * c;                                              \
  do {                                                                  \
    pp = (program_type *) malloc(sizeof(program_type));                 \
    Serial.print("malloc=");                                            \
    Serial.println((uintptr_t) pp);                                     \
    *pp = (program_type) { PNAME##_setup, PNAME##_next_frame };         \
    p = p->next_program = pp;                                           \
    p->total_iterations = ITER;                                         \
    p->data = malloc(sizeof(PNAME##_data));                             \
    c = (PNAME##_data*) p->data;                                        \
    *c = (PNAME##_data) PNAME##_DEFAULT_CFG;                            \ 
  } while(0)
#define ADD_PROGRAM_SIMPLE(PNAME, ITER)                                 \
  do {                                                                  \
    pp = (program_type *) malloc(sizeof(program_type));                 \
    Serial.print("malloc=");                                            \
    Serial.println((uintptr_t) pp);                                     \
    *pp = (program_type) { PNAME##_setup, PNAME##_next_frame };         \
    p = p->next_program = pp;                                           \
    p->total_iterations = ITER;                                         \
  } while(0)
#define ADD_ACTION(PNAME) ADD_PROGRAM_SIMPLE(PNAME, 1)
#define PROGRAM_DELAY(X)                        \
      {                                         \
        _P_(delay, 1);                          \
        c->duration = X;                        \
      }                                         \

    void program1_setup(program_type *program) {
      program_type *p = program;
      program_type *pp;
      
      if(!program->next_program) {
        program->total_iterations = 0;
        program->iterations = 0;
    
        static uint16_t chase_colors[] = { YELLOW, BLUE, WHITE, BLACK };
        static uint16_t red = 0xa00;
        {_P_(chase, 1);
          c->pattern=chase_colors; c->pattern_len=1; c->direction=1;}
        PROGRAM_DELAY(300);
        {_P_(chase, 1);
          c->pattern = &chase_colors[1]; c->pattern_len = 1; c->direction = -1;}
        PROGRAM_DELAY(600);
        {_P_(chase, 1); 
          c->pattern = &chase_colors[2]; c->pattern_len = 2; c->direction = 1;}
        {_P_(chase, 1);
          c->pattern = &chase_colors[3]; c->pattern_len = 1; c->direction = 1;}
        {_P_(cycle, 10); p->delay = 600; }
        ADD_ACTION(fadeout);
        {_P_(fadein_to, 1);
          c->color = AQUA;}
        {_P_(chase, 1);
          c->pattern = colors; c->pattern_len = num_colors; c->direction = -1; c->speed = 40;}
        ADD_PROGRAM_SIMPLE(twinkle, 7);
        {_P_(chase, 1);
          c->pattern = &red; c->pattern_len = 1;}
        for(int i=0; i < 2; i++) {
          PROGRAM_DELAY(350);
          {_P_(throb, 2);
            c->amount = 0xb0; c->do_shift = 0; c->speed = 30;}
        }
        PROGRAM_DELAY(1000); 
        ADD_PROGRAM_SIMPLE(twinkle, 30);
        static uint16_t colors2[] = {NAVY, GREEN};
        for(int j=0;j<3;j++) {
          {_P_(chase, 1);
            c->pattern = &colors2[0]; c->pattern_len = 1; c->direction = -1; c->speed = 10;
            c->intensity = j == 0 ? 0x05 : j == 1 ? 0x50 : 0xCC;}
        }
        for(int j=0;j<3;j++) {
          {_P_(chase, 1);
            c->pattern = &colors2[1]; c->pattern_len = 1; c->direction = 1; c->speed = 10;
            c->intensity = j == 0 ? 0x05 : j == 1 ? 0x50 : 0xCC;}
        }
        ADD_ACTION(fadeout);
        for(int i=0; i < 4; i++) {
          {_P_(chase, 1);
            c->initial_fill = BLACK; c->pattern = RED_PTR; c->pattern_len = 1; c->direction = 1;
            c->speed = 9; c->clear_behind = 1;}
        } 
        p = p->next_program = program; // and loop
      } 
    }
    
    void program2_setup(program_type *program) {
      program_type *p = program;
      program_type *pp;
      
      if(!program->next_program) {
        program->total_iterations = 0;
        program->iterations = 0;
    
        static uint16_t wrap_colors[] = { BLUE, RED, GREEN };
        {_P_(wrap, -1);
          c->colors=wrap_colors; c->colors_len=3;}
          PROGRAM_DELAY(30000);
        p = p->next_program = program; // and loop
      }
    }
    
    void program3_setup(program_type *program) {
      program_type *p = program;
      program_type *pp;
      
      if(!program->next_program) {
        program->total_iterations = 0;
        program->iterations = 0;
    
        ADD_PROGRAM_SIMPLE(twinkle, -1);
    
        p = p->next_program = program; // and loop
      }
    }
    
    void program4_setup(program_type *program) {
      program_type *p = program;
      program_type *pp;
      
      if(!program->next_program) {
        program->total_iterations = 0;
        program->iterations = 0;
    
    static uint16_t wrap_colors[] = { GREEN };
        {_P_(cycle, -1); p->delay = 10000; c->pattern = wrap_colors; c->pattern_len=1;}
        PROGRAM_DELAY(30000);

        p = p->next_program = program; // and loop
      }
    }

//void program1_setup(program_type *program) {
//  program_type *p = program;
//  program_type *pp;
//
//  if(!program->next_program) {
//    program->total_iterations=0;
//    program->iterations = 0;
//    
//    static uint16_t wrap_colors[] = { YELLOW, GREEN };
//    {_P_(cycle, -1); p->delay = 10000; c->pattern = wrap_colors; c->pattern_len=2;}
//    PROGRAM_DELAY(3000);
//    p = p->next_program = program; // and loop
//  }
//}
//void program2_setup(program_type *program) {
//  program_type *p = program;
//  program_type *pp;
//
//  if(!program->next_program) {
//    program->total_iterations=0;
//    program->iterations = 0;
//    
//    static uint16_t wrap_colors[] = { YELLOW, GREEN };
//    static uint16_t chase_colors[] = { YELLOW, GREEN, BLACK };
//    static uint16_t red = 0xa00;
//    {_P_(chase, 1);
//      c->pattern=chase_colors; c->pattern_len=1; c->direction=1;}
//    PROGRAM_DELAY(300);
//    {_P_(chase, 1);
//      c->pattern = &chase_colors[1]; c->pattern_len = 1; c->direction = -1;}
//    PROGRAM_DELAY(300);
//    p = p->next_program = program; // and loop
//  }
//}
//void program3_setup(program_type *program) {
//  program_type *p = program;
//  program_type *pp;
//
//  if(!program->next_program) {
//    program->total_iterations=0;
//    program->iterations = 0;
//    
//    static uint16_t wrap_colors[] = { YELLOW, GREEN };
//    {_P_(wrap, -1);
//      c->colors=wrap_colors; c->colors_len=2;}
//    p = p->next_program = program; // and loop
//  }
//}
//void program4_setup(program_type *program) {
//  program_type *p = program;
//  program_type *pp;
//
//  if(!program->next_program) {
//    program->total_iterations=0;
//    program->iterations = 0;
//    
//    static uint16_t wrap_colors[] = { YELLOW, GREEN };
//    {_P_(wrap, -1);
//      c->colors=wrap_colors; c->colors_len=2;}
//    p = p->next_program = program; // and loop
//  }
//}
