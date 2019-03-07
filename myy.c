#include <myy/myy.h>
#include <myy/current/opengl.h>
#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/opengl/shaders_pack.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/matrices.h>

#include <myy/helpers/opengl/buffers.h>

#include <myy/helpers/position.h>

#include <menu_parts.h>

#include <string.h>

#include "shaders.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


#define GLOBAL_BACKGROUND_COLOR 0.2f, 0.5f, 0.8f, 1.0f
#define DEFAULT_OFFSET_4D position_S_4D_struct(0,0,0,0)



typedef void * gpu_buffer_offset_t;

GLuint glbuffer_text;
GLuint n_points_for_text;

struct dimensions_S_ {
	uint16_t width, height;
};

typedef struct dimensions_S_ dimensions_S;

__attribute__((unused))
static inline dimensions_S dimensions_S_struct(
	uint16_t const width, uint16_t const height)
{
	dimensions_S dimensions = { .width = width, .height = height };
	return dimensions;
}

struct rgba8 {
	uint8_t r, g, b, a;
};
static inline struct rgba8 rgba8_color(
	uint8_t const r, uint8_t const g, uint8_t const b, uint8_t const a)
{
	struct rgba8 color = { .r = r, .g = g, .b = b, .a = a };
	return color;
}

struct simple_rgb_point {
	position_S pos;
	int16_t z, w;
	struct rgba8 color;
};
myy_vector_template(rgb_points, struct simple_rgb_point)

static inline struct simple_rgb_point simple_rgb_point_struct(
	position_S const pos, int16_t depth,
	struct rgba8 const color)
{
	struct simple_rgb_point const point = {
		.pos   = pos,
		.color = color,
		.z     = depth,
		.w     = 1
	};

	return point;
}

static inline void simple_rgb_triangle(
	myy_vector_rgb_points * __restrict const points,
	int16_t depth,
	position_S const a, position_S const b, position_S const c,
	struct rgba8 const color)
{
	struct simple_rgb_point triangle_vertices[3] = {
		simple_rgb_point_struct(a, depth, color),
		simple_rgb_point_struct(b, depth, color),
		simple_rgb_point_struct(c, depth, color)
	};
	myy_vector_rgb_points_add(points, 3, triangle_vertices);
}

static inline void simple_rgb_quad(
	myy_vector_rgb_points * __restrict const points,
	int16_t const depth,
	position_S const up_left, position_S const down_right,
	struct rgba8 const color)
{
	position_S const down_left = {
		.x = up_left.x,
		.y = down_right.y
	};
	position_S const up_right = {
		.x = down_right.x,
		.y = up_left.y
	};
	struct simple_rgb_point two_triangles_quad_vertices[6] = {
		simple_rgb_point_struct(up_left, depth, color),
		simple_rgb_point_struct(down_left, depth, color),
		simple_rgb_point_struct(up_right, depth, color),

		simple_rgb_point_struct(down_right, depth, color),
		simple_rgb_point_struct(up_right, depth, color),
		simple_rgb_point_struct(down_left, depth, color)
	};

	myy_vector_rgb_points_add(points, 6, two_triangles_quad_vertices);
}

struct menu_forms {
	GLuint n_points;
	myy_vector_rgb_points cpu_buffer;
	GLuint gpu_buffer;
	position_S_4D offset;
} test_menu;

static inline void menu_forms_init(
	struct menu_forms * __restrict const forms)
{
	forms->n_points = 0;
	forms->cpu_buffer = myy_vector_rgb_points_init(256);
	glGenBuffers(1, &forms->gpu_buffer);
	forms->offset = DEFAULT_OFFSET_4D;
}

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

static inline void menu_forms_reset(
	struct menu_forms * __restrict const forms)
{
	myy_vector_rgb_points_reset(&forms->cpu_buffer);
}

static inline void menu_forms_store_to_gpu(
	struct menu_forms * __restrict const forms)
{
	myy_vector_rgb_points * __restrict const points =
		&forms->cpu_buffer;

	glBindBuffer(GL_ARRAY_BUFFER, forms->gpu_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		myy_vector_rgb_points_allocated_used(points),
		myy_vector_rgb_points_data(points),
		GL_DYNAMIC_DRAW);

	forms->n_points = myy_vector_rgb_points_length(points);
}

