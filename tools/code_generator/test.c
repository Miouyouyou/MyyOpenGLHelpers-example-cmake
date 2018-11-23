#include "myy/helpers/file.h"
#include <stddef.h>


/* Let's be honest, if we have to deal with more than
 * - 64  OpenGL shader programs
 * - 128 Textures
 * - 128 Buffers
 * We'll have to use a SQLite database with a nice GUI
 */
#define MAX_FILES_PER_SHADERS   4
#define MAX_SHADER_PROGRAMS    64
#define MAX_TEXTURES          128
#define MAX_BUFFERS           128

enum program_status {
	program_status_ok,
	program_status_not_enough_arguments,
	program_status_nothing
};

enum section_being_parsed {
	section_not_found_at_the_moment,
	section_programs,
	section_textures,
	section_fonts,
	n_sections,
};

struct string_s {
	size_t size;
	uint8_t const * __restrict start;
};

typedef struct string_s string;

/** [Programs] structures **/

enum shader_type {
	shader_type_vertex,
	shader_type_fragment,
	shader_type_compute,
	shader_type_geometry,
	n_shaders_types
};

struct definition_shader_file {
	enum shader_type type;
	string filepath;
};

struct list_definitions_shader_files {
	uint8_t count;
	uint8_t max;
	struct definition_shader_file file[MAX_FILES_PER_SHADERS];
};

struct definition_shader_program {
	string name;
	struct list_definitions_shader_files file_list;
};

struct list_definition_shader_program {
	uint8_t count;
	uint8_t max;
	struct definition_shader_program programs[MAX_SHADER_PROGRAMS];
};

/** [Textures] **/

struct definition_texture {
	string name;
	string filepath;
};

struct list_definition_texture {
	uint8_t count;
	uint8_t max;
	struct definition_texture textures[MAX_TEXTURES];
};

/** [Buffers] **/

enum buffer_duplication_type {
	single_buffer,
	double_buffer,
	triple_buffer,
	n_buffer_duplication_types
};

enum buffer_type {
	buffer_float,
	buffer_double,
	buffer_int,
	n_buffers_types
};

struct definition_buffer {
	string name;
	enum buffer_duplication_type duplication;
	enum buffer_type type;
	uint32_t size;
};

struct list_definition_buffer {
	uint8_t count;
	uint8_t max;
	struct definition_buffer buffers[MAX_BUFFERS];
};

struct parsing_state {
	uint8_t const * __restrict cursor;
	uint8_t const * __restrict const end;
	enum section_being_parsed * __restrict const section_parsed;

	struct list_definition_shader_program  programs_list;
	struct list_definition_texture         textures_list;
	struct list_definition_buffer          buffers_list;
};

uint8_t const * string_go_to_next_ascii_character(
	uint8_t const * __restrict cursor,
	uint8_t const * __restrict const end)
{
	while (cursor < end && *cursor < 33)
		cursor++;
	return cursor;
}

uint8_t const * string_go_to_next_line_posix(
	uint8_t const * __restrict cursor,
	uint8_t const * __restrict const end)
{
	while (cursor < end && *cursor != '\n')
		cursor++;
	return cursor;
}

uint8_t const * change_section(
	uint8_t const * __restrict cursor,
	uint8_t const * __restrict const end,
	enum section_being_parsed * __restrict const section_parsed)
{
	return cursor;
}

uint8_t const (* parse_section_content[n_sections])(
	uint8_t const * __restrict cursor,
	uint8_t const * __restrict const end) =
{
	[section_not_found_at_the_moment] = parse_invalid_text_line,
	[section_programs]                = parse_program_section_line,
	[section_textures]                = parse_texture_section_line,
	[section_fonts]                   = parse_font_section_line
};

int main(int argc, char **argv)
{
	enum program_status ret = program_status_ok;

	struct myy_fh_map_handle handle =
		fh_MapFileToMemory("metadata.ini");

	uint8_t const * __restrict cursor =
		handle.address;
	uint8_t const * __restrict const end =
		handle.address + handle.length;

	enum section_being_parsed parsing =
		section_not_found_at_the_moment;
	cursor = 
		string_go_to_next_ascii_character(cursor, end);

	if (cursor == end) {
		ret = program_status_nothing;
		goto out;
	}

	if (*cursor != '[') {
		cursor = parse_section_content[parsing](
			cursor, end);
	}
	else {
		cursor = change_section(cursor, end, &parsing);
	}
	fh_UnmapFileFromMemory(handle);
out:
	return ret;
}

#include "myy/current/opengl.h"

struct my_gl_program_shader {
	enum shader_type type;
	GLchar * name;
};

uint_fast8_t myy_gl_program_link(
	struct my_gl_program_shader const * __restrict const shaders,
	GLchar const * __restrict const attributes_names,
	uint_fast8_t const n_attributes,
	GLuint * __restrict const program_id)
{
	GLuint program = glCreateProgram();

	if (!program) {
		glhPrintError();
		goto no_program;
	}

	if (!glhLinkShaders(program, shaders))
		goto could_not_link_shaders;

	if (!glhBindAttributes(attributes_names, n_attributes))
		goto could_not_bind_attributes;

	glLinkProgram(program);
	if 
		glhPrintError();
		goto could_not_link_program;
	}

	*program_id = program;

	return 1;

could_not_link_program:
could_not_bind_attributes:
could_not_link_shaders:
	glDeleteProgram(program);
no_program:
	return 0;
}

uint_fast8_t myy_gl_program_get_uniforms_locations(
	GLuint const program_id,
	uint_fast8_t const n_uniforms,
	GLchar const * __restrict const uniforms_names,
	GLint * __restrict const uniforms_id)
{
	uint_fast8_t u = 0;
	for (; u < n_uniforms; u++)
	{
		GLint uniform_index = glGetUniformLocation(program,
			uniforms_names[u]);
		if (uniform_index != -1) {
			glhPrintError();
			break;
		}
	}
	return u == n_uniforms;
}
