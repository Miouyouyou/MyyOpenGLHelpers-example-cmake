#ifndef MYY_SRC_GENERATED_DATAINFOS
#define MYY_SRC_GENERATED_DATAINFOS 1

#include <myy/current/opengl.h>
struct glsl_shader {
	GLuint type;
	uint32_t str_pos;
};
enum glsl_shader_name {
	glsl_shaders_count
};

enum glsl_program_name {
	glsl_programs_count
};

struct glsl_programs_shared_data {
	GLuint programs[glsl_programs_count];
	struct glsl_shader shaders[glsl_shaders_count];
	GLchar strings[512];
};

enum myy_current_textures_id {
	n_textures_id
};

void glhShadersPackLoader
(struct glsl_programs_shared_data * __restrict const data);

#endif
