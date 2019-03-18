#include <src/widgets/text_buffer.h>
#include <src/widgets/common_types.h>
#include <shaders.h>

#include <myy/helpers/position.h>


void text_buffer_init(
	struct text_buffer * __restrict const text_buf,
	struct gl_text_infos * __restrict const text_atlas_properties)
{
	text_buf->points     = 0;
	text_buf->cpu_buffer = myy_vector_gl_chars_init(1024);
	glGenBuffers(1, &text_buf->gpu_buffer);
	text_buf->offset     = DEFAULT_OFFSET_4D;
	text_buf->text_display_atlas = text_atlas_properties;
}

void text_buffer_move(
	struct text_buffer * __restrict const text_buf,
	position_S relative_move)
{
	position_S pos = position_S_2D_from_4D(text_buf->offset);

	pos.x += relative_move.x;
	pos.y += relative_move.y;

	position_S const clamped_position =
		position_S_clamp_to_rectangle(pos, text_buf->offset_limits);
	text_buf->offset = position_S_4D_struct(
		clamped_position.x, clamped_position.y, 0, 0);
}

void text_buffer_reset(
	struct text_buffer * __restrict const text_buf)
{
	myy_vector_gl_chars_reset(&text_buf->cpu_buffer);
}

void text_buffer_add_cb(
	void * text_buf,
	struct myy_gl_text_quad const * __restrict const quads,
	uint32_t const n_quads,
	struct myy_text_properties const * __restrict const props)
{
	struct text_buffer * __restrict const gl_text_buffer =
		(struct text_buffer * __restrict) text_buf;

	myy_vector_gl_chars * __restrict const gl_string =
		&gl_text_buffer->cpu_buffer;

	for (size_t i = 0; i < n_quads; i++) {
		store_to_gl_string(gl_string, quads+i);
	}
}

void text_buffer_add_string(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const utf8_string,
	position_S * __restrict const position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties)
{
	myy_string_to_quads(
		text_buf->text_display_atlas,
		utf8_string,
		position,
		properties,
		text_buffer_add_cb,
		text_buf);
}

void text_buffer_add_strings_list(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const * __restrict const utf8_strings,
	position_S * __restrict const start_position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties,
	int16_t vertical_offset_between_strings)
{
	myy_strings_list_to_quads(
		text_buf->text_display_atlas,
		utf8_strings,
		start_position,
		properties,
		text_buffer_add_cb,
		text_buf,
		vertical_offset_between_strings);
}

void text_buffer_add_n_chars_from_string(
	struct text_buffer * __restrict const text_buf,
	uint8_t const * __restrict const utf8_string,
	size_t const utf8_string_size,
	position_S * __restrict const position, /* TODO : Needs to be 3D or 4D... */
	struct myy_text_properties const * __restrict const properties)
{
	myy_gl_text_infos_chars_to_quads(
		text_buf->text_display_atlas,
		utf8_string,
		utf8_string_size,
		position,
		properties,
		text_buffer_add_cb,
		text_buf);
}

void text_buffer_store_to_gpu(
	struct text_buffer * __restrict const gl_text_buffer)
{
	myy_vector_gl_chars * __restrict const gl_string =
		(void *) (&gl_text_buffer->cpu_buffer);

	glBindBuffer(GL_ARRAY_BUFFER, gl_text_buffer->gpu_buffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		myy_vector_gl_chars_allocated_used(gl_string),
		myy_vector_gl_chars_data(gl_string),
		GL_STATIC_DRAW);
	gl_text_buffer->points = myy_vector_gl_chars_length(gl_string) * 6;
}

void text_buffer_draw(struct text_buffer const * __restrict const text_buf)
{
	GLuint const n_points = text_buf->points;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(myy_programs.text_id);

	glUniform4f(myy_programs.text_unif_global_offset,
		(float) (text_buf->offset.x),
		(float) (text_buf->offset.y),
		0.0f, 0.0f);
	glBindBuffer(GL_ARRAY_BUFFER, text_buf->gpu_buffer);
	glVertexAttribPointer(
		text_xy, 2, GL_SHORT, GL_FALSE,
		sizeof(struct gl_text_vertex),
		(uint8_t *) (offsetof(struct gl_text_vertex, x)));
	glVertexAttribPointer(
		text_in_st, 2, GL_UNSIGNED_SHORT, GL_FALSE,
		sizeof(struct gl_text_vertex),
		(uint8_t *) (offsetof(struct gl_text_vertex, s)));

	/* The shadow text <- WARNING GPU INTENSIVE due to overdraw ! */
	/* TODO Use SDF instead and do the whole thing once */

	/* The text */
	glUniform4f(myy_programs.text_unif_text_offset, 2.0f, 2.0f, 0.0f, 0.0f);
	glUniform3f(myy_programs.text_unif_rgb, 0.1f, 0.1f, 0.1f);
	glDrawArrays(GL_TRIANGLES, 0, n_points);

	glUniform4f(myy_programs.text_unif_text_offset, 0.0f, 0.0f, -0.1f, 0.0f);
	glUniform3f(myy_programs.text_unif_rgb, 0.95f, 0.95f, 0.95f);
	glDrawArrays(GL_TRIANGLES, 0, n_points);

	glDisable(GL_BLEND);

	glUseProgram(0);
}
