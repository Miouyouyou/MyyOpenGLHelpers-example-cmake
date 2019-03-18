#include <src/widgets/stencil.h>
#include <src/widgets/common_types.h>


#include <myy/current/opengl.h>
#include <myy/helpers/matrices.h>
#include <myy/helpers/position.h>

#include <shaders.h>

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

GLuint simple_stencil_stored_points(
	struct simple_stencil const * __restrict const simple_stencil_object)
{
	return 
		myy_vector_stencil_triangles_length(
			&simple_stencil_object->cpu_buffer)
		* 3;
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

