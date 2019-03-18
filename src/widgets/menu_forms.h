#ifndef MYY_WIDGETS_MENU_FORMS_H
#define MYY_WIDGETS_MENU_FORMS_H 1

#include <src/widgets/text_buffer.h>

#include <myy/current/opengl.h>
#include <myy/helpers/dimensions.h>
#include <myy/helpers/matrices.h>
#include <myy/helpers/position.h>
#include <src/widgets/simple_forms.h>

#include <shaders.h>

struct menu_forms {
	GLuint n_points;
	myy_vector_rgb_points cpu_buffer;
	GLuint gpu_buffer;
	position_S_4D offset;
} test_menu;

/* TODO DOCUMENT !
 * IIRC, the whole point was to have menu elements, with
 * predetermined functions for setting up and drawing.
 * All the buffers are shared, in order to reduce the
 * amount of draw calls to draw the menu.
 */
struct menu_parts_handler {
	position_S_4D pos;
	struct text_buffer static_text;
	struct menu_forms forms;
	struct text_buffer input_text;
};

enum menu_part_type {
	menu_part_type_invalid,
	menu_part_type_label,
	menu_part_type_toggle_button,
	menu_part_type_input_register,
	menu_part_type_input_numeric,
	menu_part_type_end,
	n_menu_part_type,
};

__attribute__((unused))
static inline bool menu_part_type_valid(enum menu_part_type type)
{
	return
		(type > menu_part_type_invalid) &
		(type < n_menu_part_type);
}

struct menu_part_header {
	enum menu_part_type type;
	position_S_4D pos;
};

struct menu_part_label {
	enum menu_part_type type;
	position_S_4D pos;
	uint8_t const * __restrict text;
};

struct menu_part_toggle_button {
	enum menu_part_type type;
	position_S_4D pos;
	uint8_t left_or_right; // casted `enum menu_toggle_position`
	uint8_t const * __restrict left_text;
	uint8_t const * __restrict right_text;
};

struct menu_part_input_register {
	enum menu_part_type type;
	position_S_4D pos;
	uint16_t arch;
	uint16_t reg_value;
	uint16_t reg_type;
	uint16_t reg_list_type;
};

struct menu_part_input_numeric {
	enum menu_part_type type;
	position_S_4D pos;
	int32_t n_chars_max;
	int32_t min_value;
	int32_t max_value;
};

struct menu_part_end {
	enum menu_part_type type;
	position_S_4D pos;
};

union menu_part {
	struct menu_part_label          label;
	struct menu_part_toggle_button  toggle_button;
	struct menu_part_input_register input_register;
	struct menu_part_input_numeric  input_numeric;
	struct menu_part_end            menu_end;
};

#define MENU_LABEL(position, label_text) {\
	.label = { \
		.type = menu_part_type_label,\
		.pos  = position,\
		.text = label_text\
	}\
}

#define MENU_INPUT_REGISTER(position, register_prefix) {\
	.input_numeric = { \
		.type = menu_part_type_input_numeric,\
		.pos  = position,\
		.n_chars_max = 2, \
		.min_value = 0, \
		.max_value = 31 \
	}\
}

#define MENU_END .menu_end = { .type = menu_part_type_end, .pos = {0,0,0,1} }

static inline void menu_forms_set_projection(
	union myy_4x4_matrix const * __restrict const projection)
{
	glUseProgram(myy_programs.menu_forms_id);
	glUniformMatrix4fv(
		myy_programs.menu_forms_unif_projection,
		1,
		GL_FALSE,
		projection->raw_data);
	glUseProgram(0);
}

void menu_forms_init(
	struct menu_forms * __restrict const forms);
void menu_forms_reset(
	struct menu_forms * __restrict const forms);
void menu_forms_store_to_gpu(
	struct menu_forms * __restrict const forms);
void menu_forms_draw(
	struct menu_forms * __restrict const forms);
static inline void menu_forms_set_global_position(
	struct menu_forms * __restrict const forms,
	position_S_4D position)
{
	forms->offset = position;
}
void menu_forms_add_arrow_left(
	struct menu_forms * __restrict const forms,
	position_S const pos,
	struct rgba8 const color);
void menu_forms_add_arrow_right(
	struct menu_forms * __restrict const forms,
	position_S const pos,
	struct rgba8 const color);
void menu_forms_add_bordered_rectangle(
	struct menu_forms * __restrict const forms,
	position_S const down_left, dimensions_S dimensions,
	struct rgba8 const color, struct rgba8 borders_color);
void menu_parts_handler_init(
	struct menu_parts_handler * __restrict const handler,
	struct gl_text_infos * __restrict const text_atlas_properties,
	uint32_t surface_width, uint32_t surface_height);
static inline void menu_parts_handler_set_draw_position(
	struct menu_parts_handler * __restrict const handler,
	position_S_4D pos)
{
	handler->pos = pos;
}

void menu_parts_reset(
	struct menu_parts_handler * __restrict const handler);
void menu_parts_store_to_gpu(
	struct menu_parts_handler * __restrict const handler);
void menu_parts_draw(
	struct menu_parts_handler * __restrict const handler);
void menu_parts_handler_generate_menu(
	union menu_part const * __restrict parts,
	struct menu_parts_handler * __restrict const handler,
	uint32_t surface_width, uint32_t surface_height);

#endif
