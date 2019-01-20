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

GLuint glbuffer_text;
GLuint n_points_for_text;

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
myy_vector_gl_chars gl_string_characters;

static void print_gl_text_quad(
	struct myy_gl_text_quad const * __restrict const quad)
{
	
}

static void store_to_gl_string(
	myy_vector_gl_chars * __restrict const gl_string,
	struct myy_gl_text_quad const * __restrict const quad)
{
	print_gl_text_quad(quad);
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

void store_text_quads(
	void * buffer_id,
	struct myy_gl_text_quad const * __restrict const quads,
	uint32_t const n_quads,
	struct myy_text_properties const * __restrict const props)
{
	union gl_text_character current_character;
	myy_vector_gl_chars * __restrict const gl_string =
		&gl_string_characters;
	for (size_t i = 0; i < n_quads; i++) {
		store_to_gl_string(gl_string, quads+i);
	}

	GLuint buf_id = *((GLuint *) buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, buf_id);
	glBufferData(
		GL_ARRAY_BUFFER,
		myy_vector_gl_chars_allocated_used(gl_string),
		myy_vector_gl_chars_data(gl_string),
		GL_STATIC_DRAW);
	n_points_for_text = 
		myy_vector_gl_chars_length(gl_string) * 6;

	glUseProgram(myy_programs.text_id);
	glVertexAttribPointer(
		text_xy, 2, GL_SHORT, GL_FALSE,
		sizeof(struct gl_text_vertex),
		(uint8_t *) (offsetof(struct gl_text_vertex, x)));
	glVertexAttribPointer(
		text_in_st, 2, GL_UNSIGNED_SHORT, GL_FALSE,
		sizeof(struct gl_text_vertex),
		(uint8_t *) (offsetof(struct gl_text_vertex, s)));
}

struct gl_text_infos gl_text_meta;

void myy_init_drawing()
{
	gl_string_characters = myy_vector_gl_chars_init(1024);
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	/* Load the text quad into the GPU */
	glGenBuffers(1, &glbuffer_text);
	struct myy_text_properties string_meta = {
		.myy_text_flows = ((block_top_to_bottom << 8) | line_left_to_right),
		.z_layer = 16,
		.r = 255, .g = 255, .b = 255, .a = 255,
		.user_metadata = NULL		
	};
	char const * __restrict const string =
		"Ni roxe des poneys rose, mais uniquement sur le toit.\n"
		"Potatoes will rule the world !\n"
		"何言ってんだこいつ・・・";

	position_S text_position = position_S_struct(300,300);

	myy_string_to_quads(
		loaded_infos, string, &text_position, &string_meta,
		store_text_quads, &glbuffer_text);

}

void myy_draw() {
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	/* Draw the text */
	glUseProgram(myy_programs.text_id);
	glDrawArrays(GL_TRIANGLES, 0, n_points_for_text);
	
}

void myy_key(unsigned int keycode) {
	if (keycode == 1) { myy_user_quit(); }
}

void myy_click(int x, int y, unsigned int button)
{

}