static inline void menu_forms_draw(
	struct menu_forms * __restrict const forms)
{
	glUseProgram(myy_programs.menu_forms_id);
	glUniform4f(myy_programs.menu_forms_unif_global_offset,
		forms->offset.x,
		forms->offset.y,
		forms->offset.z,
		forms->offset.w);
	glBindBuffer(GL_ARRAY_BUFFER, forms->gpu_buffer);
	glVertexAttribPointer(
		menu_forms_xyz, 4, GL_SHORT, GL_FALSE,
		myy_vector_rgb_points_type_size(),
		(gpu_buffer_offset_t)
		(offsetof(struct simple_rgb_point, pos)));
	glVertexAttribPointer(
		menu_forms_in_color, 4, GL_UNSIGNED_BYTE, GL_TRUE,
		myy_vector_rgb_points_type_size(),
		(gpu_buffer_offset_t)
		(offsetof(struct simple_rgb_point, color)));

	glDrawArrays(GL_TRIANGLES, 0, forms->n_points);
	glUseProgram(0);
}

static inline void menu_forms_set_global_position(
	struct menu_forms * __restrict const forms,
	position_S_4D position)
{
	forms->offset = position;
}

static inline void menu_forms_add_arrow_left(
	struct menu_forms * __restrict const forms,
	position_S const pos,
	struct rgba8 const color)
{
	myy_vector_rgb_points * __restrict const forms_buffer =
		&forms->cpu_buffer;
	position_S const a = {
		.x = 0+pos.x,
		.y = 16+pos.y
	};
	position_S const b = {
		.x = 32+pos.x,
		.y = 32+pos.y
	};
	position_S const c = {
		.x = 32+pos.x,
		.y = 0+pos.y
	};
	int16_t const depth = 0;

	simple_rgb_triangle(forms_buffer, depth, a, b, c, color);
}

static inline void menu_forms_add_arrow_right(
	struct menu_forms * __restrict const forms,
	position_S const pos,
	struct rgba8 const color)
{
	myy_vector_rgb_points * __restrict const forms_buffer =
		&forms->cpu_buffer;
	position_S const a = {
		.x = 0+pos.x,
		.y = 0+pos.y
	};
	position_S const b = {
		.x = 0+pos.x,
		.y = 32+pos.y
	};
	position_S const c = {
		.x = 32+pos.x,
		.y = 16+pos.y
	};
	int16_t const depth = 0;

	simple_rgb_triangle(forms_buffer, depth, a, b, c, color);
}

static inline void menu_forms_add_bordered_rectangle(
	struct menu_forms * __restrict const forms,
	position_S const up_left, dimensions_S dimensions,
	struct rgba8 const color, struct rgba8 borders_color)
{
	myy_vector_rgb_points * __restrict const forms_buffer =
		&forms->cpu_buffer;

	position_S const down_right = {
		.x = up_left.x + dimensions.width,
		.y = up_left.y + dimensions.height
	};
	int16_t zone_depth = 0;

	simple_rgb_quad(forms_buffer, zone_depth, up_left, down_right, color);

	position_S const border_up_left = {
		.x = up_left.x - 1,
		.y = up_left.y - 1
	};
	position_S const border_down_right = {
		 // +1 to compensate the -1, +1 for the border
		.x = border_up_left.x + dimensions.width + 2,
		.y = border_up_left.y + dimensions.height + 2
	};

	int16_t const border_depth = 1; // Greater mean behind
	simple_rgb_quad(forms_buffer, border_depth,
		border_up_left, border_down_right, borders_color);
}

struct stencil_vertex {
	int16_t x, y;
};
union stencil_triangle {
	struct {
		struct stencil_vertex a, b, c;
	} points;
	uint16_t raw[6];
};
myy_vector_template(stencil_triangles, union stencil_triangle)


