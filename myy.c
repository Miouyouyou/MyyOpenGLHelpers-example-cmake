#include <myy.h>
#include <myy/current/opengl.h>
#include <src/generated/opengl/shaders_infos.h>
#include <myy/helpers/opengl/loaders.h>

struct glsl_programs_shared_data data;
void myy_init_drawing()
{
	glhShadersPackLoader(&data);
	glhBuildAndSaveSimpleProgram(&data,
		glsl_file_standard_vsh,
		glsl_file_standard_fsh,
		glsl_program_standard);
}

void myy_draw() {

	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glClearColor(0.1f, 0.7f, 0.9f, 1.0f);
}

void myy_key(unsigned int keycode) {
	if (keycode == 1) { myy_user_quit(); }
}
