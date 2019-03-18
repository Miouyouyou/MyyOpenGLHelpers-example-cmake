#ifndef MYY_MENU_PARTS_H
#define MYY_MENU_PARTS_H 1

#include <stdint.h>
#include <myy/helpers/position.h>
#include <myy/helpers/c_types.h>

#include <src/widgets/menu_forms.h>

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

struct arm64_mov_menu {
	union menu_part to_label;
	union menu_part to_register;
	union menu_part from_label;
	union menu_part from_register;
	union menu_part end;
};


#endif
