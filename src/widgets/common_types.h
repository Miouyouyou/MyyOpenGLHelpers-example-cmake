#ifndef MYY_WIDGETS_COMMON_TYPES_H
#define MYY_WIDGETS_COMMON_TYPES_H 1

#include <stdint.h>
#include <myy/helpers/vector.h>

typedef void * gpu_buffer_offset_t;

myy_vector_template(utf8, uint8_t)
typedef myy_vector_utf8 myy_utf8_string;

#define GLOBAL_BACKGROUND_COLOR 0.2f, 0.5f, 0.8f, 1.0f
#define DEFAULT_OFFSET_4D position_S_4D_struct(0,0,0,0)

#endif
