#ifndef MYY_WIDGETS_TEXT_BUFFER_H
#define MYY_WIDGETS_TEXT_BUFFER_H 1

#include <stdint.h>
#include <myy/current/opengl.h>
#include <myy/helpers/position.h>
#include <myy/helpers/fonts/packed_fonts_display.h>

struct gl_text_vertex {
	int16_t x, y;
	uint16_t s, t;
};

union gl_text_character {
	struct {
		struct {
			struct gl_text_vertex up_left, down_left, up_right;
		} first;
		struct {
			struct gl_text_vertex down_right, up_right, down_left;
		} second;
	} triangles;
	struct gl_text_vertex points[6];
	uint16_t raw[24];
};
myy_vector_template(gl_chars, union gl_text_character);


struct text_buffer {
	GLuint points;
	myy_vector_gl_chars cpu_buffer;
	GLuint gpu_buffer;
	position_S_4D offset;
	struct myy_rectangle offset_limits;
	struct gl_text_infos * text_display_atlas;
} menu_text;

__attribute__((unused))
static inline void store_to_gl_string(
	myy_vector_gl_chars * __restrict const gl_string,
	struct myy_gl_text_quad const * __restrict const quad)
{
	union gl_text_character character;
	{
		struct gl_text_vertex up_left = {
			quad->left, quad->up, quad->tex_left, quad->tex_up
		};
		character.triangles.first.up_left = up_left;
	}
	{
		struct gl_text_vertex down_left = {
			quad->left, quad->down, quad->tex_left, quad->tex_down
		};
		character.triangles.first.down_left = down_left;
	}
	{
		struct gl_text_vertex up_right = {
			quad->right, quad->up, quad->tex_right, quad->tex_up
		};
		character.triangles.first.up_right = up_right;
	}
	{
		struct gl_text_vertex down_right = {
			quad->right, quad->down, quad->tex_right, quad->tex_down
		};
		character.triangles.second.down_right = down_right;
	}
	{
		struct gl_text_vertex up_right = {
			quad->right, quad->up, quad->tex_right, quad->tex_up
		};
		character.triangles.second.up_right = up_right;
	}
	{
		struct gl_text_vertex down_left = {
			quad->left, quad->down, quad->tex_left, quad->tex_down
		};
		character.triangles.second.down_left = down_left;
	}
	myy_vector_gl_chars_add(gl_string, 1, &character);
}

void text_buffer_init(
	struct text_buffer * __restrict const text_buf,
	struct gl_text_infos * __restrict const text_atlas_properties);

__attribute__((unused))
static inline void text_buffer_set_offset_limits(
	struct text_buffer * __restrict const text_buf,
	struct myy_rectangle const limits)
{
	text_buf->offset_limits = limits;
}

void text_buffer_move(
	struct text_buffer * __restrict const text_buf,
	position_S relative_move);

void text_buffer_reset(
	struct text_buffer * __restrict const text_buf);

void text_buffer_add_cb(
	void * text_buf,
	struct myy_gl_text_quad const * __restrict const quads,
	uint32_t const n_quads,
	struct myy_text_properties const * __restrict const props);

void text_buffer_add_string(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const utf8_string,
	position_S * __restrict const position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties);

void text_buffer_add_strings_list(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const * __restrict const utf8_strings,
	position_S * __restrict const start_position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties,
	int16_t vertical_offset_between_strings);

void text_buffer_add_n_chars_from_string(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const utf8_string,
	size_t const utf8_string_size,
	position_S * __restrict const position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties);

void text_buffer_store_to_gpu(
	struct text_buffer * __restrict const gl_text_buffer);

static inline void text_buffer_set_global_position(
	struct text_buffer * __restrict const text_buf,
	position_S_4D position)
{
	text_buf->offset = position;
}

void text_buffer_draw(struct text_buffer const * __restrict const text_buf);


#endif
