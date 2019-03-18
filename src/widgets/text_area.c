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
	struct myy_text_properties default_props = {
		.myy_text_flows = ((block_top_to_bottom << 8) | line_left_to_right),
		.z_layer = 16,
		.r = 255, .g = 255, .b = 255, .a = 255,
		.user_metadata = NULL		
	};
	text_area->default_properties = default_props;

	return myy_vector_utf8_is_valid(&text_area->value);
}

static void myy_text_area_regen_display(
	struct myy_text_area * __restrict const text_area)
{
	struct text_buffer * __restrict const text_buffer =
		&text_area->display;
	position_S position = text_area->position;
	text_buffer_reset(text_buffer);
	/* TODO Add a version that take myy_utf8_string as argument */
	text_buffer_add_n_chars_from_string(
		text_buffer,
		myy_vector_utf8_data(&text_area->value),
		myy_vector_utf8_length(&text_area->value),
		&position,
		&text_area->default_properties);
	text_buffer_store_to_gpu(text_buffer);
}

void myy_text_area_reset(
	struct myy_text_area * __restrict const text_area)
{
	myy_vector_utf8_reset(&text_area->value);
}

bool myy_text_area_append_text_utf8_characters(
	struct myy_text_area * __restrict const text_area,
	size_t const n_characters,
	uint8_t const * __restrict const characters)
{
	/* TODO UTF-8 checks should be done here */

	/* Vector add fail if not enough memory.
	 * In which case, it won't touch the memory and
	 * we will just redraw the same content.
	 */
	bool ret = myy_vector_utf8_add(
		&text_area->value,
		n_characters,
		characters);

	/* TODO
	 * Even if the first doesn't fail, this one
	 * could still fail and lead to some situations
	 * where the null byte can't be relied upon.
	 * Should we rely upon this anyway ?
	 */
	uint8_t const terminating_null_byte = 0;
	myy_vector_utf8_add(
		&text_area->value,
		1,
		&terminating_null_byte);

	myy_text_area_regen_display(text_area);
	return ret;
}

bool myy_text_area_set_text_utf8_characters(
	struct myy_text_area *  __restrict const text_area,
	size_t const n_characters,
	uint8_t const * __restrict const utf8_characters)
{
	/* TODO UTF-8 checks should be done here */
	myy_text_area_reset(text_area);
	return myy_text_area_append_text_utf8_characters(
		text_area, n_characters, utf8_characters);
}

bool myy_text_area_append_text_c_string(
	struct myy_text_area * __restrict const text_area,
	char const * __restrict const c_string)
{
	return myy_text_area_append_text_utf8_characters(
		text_area,
		strlen(c_string),
		(uint8_t * __restrict) c_string);

}

bool myy_text_area_set_text_c_string(
	struct myy_text_area * __restrict const text_area,
	char const * __restrict const c_string)
{
	/* TODO Char == Octet isn't guaranteed at all
	 * It works on Intel and ARM architectures though
	 */
	return myy_text_area_set_text_utf8_characters(
		text_area, strlen(c_string),
		(uint8_t const * __restrict) c_string);
}

void myy_text_area_draw(
	struct myy_text_area * __restrict const text_area)
{
	text_buffer_draw(&text_area->display);
}

