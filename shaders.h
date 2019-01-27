#ifndef MYY_PACKED_SHADERS_RUNTIME_DATA_H
#define MYY_PACKED_SHADERS_RUNTIME_DATA_H 1

#include <myy/current/opengl.h>

enum myy_shader_standard_attribs {
	standard_xy = 5,
	standard_in_st = 6,
};
enum myy_shader_text_attribs {
	text_xy = 0,
	text_in_st = 1,
};
enum myy_shader_simple_stencil_attribs {
	simple_stencil_xy = 0,
};

struct {
	GLuint standard_id;
	GLint  standard_unif_projection;
	GLint  standard_unif_tex_projection;
	GLint  standard_unif_tex;
	GLuint text_id;
	GLint  text_unif_projection;
	GLint  text_unif_texture_projection;
	GLint  text_unif_text_offset;
	GLint  text_unif_global_offset;
	GLint  text_unif_fonts_texture;
	GLint  text_unif_rgb;
	GLuint simple_stencil_id;
	GLint  simple_stencil_unif_projection;
} myy_programs;

#endif