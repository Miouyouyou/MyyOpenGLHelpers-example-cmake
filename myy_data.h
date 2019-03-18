#ifndef MYY_DATA_H
#define MYY_DATA_H 1

#include <stdint.h>

uint8_t const * __restrict const supported_armv8_instructions[] = {
	(uint8_t const *) "ADC",
	(uint8_t const *) "ADD",
	(uint8_t const *) "ADD - Shifted register",
	(uint8_t const *) "ADD - Immediate",
	(uint8_t const *) "ADR - Immediate",
	(uint8_t const *) "ADR - 4Kb Page alignment",
	(uint8_t const *) "AND - Immediate",
	(uint8_t const *) "ASR",
	(uint8_t const *) "B",
	(uint8_t const *) "B - Conditional",
	(uint8_t const *) "BL",
	(uint8_t const *) "BLR",
	(uint8_t const *) "BR",
	(uint8_t const *) "RET",
	(uint8_t const *) "CBZ CBNZ",
	(uint8_t const *) "CCM - Immediate",
	(uint8_t const *) "CCM - Register",
	(uint8_t const *) "DIV",
	(uint8_t const *) "LSL - Register",
	(uint8_t const *) "LSR - Register",
	(uint8_t const *) "LDR - Register",
	(uint8_t const *) "LDR - Register",
	(uint8_t const *) "LDR - Immediate",
	(uint8_t const *) "LDR - Unsigned immediate",
	(uint8_t const *) "LDP",
	(uint8_t const *) "MADD",
	(uint8_t const *) "MOV - Register",
	(uint8_t const *) "MOVK",
	(uint8_t const *) "MOVN",
	(uint8_t const *) "MOVW",
	(uint8_t const *) "MOVZ",
	(uint8_t const *) "MSUB",
	(uint8_t const *) "MUL",
	(uint8_t const *) "MVN - Shifted register",
	(uint8_t const *) "NEG",
	(uint8_t const *) "NGC",
	(uint8_t const *) "SBC",
	(uint8_t const *) "STR - Register",
	(uint8_t const *) "STR - Immediate",
	(uint8_t const *) "STR - Unsigned immediate",
	(uint8_t const *) "STP",
	(uint8_t const *) "SUB",
	(uint8_t const *) "SVC",
	NULL
};

struct arm64_mov_menu mov_menu = {
	.to_label      =
		MENU_LABEL(POSITION_4D_SIMPLE(20,100,0), (uint8_t const *) "To"),
	.to_register   =
		MENU_INPUT_REGISTER(POSITION_4D_SIMPLE(70,104, 0), "X"),
	.from_label    = 
		MENU_LABEL(POSITION_4D_SIMPLE(170,100,0), (uint8_t const *) "From"),
	.from_register =
		MENU_INPUT_REGISTER(POSITION_4D_SIMPLE(220,104,0), "X")
};

#endif