struct simple_stencil {
	GLuint points;
	myy_vector_stencil_triangles cpu_buffer;
	GLuint gpu_buffer;
} menu_stencil;
void simple_stencil_init(
	struct simple_stencil * __restrict const simple_stencil_object)
{
	simple_stencil_object->cpu_buffer = 
		myy_vector_stencil_triangles_init(128);
	glGenBuffers(1, &simple_stencil_object->gpu_buffer);
	glUseProgram(myy_programs.simple_stencil_id);
	glEnableVertexAttribArray(simple_stencil_xy);
	glUseProgram(0);
}
void simple_stencil_start_preparing(
	struct simple_stencil * __restrict const simple_stencil_object)
{
	myy_vector_stencil_triangles_reset(&simple_stencil_object->cpu_buffer);
}
void simple_stencil_done_preparing(
	struct simple_stencil * __restrict const simple_stencil_object)
{
	myy_vector_stencil_triangles * __restrict const stencil_triangles =
		&simple_stencil_object->cpu_buffer;
	glBindBuffer(GL_ARRAY_BUFFER, simple_stencil_object->gpu_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		myy_vector_stencil_triangles_allocated_used(stencil_triangles),
		myy_vector_stencil_triangles_data(stencil_triangles),
		GL_DYNAMIC_DRAW);
	simple_stencil_object->points =
		myy_vector_stencil_triangles_length(stencil_triangles) * 3;
}
static inline void simple_stencil_set_projection(
	union myy_4x4_matrix const * __restrict const projection_matrix)
{
	glUseProgram(myy_programs.simple_stencil_id);
	glUniformMatrix4fv(
		myy_programs.simple_stencil_unif_projection,
		1,
		GL_FALSE,
		projection_matrix->raw_data);
	glUseProgram(0);
}
GLuint simple_stencil_stored_points(
	struct simple_stencil const * __restrict const simple_stencil_object)
{
	return 
		myy_vector_stencil_triangles_length(
			&simple_stencil_object->cpu_buffer)
		* 3;
}

void simple_stencil_apply(
	struct simple_stencil const * __restrict const simple_stencil_object)
{
	glUseProgram(myy_programs.simple_stencil_id);
	glEnable(GL_STENCIL_TEST);

	/* Disable the depth tests for the moment */
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	/* Clear our stencil */
	glClear(GL_STENCIL_BUFFER_BIT);

	/* Increase every pixel of the stencil buffer when drawing on it.
	 * But act as a shield and don't leak anything on the framebuffer.
	 * 
	 * That way, the drawings will set our stencil zone.
	 */
	glStencilFunc(GL_NEVER, 0, 0xFF);
	// zFail, zPass will never happen with the Depth test disabled
	glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);

	/* Draw our stencil triangles */
	glBindBuffer(GL_ARRAY_BUFFER, simple_stencil_object->gpu_buffer);
	glVertexAttribPointer(simple_stencil_xy,
		2, GL_SHORT, GL_FALSE, 2*sizeof(int16_t),
		(gpu_buffer_offset_t) 0);
	glDrawArrays(GL_TRIANGLES, 0, simple_stencil_object->points);
	glUseProgram(0);

	/* Re-enable the Depth Test */
	glEnable(GL_DEPTH_TEST);

	/* Draw anything that goes through the stencil zone.
	 * Don't draw anything outside.
	 * 
	 * The Stencil test pass if :
	 * (0 & 0xFF) < (stencil_value & 0xff)
	 * 
	 * stencil_value will be superior to 0 if something
	 * was drawn on it before.
	 */
	glStencilFunc(GL_LESS, 0, 0xFF);
	// Make the stencil constant for now
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
}

void simple_stencil_put_away(
	struct simple_stencil const * __restrict const simple_stencil_object)
{
	glDisable(GL_STENCIL_TEST);
}

void simple_stencil_store_rectangle(
	struct simple_stencil * __restrict const simple_stencil_object,
	position_S up_left,
	position_S down_right)
{
	myy_vector_stencil_triangles * __restrict const stencil_triangles =
		&simple_stencil_object->cpu_buffer;
	position_S up_right  = { .x = down_right.x, .y = up_left.y };
	position_S down_left = { .x = up_left.x,    .y = down_right.y };

	union stencil_triangle const first = {
		.points = {
			.a = {
				.x = up_left.x,
				.y = up_left.y
			},
			.b = {
				.x = down_left.x,
				.y = down_left.y
			},
			.c = {
				.x = up_right.x,
				.y = up_right.y
			}
		}
	};

	myy_vector_stencil_triangles_add(stencil_triangles, 1, &first);

	union stencil_triangle const second = {
		.points = {
			.a = {
				.x = down_right.x,
				.y = down_right.y
			},
			.b = {
				.x = up_right.x,
				.y = up_right.y
			},
			.c = {
				.x = down_left.x,
				.y = down_left.y
			}
		}
	};

	myy_vector_stencil_triangles_add(stencil_triangles, 1, &second);
}



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



