#include <src/widgets/text_area.h>
#include <src/widgets/common_types.h>
#include <src/widgets/text_buffer.h>

#include <stdint.h>

#include <myy/helpers/c_types.h>
#include <myy/helpers/position.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>

bool myy_text_area_init(
	struct myy_text_area * __restrict const text_area,
	struct gl_text_infos * __restrict const atlas,
	position_S position)
{
	text_area->value = myy_vector_utf8_init(4096);
	text_buffer_init(&text_area->display, atlas);
	text_area->position = position;

	return myy_vector_utf8_is_valid(&text_area->value);
}

void myy_text_area_reset(
	struct myy_text_area * __restrict const text_area)
{
	myy_vector_utf8_reset(&text_area->value);
	text_buffer_reset(&text_area->display);
}

void myy_text_area_draw(
	struct myy_text_area * __restrict const text_area)
{
	text_buffer_draw(&text_area->display);
}







