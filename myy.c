#include <myy.h>
#include <myy/current/opengl.h>
#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/opengl/shaders_pack.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/matrices.h>

#include <string.h>

#include "shaders.h"

#define QUAD_LEFT  -0.05f
#define QUAD_RIGHT  0.05f
#define QUAD_UP     0.71f
#define QUAD_DOWN  -0.71f

#define TEX_LEFT    0.0f
#define TEX_RIGHT   1.0f
#define TEX_UP      0.0f
#define TEX_DOWN    1.0f

float quad[] = {
	QUAD_LEFT, QUAD_UP, TEX_LEFT, TEX_UP,
	QUAD_LEFT, QUAD_DOWN, TEX_LEFT, TEX_DOWN,
	QUAD_RIGHT, QUAD_DOWN, TEX_RIGHT, TEX_DOWN,
	QUAD_RIGHT, QUAD_UP, TEX_RIGHT, TEX_UP,
	QUAD_LEFT, QUAD_UP, TEX_LEFT, TEX_UP,
	QUAD_RIGHT, QUAD_DOWN, TEX_RIGHT, TEX_DOWN
};

enum attribs {
	attr_xy,
	attr_st
};
GLuint glbuffer_quad;
GLuint tex;

struct gl_text_infos infos;
US_two_tris_quad_3D characters[53];

void myy_init_drawing()
{
	myy_shaders_pack_load_all_programs_from_file(
		"shaders/shaders.pack",
		(uint8_t * __restrict) &myy_programs);
	struct gl_text_infos loaded_infos = myy_packed_fonts_load(
		"data/font_pack_meta.dat", NULL);

	char const * __restrict const string =
		"Je roxe des poneys rose, mais uniquement sur le toit.";
	position_S text_position = position_S_struct(300,300);
	for (uint_fast32_t i = 0; i < 53; i++) {
		myy_glyph_to_twotris_quad_window_coords(
			&loaded_infos, string[i], characters+i, &text_position);
	}
	
	memcpy(&infos, &loaded_infos, sizeof(loaded_infos));
	glGenBuffers(1, &glbuffer_quad);
	glBindBuffer(GL_ARRAY_BUFFER, glbuffer_quad);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(characters), &characters, GL_STATIC_DRAW);


	glActiveTexture(GL_TEXTURE1);
	glhUploadMyyRawTextures("textures/fonts_bitmap.myyraw", 1, &tex);
	
	union myy_4x4_matrix matrix;
	myy_matrix_4x4_ortho_layered_window_coords(&matrix, 1280, 720, 64);
	glUseProgram(myy_programs.text_id);
	glUniform1i(
		myy_programs.text_unif_fonts_texture,
		1);
	glUniformMatrix4fv(
		myy_programs.text_unif_projection,
		1,
		GL_FALSE,
		matrix.raw_data);

	glEnableVertexAttribArray(text_xyz);
	glEnableVertexAttribArray(text_in_st);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glBindBuffer(GL_ARRAY_BUFFER, glbuffer_quad);
	glVertexAttribPointer(
		text_in_st, 2, GL_UNSIGNED_SHORT, GL_TRUE,
		sizeof(struct US_textured_point_3D),
		(uint8_t *) (offsetof(struct US_textured_point_3D, s)));
	glVertexAttribPointer(
		text_xyz, 3, GL_SHORT, GL_FALSE,
		sizeof(struct US_textured_point_3D),
		(uint8_t *) (offsetof(struct US_textured_point_3D, x)));
}



void myy_draw() {
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glDrawArrays(GL_TRIANGLES, 0, 53*n_corners_two_tris_quad);

}

void myy_key(unsigned int keycode) {
	if (keycode == 1) { myy_user_quit(); }
}