static void store_to_gl_string(
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

struct myy_rectangle {
	int16_t top, bottom, left, right;
};

struct text_buffer {
	GLuint points;
	myy_vector_gl_chars cpu_buffer;
	GLuint gpu_buffer;
	position_S_4D offset;
	struct myy_rectangle offset_limits;
	struct gl_text_infos * text_display_atlas;
} menu_text;

void text_buffer_init(
	struct text_buffer * __restrict const text_buf,
	struct gl_text_infos * __restrict const text_atlas_properties)
{
	text_buf->points     = 0;
	text_buf->cpu_buffer = myy_vector_gl_chars_init(1024);
	glGenBuffers(1, &text_buf->gpu_buffer);
	text_buf->offset     = DEFAULT_OFFSET_4D;
	text_buf->text_display_atlas = text_atlas_properties;
}

__attribute__((unused))
static inline void text_buffer_set_offset_limits(
	struct text_buffer * __restrict const text_buf,
	struct myy_rectangle const limits)
{
	text_buf->offset_limits = limits;
}

static inline position_S position_S_clamp_to_rectangle(
	position_S pos,
	struct myy_rectangle limits)
{
	
	pos.x = (pos.x >= limits.left)  ? pos.x : limits.left;
	pos.x = (pos.x <= limits.right) ? pos.x : limits.right;

	pos.y = (pos.y >= limits.top)    ? pos.y : limits.top;
	pos.y = (pos.y <= limits.bottom) ? pos.y : limits.bottom;

	return pos;
}

void text_buffer_move(
	struct text_buffer * __restrict const text_buf,
	position_S relative_move)
{
	position_S pos = position_S_2D_from_4D(text_buf->offset);

	pos.x += relative_move.x;
	pos.y += relative_move.y;

	position_S const clamped_position =
		position_S_clamp_to_rectangle(pos, text_buf->offset_limits);
	text_buf->offset = position_S_4D_struct(
		clamped_position.x, clamped_position.y, 0, 0);
}

void text_buffer_reset(
	struct text_buffer * __restrict const text_buf)
{
	myy_vector_gl_chars_reset(&text_buf->cpu_buffer);
}

void text_buffer_add_cb(
	void * text_buf,
	struct myy_gl_text_quad const * __restrict const quads,
	uint32_t const n_quads,
	struct myy_text_properties const * __restrict const props)
{
	struct text_buffer * __restrict const gl_text_buffer =
		(struct text_buffer * __restrict) text_buf;

	myy_vector_gl_chars * __restrict const gl_string =
		&gl_text_buffer->cpu_buffer;

	for (size_t i = 0; i < n_quads; i++) {
		store_to_gl_string(gl_string, quads+i);
	}
}

void text_buffer_add_string(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const utf8_string,
	position_S * __restrict const position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties)
{
	myy_string_to_quads(
		text_buf->text_display_atlas,
		utf8_string,
		position,
		properties,
		text_buffer_add_cb,
		text_buf);
}

void text_buffer_add_strings_list(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const * __restrict const utf8_strings,
	position_S * __restrict const start_position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties,
	int16_t vertical_offset_between_strings)
{
	myy_strings_list_to_quads(
		text_buf->text_display_atlas,
		utf8_strings,
		start_position,
		properties,
		text_buffer_add_cb,
		text_buf,
		vertical_offset_between_strings);
}

void text_buffer_add_n_chars_from_string(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const utf8_string,
	size_t const utf8_string_size,
	position_S * __restrict const position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties)
{
	myy_gl_text_infos_chars_to_quads(
		text_buf->text_display_atlas,
		utf8_string,
		utf8_string_size,
		position,
		properties,
		text_buffer_add_cb,
		text_buf);
}

void text_buffer_store_to_gpu(
	struct text_buffer * __restrict const gl_text_buffer)
{
	myy_vector_gl_chars * __restrict const gl_string =
		(void *) (&gl_text_buffer->cpu_buffer);

	glBindBuffer(GL_ARRAY_BUFFER, gl_text_buffer->gpu_buffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		myy_vector_gl_chars_allocated_used(gl_string),
		myy_vector_gl_chars_data(gl_string),
		GL_STATIC_DRAW);
	gl_text_buffer->points = myy_vector_gl_chars_length(gl_string) * 6;
}

static inline void text_buffer_set_global_position(
	struct text_buffer * __restrict const text_buf,
	position_S_4D position)
{
	text_buf->offset = position;
}


void text_buffer_draw(struct text_buffer const * __restrict const text_buf)
{
	GLuint const n_points = text_buf->points;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(myy_programs.text_id);

	glUniform4f(myy_programs.text_unif_global_offset,
		(float) (text_buf->offset.x),
		(float) (text_buf->offset.y),
		0.0f, 0.0f);
	glBindBuffer(GL_ARRAY_BUFFER, text_buf->gpu_buffer);
	glVertexAttribPointer(
		text_xy, 2, GL_SHORT, GL_FALSE,
		sizeof(struct gl_text_vertex),
		(uint8_t *) (offsetof(struct gl_text_vertex, x)));
	glVertexAttribPointer(
		text_in_st, 2, GL_UNSIGNED_SHORT, GL_FALSE,
		sizeof(struct gl_text_vertex),
		(uint8_t *) (offsetof(struct gl_text_vertex, s)));

	/* The shadow text <- WARNING GPU INTENSIVE due to overdraw ! */
	/* TODO Use SDF instead and do the whole thing once */

	/* The text */
	glUniform4f(myy_programs.text_unif_text_offset, 2.0f, 2.0f, 0.0f, 0.0f);
	glUniform3f(myy_programs.text_unif_rgb, 0.1f, 0.1f, 0.1f);
	glDrawArrays(GL_TRIANGLES, 0, n_points);

	glUniform4f(myy_programs.text_unif_text_offset, 0.0f, 0.0f, -0.1f, 0.0f);
	glUniform3f(myy_programs.text_unif_rgb, 0.95f, 0.95f, 0.95f);
	glDrawArrays(GL_TRIANGLES, 0, n_points);

	glDisable(GL_BLEND);

	glUseProgram(0);
}

/* TODO DOCUMENT !
 * IIRC, the whole point was to have menu elements, with
 * predetermined functions for setting up and drawing.
 */
struct menu_parts_handler {
	position_S_4D pos;
	struct text_buffer static_text;
	struct menu_forms forms;
	struct text_buffer input_text;
};

void menu_parts_handler_init(
	struct menu_parts_handler * __restrict const handler,
	struct gl_text_infos * __restrict const text_atlas_properties)
{
	text_buffer_init(&handler->static_text, text_atlas_properties);
	text_buffer_init(&handler->input_text, text_atlas_properties);
	menu_forms_init(&handler->forms);
	handler->pos = position_S_4D_struct(880,0,0,0);
	/* TODO Add a set position function */
	text_buffer_set_global_position(&handler->input_text, handler->pos);
	text_buffer_set_global_position(&handler->static_text, handler->pos);
	menu_forms_set_global_position(&handler->forms, handler->pos);
}

static inline void menu_parts_handler_set_draw_position(
	struct menu_parts_handler * __restrict const handler,
	position_S_4D pos)
{
	handler->pos = pos;
}

void menu_part_generate_label(
	union menu_part const * __restrict const label_part,
	struct menu_parts_handler * __restrict const handler)
{
	struct menu_part_label const * __restrict const label =
		&(label_part->label);
	struct myy_text_properties props = {
		.myy_text_flows = ((block_top_to_bottom << 8) | line_left_to_right),
		.z_layer = 16,
		.r = 255, .g = 255, .b = 255, .a = 255,
		.user_metadata = NULL
	};
	position_S pos = position_S_2D_from_4D(label->pos);
	text_buffer_add_string(&handler->static_text,
		label->text,
		&pos,
		&props);
}

void menu_part_generate_not_implemented(
	union menu_part const * __restrict const unknown_part,
	struct menu_parts_handler * __restrict const handler)
{
}

void menu_part_generate_input_numeric(
	union menu_part const * __restrict const input_numeric_part,
	struct menu_parts_handler * __restrict const handler)
{
	struct menu_part_input_numeric const * __restrict const input_numeric =
		&(input_numeric_part->input_numeric);

	position_S pos = position_S_2D_from_4D(input_numeric->pos);
	menu_forms_add_bordered_rectangle(
		&handler->forms,
		pos,
		dimensions_S_struct(50,32),
		rgba8_color(0,0,0,255),
		rgba8_color(120,120,120,255));
}

void menu_parts_reset(
	struct menu_parts_handler * __restrict const handler)
{
	text_buffer_reset(&handler->static_text);
	text_buffer_reset(&handler->input_text);
	menu_forms_reset(&handler->forms);
}

void menu_parts_store_to_gpu(
	struct menu_parts_handler * __restrict const handler)
{
	text_buffer_store_to_gpu(&handler->static_text);
	text_buffer_store_to_gpu(&handler->input_text);
	menu_forms_store_to_gpu(&handler->forms);
}

void menu_parts_draw(
	struct menu_parts_handler * __restrict const handler)
{
	menu_forms_draw(&handler->forms);

	text_buffer_draw(&handler->static_text);
	text_buffer_draw(&handler->input_text);
}

void (* menu_parts_generator[n_menu_part_type])(
	union menu_part const * __restrict,
	struct menu_parts_handler * __restrict) =
{
	[menu_part_type_invalid]        = menu_part_generate_not_implemented,
	[menu_part_type_label]          = menu_part_generate_label,
	[menu_part_type_toggle_button]  = menu_part_generate_not_implemented,
	[menu_part_type_input_register] = menu_part_generate_not_implemented,
	[menu_part_type_input_numeric]  = menu_part_generate_input_numeric,
	[menu_part_type_end]            = menu_part_generate_not_implemented,
};

static void menu_parts_add_static_parts(
	struct menu_parts_handler * __restrict const handler)
{
	simple_rgb_quad(&handler->forms.cpu_buffer, 15,
		position_S_struct(0,0), position_S_struct(400,720),
		rgba8_color(0,0,0,1));
}

void menu_parts_handler_generate_menu(
	union menu_part const * __restrict parts,
	struct menu_parts_handler * __restrict const handler)
{
	union menu_part const * __restrict current_part =
		(void *) (parts++);

	struct menu_part_header current_header =
		*((struct menu_part_header *) current_part);

	menu_parts_reset(handler);
	while(menu_part_type_valid(current_header.type)
	      && current_header.type != menu_part_type_end)
	{
		menu_parts_generator[current_header.type](
			current_part,
			handler);
		current_part = (void *) (parts++);
		current_header = *((struct menu_part_header *) current_part);
	}

	menu_parts_add_static_parts(handler);

	menu_parts_store_to_gpu(handler);
}

struct arm64_mov_menu mov_menu = {
	.to_label      =
		MENU_LABEL(POSITION_4D_SIMPLE(20,100,0), (uint8_t const *) "To"),
	.to_register   =
		MENU_INPUT_REGISTER(POSITION_4D_SIMPLE(70,78, 0), "X"),
	.from_label    = 
		MENU_LABEL(POSITION_4D_SIMPLE(170,100,0), (uint8_t const *) "From"),
	.from_register =
		MENU_INPUT_REGISTER(POSITION_4D_SIMPLE(220,78,0), "X")
};

struct menu_parts_handler menu_handler;


struct gl_text_infos gl_text_meta;

myy_vector_template(utf8, uint8_t)

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

/* Binary flags or enumeration... hmm... ? */
enum myy_text_edit_module_flags {
	myy_text_edit_module_flag_inactive,
	myy_text_edit_module_flag_editing,
	myy_text_edit_module_flag_finishing,
};

struct myy_text_edit_module {
	uint32_t flags;
	uint32_t reserved;
	myy_vector_utf8    * __restrict edited_buffer;
	struct myy_text_area * __restrict area;
	off_t edited_buffer_insertion_before;
	off_t edited_buffer_insertion_after;
	myy_vector_utf8 inserted_data_buffer;
};

bool myy_text_edit_module_init(
	struct myy_text_edit_module * __restrict const text_edit_module)
{
	/* TODO Can't we have a default implementation instead ? */
	text_edit_module->flags = myy_text_edit_module_flag_inactive;
	text_edit_module->edited_buffer   = NULL;
	text_edit_module->area            = NULL;
	text_edit_module->edited_buffer_insertion_before = 0;
	text_edit_module->edited_buffer_insertion_after  = 0;
	/* NOTE 4096 octets doesn't mean 4096 UTF-8 characters,
	 *      as these can be encoded on multiple bytes
	 */
	text_edit_module->inserted_data_buffer =
		myy_vector_utf8_init(4096);
	
	return myy_vector_utf8_is_valid(
		&text_edit_module->inserted_data_buffer);
}

void myy_text_edit_module_attach(
	struct myy_text_edit_module * __restrict const text_edit_module,
	struct myy_text_area * __restrict const text_area,
	off_t insertion_point)
{
	text_edit_module->edited_buffer  = &text_area->value;
	text_edit_module->area           = text_area;
	text_edit_module->edited_buffer_insertion_before = insertion_point;
	text_edit_module->edited_buffer_insertion_after  = insertion_point;
	myy_vector_utf8_reset(&text_edit_module->inserted_data_buffer);
	/* Set it last, just in case something accessed this flag
	 * before everything was setup.
	 */
	text_edit_module->flags = myy_text_edit_module_flag_editing;
}

void myy_text_edit_module_detach(
	struct myy_text_edit_module * __restrict const text_edit_module)
{
	/* TODO Error management */
	text_edit_module->flags = myy_text_edit_module_flag_finishing;
	size_t const inserted_string_length =
		myy_vector_utf8_length(&text_edit_module->inserted_data_buffer);
	off_t const insertion_point =
		text_edit_module->edited_buffer_insertion_before;
	off_t const actual_after_insert_index =
		insertion_point
		+ inserted_string_length;
	myy_vector_utf8 * __restrict const edited_buffer =
		text_edit_module->edited_buffer;

	myy_vector_utf8_shift_from(
		edited_buffer,
		text_edit_module->edited_buffer_insertion_after,
		actual_after_insert_index);
	myy_vector_utf8_write_at(
		edited_buffer,
		insertion_point,
		myy_vector_utf8_data(&text_edit_module->inserted_data_buffer),
		inserted_string_length);
	myy_vector_utf8_add(
		edited_buffer,
		1,
		(uint8_t const * __restrict) "\0");

	/* TODO : There's no draw function for the text area... ? */

	text_edit_module->flags = myy_text_edit_module_flag_inactive;
}

void myy_text_edit_module_provoke_redraw(
	struct myy_text_edit_module * __restrict const text_edit_module)
{
	position_S draw_position = text_edit_module->area->position;
	struct text_buffer * __restrict const text_display =
		&text_edit_module->area->display;
	/* TODO Too complex ! */
	struct myy_text_properties properties = {
		.myy_text_flows = ((block_top_to_bottom << 8) | line_left_to_right),
		.z_layer = 16,
		.r = 255, .g = 255, .b = 255, .a = 255,
		.user_metadata = NULL
	};
	uint8_t const * __restrict const edited_buffer =
		myy_vector_utf8_data(text_edit_module->edited_buffer);
	size_t const after_size =
		myy_vector_utf8_length(text_edit_module->edited_buffer) -
		text_edit_module->edited_buffer_insertion_after;
	text_buffer_reset(text_display);

	text_buffer_add_n_chars_from_string(
		text_display,
		edited_buffer,
		text_edit_module->edited_buffer_insertion_before,
		&draw_position,
		&properties);
	text_buffer_add_n_chars_from_string(
		text_display,
		myy_vector_utf8_data(&text_edit_module->inserted_data_buffer),
		myy_vector_utf8_length(&text_edit_module->inserted_data_buffer),
		&draw_position,
		&properties);
	text_buffer_add_n_chars_from_string(
		text_display,
		edited_buffer+text_edit_module->edited_buffer_insertion_after,
		after_size,
		&draw_position,
		&properties);
	text_buffer_store_to_gpu(text_display);
}

bool myy_text_edit_module_add_text(
	struct myy_text_edit_module * __restrict const text_edit_module,
	uint8_t const * __restrict const text,
	size_t text_size)
{
	bool added = myy_vector_utf8_add(
		&text_edit_module->inserted_data_buffer,
		text_size,
		text);

	if (added)
		myy_text_edit_module_provoke_redraw(text_edit_module);

	return added;
}

struct myy_text_area area;
struct myy_text_edit_module module;
void myy_init_drawing()
{
	myy_shaders_pack_load_all_programs_from_file(
		"data/shaders.pack",
		(uint8_t * __restrict) &myy_programs);

	struct gl_text_infos * __restrict const loaded_infos =
		&gl_text_meta;
	struct myy_sampler_properties properties =
		myy_sampler_properties_default();

	glActiveTexture(GL_TEXTURE4);
	myy_packed_fonts_load(
		"data/font_pack_meta.dat", loaded_infos, NULL, &properties);

	float inv_tex_width  = 1.0f/loaded_infos->tex_width;
	float inv_tex_height = 1.0f/loaded_infos->tex_height;

	union myy_4x4_matrix matrix;
	myy_matrix_4x4_ortho_layered_window_coords(&matrix, 1920, 1080, 64);
	glUseProgram(myy_programs.text_id);
	glUniform1i(
		myy_programs.text_unif_fonts_texture,
		4);
	glUniformMatrix4fv(
		myy_programs.text_unif_projection,
		1,
		GL_FALSE,
		matrix.raw_data);
	glUniform2f(
		myy_programs.text_unif_texture_projection,
		inv_tex_width,
		inv_tex_height);

	glEnableVertexAttribArray(text_xy);
	glEnableVertexAttribArray(text_in_st);


	/* Load the text quads into the GPU */
	text_buffer_init(&menu_text, &gl_text_meta);

	struct myy_text_properties string_display_props = {
		.myy_text_flows = ((block_top_to_bottom << 8) | line_left_to_right),
		.z_layer = 16,
		.r = 255, .g = 255, .b = 255, .a = 255,
		.user_metadata = NULL		
	};
	uint8_t const * __restrict const string =
		(uint8_t const * __restrict)
		"MOV R0, R1, LSL #3\n"
		"Ni roxe des poneys rose, mais uniquement sur le toit.\n"
		"Potatoes will rule the world !\n"
		"何言ってんだこいつ・・・";

	struct myy_rectangle menu_move_limits = {
		.left = 0,    .right  = 0,
		.top  = -600, .bottom = 64
	};
	text_buffer_set_offset_limits(&menu_text, menu_move_limits);
	position_S text_position = position_S_struct(300,60);
	int64_t offset_between_lines = 32;

	text_buffer_add_strings_list(
		&menu_text, supported_armv8_instructions,
		&text_position, &string_display_props,
		offset_between_lines);

	text_buffer_store_to_gpu(&menu_text);
	simple_stencil_init(&menu_stencil);
	simple_stencil_set_projection(&matrix);
	simple_stencil_start_preparing(&menu_stencil);
	simple_stencil_store_rectangle(&menu_stencil,
		position_S_struct(300, 90),
		position_S_struct(600, 600));
	simple_stencil_done_preparing(&menu_stencil);

	menu_forms_init(&test_menu);
	menu_forms_set_projection(&matrix);
	menu_forms_reset(&test_menu);
	menu_forms_add_arrow_left(&test_menu,
		position_S_struct(500,300),
		rgba8_color(0, 175, 225, 255));
	menu_forms_add_arrow_right(&test_menu,
		position_S_struct(548,300),
		rgba8_color(40, 40, 40, 255));
	menu_forms_add_bordered_rectangle(&test_menu,
		position_S_struct(500,348),
		dimensions_S_struct(80, 32),
		rgba8_color(60, 60, 60, 255),
		rgba8_color(255,255,255,255));
	menu_forms_store_to_gpu(&test_menu);

	menu_parts_handler_init(&menu_handler, &gl_text_meta);
	menu_parts_handler_generate_menu(
		(union menu_part *) (&mov_menu),
		&menu_handler);


	myy_text_area_init(&area, &gl_text_meta, position_S_struct(32,32));
	/*myy_text_edit_module_init(&module);
	myy_text_edit_module_attach(&module, &area, 0);
	myy_text_edit_module_add_text(
		&module,
		(uint8_t const * __restrict) "そういうことか",
		sizeof("そういうことか"));*/
	
	glClearColor(GLOBAL_BACKGROUND_COLOR);
}

GLuint glbuffer_menu;
uint32_t current_offset;

void prepare_menu() {
	/*glGenBuffers(1, &glbuffer_menu);
	glBindBuffer(GL_ARRAY_BUFFER, glbuffer_menu);
	glBufferData(GL_ARRAY_BUFFER, 4*1024*1024, NULL, GL_STATIC_DRAW);
	prepare_title("Miaou");
	prepare_dropdown_menu(position_S_struct(100,100), "To :", x_regs);
	prepare_dropdown_menu(position_S_struct(200,100), "From :", w_regs);*/
	
}

void myy_draw() {
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	simple_stencil_apply(&menu_stencil);
	text_buffer_draw(&menu_text);
	simple_stencil_put_away(&menu_stencil);
	menu_forms_draw(&test_menu);
	menu_parts_draw(&menu_handler);
	myy_text_area_draw(&area);
}

void myy_key(unsigned int keycode) {
	if (keycode == 1) { myy_user_quit(); }
}

void myy_doubleclick(int x, int y, unsigned int button)
{
	if (button == 4)
		text_buffer_move(&menu_text, position_S_struct(0, 16));
	if (button == 5)
		text_buffer_move(&menu_text, position_S_struct(0, -16));
}


void myy_click(int x, int y, unsigned int button)
{
	if (button == 4)
		text_buffer_move(&menu_text, position_S_struct(0, 16));
	if (button == 5)
		text_buffer_move(&menu_text, position_S_struct(0, -16));
}

void myy_move(int x, int y, int start_x, int start_y)
{
	
}


void myy_text(
	char const * __restrict const text,
	size_t const text_size)
{
	LOG("Text %s\n", text);
	//myy_text_edit_module_add_text(&module, text, text_size);
}
