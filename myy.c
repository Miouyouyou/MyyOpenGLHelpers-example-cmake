#include <myy.h>
#include <myy/current/opengl.h>
#include <src/generated/opengl/shaders_infos.h>
#include <myy/helpers/opengl/loaders.h>

#define QUAD_LEFT  -0.05f
#define QUAD_RIGHT  0.05f
#define QUAD_UP     0.71f
#define QUAD_DOWN  -0.71f

#define TEX_LEFT    0.0f
#define TEX_RIGHT   1.0f
#define TEX_UP      0.0f
#define TEX_DOWN    1.0f

static GLfloat quad[] = {
	QUAD_LEFT,  QUAD_UP,   TEX_LEFT,  TEX_UP,
	QUAD_LEFT,  QUAD_DOWN, TEX_LEFT,  TEX_DOWN,
	QUAD_RIGHT, QUAD_UP,   TEX_RIGHT, TEX_UP,

	QUAD_RIGHT, QUAD_DOWN, TEX_RIGHT, TEX_DOWN,
	QUAD_RIGHT, QUAD_UP,   TEX_RIGHT, TEX_UP,
	QUAD_LEFT,  QUAD_DOWN, TEX_LEFT,  TEX_DOWN,
};

enum attribs {
	attr_xy,
	attr_st
};
GLuint glbuffer_quad;
GLuint tex;

void myy_init_drawing()
{
	GLuint program = glhSetupProgram(
		"shaders/standard.vert", "shaders/standard.frag",
		1, "xy\0in_st\0");

	glGenBuffers(1, &glbuffer_quad);
	glBindBuffer(GL_ARRAY_BUFFER, glbuffer_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	struct myy_fh_map_handle fonts_texture_mapped =
		fh_MapFileToMemory("textures/fonts.raw");
	glActiveTexture(GL_TEXTURE1);
	
	if (fonts_texture_mapped.ok) {
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_ALPHA,
			/* width, height */
			64, 512,
			0,
			GL_ALPHA,
			GL_UNSIGNED_BYTE,
			fonts_texture_mapped.address);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		fh_UnmapFileFromMemory(fonts_texture_mapped);
	}

	glUseProgram(program);
	glUniform1i(
		glGetUniformLocation(program, "fonts_texture"),
		1);
	glEnableVertexAttribArray(attr_xy);
	glEnableVertexAttribArray(attr_st);
	glVertexAttribPointer(
		attr_xy, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
		(GLvoid const *) 0);
	glVertexAttribPointer(
		attr_st, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
		(GLvoid const *) (2*sizeof(float)));
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}


void myy_draw() {
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glClearColor(0.1f, 0.7f, 0.9f, 1.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void myy_key(unsigned int keycode) {
	if (keycode == 1) { myy_user_quit(); }
}
