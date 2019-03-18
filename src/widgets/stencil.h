#ifndef MYY_WIDGETS_STENCIL_H
#define MYY_WIDGETS_STENCIL_H 1

#include <stdint.h>

#include <myy/current/opengl.h>
#include <myy/helpers/position.h>
#include <myy/helpers/vector.h>
#include <myy/helpers/matrices.h>

#include <shaders.h>

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
	struct simple_stencil * __restrict const simple_stencil_object);
void simple_stencil_start_preparing(
	struct simple_stencil * __restrict const simple_stencil_object);
void simple_stencil_done_preparing(
	struct simple_stencil * __restrict const simple_stencil_object);
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
	struct simple_stencil const * __restrict const simple_stencil_object);
void simple_stencil_apply(
	struct simple_stencil const * __restrict const simple_stencil_object);
static inline void simple_stencil_put_away(
	struct simple_stencil const * __restrict const simple_stencil_object)
{
	glDisable(GL_STENCIL_TEST);
}
void simple_stencil_store_rectangle(
	struct simple_stencil * __restrict const simple_stencil_object,
	position_S up_left,
	position_S down_right);

#endif
