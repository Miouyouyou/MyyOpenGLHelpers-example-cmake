#ifndef MYY_GENERATED_OPENGL_ENUMS_H
#define MYY_GENERATED_OPENGL_ENUMS_H 1
#include <myy/current/opengl.h>
#include <stdint.h>
enum glsl_file {
	glsl_file_standard_vsh,
	glsl_file_standard_fsh,
	n_glsl_files,
};

enum glsl_program_name {
	glsl_program_standard,
	n_glsl_programs,
};

enum glsl_program_uniform {
	standard_shader_unif_projection,
	standard_shader_unif_pixel_offset,
	standard_shader_unif_sampler,
	n_glsl_program_uniforms,
};

enum standard_attribute {
	standard_shader_attr_xyz,
	standard_shader_attr_in_st,
	n_standard_attributes,
};

struct glsl_elements {
	struct { uint16_t n, pos; } attributes, uniforms;
};

struct glsl_shader {
	GLuint type;
	uint32_t str_pos;
};

struct glsl_programs_shared_data {
	GLuint programs[n_glsl_programs];
	GLint  uniforms[n_glsl_program_uniforms];
	struct glsl_shader shaders[n_glsl_files];
	struct glsl_elements metadata[n_glsl_programs];
	uint8_t strings[44];
	uint8_t identifiers[42];
};

typedef GLuint uniform_location;
typedef GLuint attribute_location;
struct glsl_programs {
	struct {
		GLuint program_id;
		/* uniforms locations */
		struct {
			uniform_location projection;
			uniform_location pixel_offset;
			uniform_location sampler;
		} uniforms;
		/* attributes location, used with glVertexAttribute */
		struct {
			attribute_location xyz;
			attribute_location st;
		} attributes;
	} standard;
} myy_glsl_programs;


#endif
