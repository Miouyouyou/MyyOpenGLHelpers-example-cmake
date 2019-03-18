#ifndef MYY_WIDGETS_TEXT_AREA_H
#define MYY_WIDGETS_TEXT_AREA_H 1

#include <src/widgets/common_types.h>

#include <src/widgets/text_buffer.h>

#include <stdint.h>

#include <myy/helpers/c_types.h>
#include <myy/helpers/position.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>


struct myy_text_area {
	myy_vector_utf8 value;
	/* TODO Display is a terrible name. Change to something else.
	 *      This contain drawing properties, so maybe draw_props ?
	 */
	struct text_buffer display;
	position_S position;
};

bool myy_text_area_init(
	struct myy_text_area * __restrict const text_area,
	struct gl_text_infos * __restrict const atlas,
	position_S position);

void myy_text_area_reset(
	struct myy_text_area * __restrict const text_area);


void myy_text_area_draw(
	struct myy_text_area * __restrict const text_area);


#endif
