#include <myy/myy.h>
#include <myy/current/opengl.h>
#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/opengl/shaders_pack.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/matrices.h>

#include <myy/helpers/opengl/buffers.h>

#include <myy/helpers/position.h>

#include <string.h>

#include "shaders.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


typedef void * gpu_buffer_offset_t;

GLuint glbuffer_text;
GLuint n_points_for_text;

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
void simple_stencil_set_stencil_functions()
{
	
	
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
	(uint8_t const *) "MOV  - Register",
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
	position_S offset;
	struct myy_rectangle offset_limits;
} menu_text;

void text_buffer_init(
	struct text_buffer * __restrict const text_buf)
{
	text_buf->points     = 0;
	text_buf->cpu_buffer = myy_vector_gl_chars_init(1024);
	glGenBuffers(1, &text_buf->gpu_buffer);
	text_buf->offset     = position_S_struct(0,0);
	
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
	position_S pos = text_buf->offset;

	pos.x += relative_move.x;
	pos.y += relative_move.y;

	text_buf->offset =
		position_S_clamp_to_rectangle(pos, text_buf->offset_limits);
}

void text_buffer_reset(
	struct text_buffer * __restrict const text_buf)
{
	myy_vector_gl_chars_reset(&text_buf->cpu_buffer);
}

void text_buffer_store(
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

	glBindBuffer(GL_ARRAY_BUFFER, gl_text_buffer->gpu_buffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		myy_vector_gl_chars_allocated_used(gl_string),
		myy_vector_gl_chars_data(gl_string),
		GL_STATIC_DRAW);
	gl_text_buffer->points = myy_vector_gl_chars_length(gl_string) * 6;
}

void text_buffer_draw(struct text_buffer const * __restrict const text_buf)
{
	GLuint const n_points = text_buf->points;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(myy_programs.text_id);

	glUniform3f(myy_programs.text_unif_global_offset,
		(float) (text_buf->offset.x),
		(float) (text_buf->offset.y),
		0.0f);
	glBindBuffer(GL_ARRAY_BUFFER, text_buf->gpu_buffer);
	glVertexAttribPointer(
		text_xy, 2, GL_SHORT, GL_FALSE,
		sizeof(struct gl_text_vertex),
		(uint8_t *) (offsetof(struct gl_text_vertex, x)));
	glVertexAttribPointer(
		text_in_st, 2, GL_UNSIGNED_SHORT, GL_FALSE,
		sizeof(struct gl_text_vertex),
		(uint8_t *) (offsetof(struct gl_text_vertex, s)));

	/* The shadow text <- WARNING GPU INTENSIVE ! */

	/* The text */
	glUniform3f(myy_programs.text_unif_text_offset, 2.0f, 2.0f, 0.0f);
	glUniform3f(myy_programs.text_unif_rgb, 0.1f, 0.1f, 0.1f);
	glDrawArrays(GL_TRIANGLES, 0, n_points);

	glUniform3f(myy_programs.text_unif_text_offset, 0.0f, 0.0f, -0.1f);
	glUniform3f(myy_programs.text_unif_rgb, 0.95f, 0.95f, 0.95f);
	glDrawArrays(GL_TRIANGLES, 0, n_points);

	glDisable(GL_BLEND);

	glUseProgram(0);
}


struct gl_text_infos gl_text_meta;

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
	myy_matrix_4x4_ortho_layered_window_coords(&matrix, 1280, 720, 64);
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
	text_buffer_init(&menu_text);

	struct myy_text_properties string_meta = {
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
		.top  = -300, .bottom = 64
	};
	text_buffer_set_offset_limits(&menu_text, menu_move_limits);
	position_S text_position = position_S_struct(300,60);
	int64_t offset_between_lines = 32;

	myy_strings_list_to_quads(
		loaded_infos, supported_armv8_instructions,
		&text_position, &string_meta,
		text_buffer_store, &menu_text, offset_between_lines);

	simple_stencil_init(&menu_stencil);
	simple_stencil_set_projection(&matrix);
	simple_stencil_start_preparing(&menu_stencil);
	simple_stencil_store_rectangle(&menu_stencil,
		position_S_struct(300, 90),
		position_S_struct(600, 600));
	simple_stencil_done_preparing(&menu_stencil);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
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
