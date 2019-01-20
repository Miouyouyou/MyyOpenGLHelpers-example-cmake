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

struct {
	GLuint standard_id;
	GLint  standard_unif_projection;
	GLint  standard_unif_tex_projection;
	GLint  standard_unif_tex;
	GLuint text_id;
	GLint  text_unif_projection;
	GLint  text_unif_texture_projection;
	GLint  text_unif_fonts_texture;
	GLint  text_unif_niouik;
} myy_programs;

#endif