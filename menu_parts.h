#ifndef MYY_MENU_PARTS_H
#define MYY_MENU_PARTS_H 1

#include <stdint.h>
#include <myy/helpers/position.h>
#include <myy/helpers/c_types.h>

enum menu_part_register_input_arch {
	menu_part_register_input_arch_arm64,
	n_menu_part_register_input_arch,
};

enum menu_part_register_input_arm64_type {
	menu_part_register_input_arm64_x_reg,
	menu_part_register_input_arm64_w_reg,
	menu_part_register_input_arm64_v_reg
};

enum menu_part_register_input_selection {
	menu_part_register_input_arm64_x_regs_only,
	menu_part_register_input_arm64_w_regs_only,
	menu_part_register_input_arm64_all_standard_regs,
	menu_part_register_input_arm64_vector_regs_only,
	menu_part_register_input_arm64_all_regs,
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

bool menu_part_type_valid(enum menu_part_type type)
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

struct arm64_mov_menu {
	union menu_part to_label;
	union menu_part to_register;
	union menu_part from_label;
	union menu_part from_register;
	union menu_part end;
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

#endif
