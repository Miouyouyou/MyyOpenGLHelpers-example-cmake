#ifndef MYY_WIDGETS_TEXT_AREA_H
#define MYY_WIDGETS_TEXT_AREA_H 1

#include <src/widgets/common_types.h>

#include <src/widgets/text_buffer.h>

#include <stdint.h>

#include <myy/helpers/c_types.h>
#include <myy/helpers/position.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>


/* TODO
 * 128 octets for a simple text area !?
 * 
 * That kind of worries me.
 * Somehow, we can't just share the text_buffer like this
 * since we have to be able to refresh it at any time.
 * If it's shared, then all the parts sharing this buffer
 * should be alerted in order to rebuild the content they
 * stored previously...
 * Kind of diffcult...
 * 
 * The properties however could be shared. That said,
 * the properties don't take THAT much space... (28 octets)
 * 
 * Hmm...
 * 
 * I might be overthinking this.
 */
struct myy_text_area {
	myy_vector_utf8 value;
	/* TODO
	 * Display is a terrible name. Change to something else.
	 * This contain drawing properties, so maybe draw_props ?
	 */
	struct text_buffer display;
	position_S position;
	struct myy_text_properties default_properties;
	uint64_t reserved;
};

bool myy_text_area_init(
	struct myy_text_area * __restrict const text_area,
	struct gl_text_infos * __restrict const atlas,
	position_S position);

void myy_text_area_reset(
	struct myy_text_area * __restrict const text_area);

void myy_text_area_draw(
	struct myy_text_area * __restrict const text_area);

bool myy_text_area_append_text_c_string(
	struct myy_text_area * __restrict const text_area,
	char const * __restrict const c_string);

bool myy_text_area_append_text_utf8_characters(
	struct myy_text_area * __restrict const text_area,
	size_t const n_characters,
	uint8_t const * __restrict const characters);

bool myy_text_area_set_text_c_string(
	struct myy_text_area * __restrict const text_area,
	char const * __restrict const c_string);

bool myy_text_area_set_text_utf8_characters(
	struct myy_text_area *  __restrict const text_area,
	size_t const n_characters,
	uint8_t const * __restrict const utf8_characters);

#endif
