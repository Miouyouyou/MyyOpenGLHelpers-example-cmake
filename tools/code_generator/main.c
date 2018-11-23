#define _ISOC99_SOURCE

#include <stdio.h>  // printf, stdin, stdout, stderr
#include <stddef.h> // NULL

#include <errno.h>  // EINVAL

/* Open */
#include <sys/types.h> // off_t
#include <stdint.h>    // uint*_t

#include <stdlib.h>    // malloc

#include <string.h> // strncmp
#include <ctype.h>  // is_alnum

#include "myy/helpers/file.h"

#define print_error(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#define whine(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

void print_usage(char const * __restrict const program_name)
{
	char const * __restrict const printed_program_name =
		program_name != NULL ? program_name : "code_generator";
	printf(
		"Usage : %p path/to/shaders/folder",
		printed_program_name);
}

#define path_separator '/'

#define line_separator '\n'

#define PROGRAMS_DEFINITIONS_PATHNAME_SIZE 4096
#define PROGRAMS_DEFINITIONS_PATHNAME_COPY_SIZE \
	(PROGRAMS_DEFINITIONS_PATHNAME_SIZE - 1)

char programs_definitions_pathname[PROGRAMS_DEFINITIONS_PATHNAME_SIZE]
	= {0};

char const * __restrict const minimalist_gles20_shader =
	"attribute vec4 a; int main() {}";
char const * __restrict const attribute_minimal_size =
	"attribute vec4 a;";

/* The range of the line, not including the carriage return or EOF */
struct line_range {
	off_t start;
	off_t stop;
};

#define MAX_LINES 256


struct array_line_ranges {
	uint32_t          size;
	struct line_range data[MAX_LINES];
};
typedef struct array_line_ranges array_line_ranges_t;

struct next_line_result {
	uint_fast8_t  found;
	uint32_t      index;
	off_t         offset;
};

struct next_line_result line_starts_next_line(
	array_line_ranges_t const * __restrict const line_ranges,
	off_t offset)
{
	struct next_line_result result = {0,0,0};
	uint32_t n_line_numbers = line_ranges->size;
	for (result.index = 0;
	     result.index < n_line_numbers && result.found == 0;
	     result.index++)
	{
		result.offset = line_ranges->data[result.index].start;
		result.found  = offset >= result.offset;
	}

	return result;
}

static inline uint_fast8_t line_starts_is_last_line(
	array_line_ranges_t const * __restrict line_ranges,
	uint32_t line_index)
{
	return ((line_ranges->size - 1) == line_index);
}

#define BIGGEST_SECTION_TITLE "[programs]"
#define BIGGEST_SECITON_TITLE_SIZE (sizeof(BIGGEST_SECTION_TITLE))

static inline uint_fast8_t metadata_section_title_size_is_valid(
	off_t const size)
{
	off_t const valid_sizes[] = {
		sizeof("programs"), sizeof("textures"), sizeof("buffers")
	};

	static size_t valid_sizes_length = sizeof(valid_sizes)/sizeof(off_t);
	
	uint_fast8_t size_is_valid = 0;

	for (uint_fast8_t i = 0;
		 i < valid_sizes_length && !size_is_valid;
		 i++)
		size_is_valid = (valid_sizes[i] == size);

	return size_is_valid;
}

enum shader_types {
	shader_type_invalid,
	shader_type_vertex,
	shader_type_fragment,
	n_shader_types,
};

struct shader_file_definition {
	enum shader_types shader_type;
	uint8_t pathname[64];
};

struct program_definition {
	uint8_t name[64];
	uint32_t n_files;
	struct shader_file_definition shader_files_definitions[n_shader_types];
};



struct texture_definition {
	uint8_t name[64];
	uint8_t pathname[64];
};

struct buffer_definition {
	uint8_t name[64];
	uint8_t clones;
	uint32_t size;
};

struct ini_content {

	uint8_t
		n_programs, max_programs,
		n_textures, max_textures,
		n_buffers,  max_buffers;
	struct program_definition programs[32];
	struct texture_definition textures[32];
	struct buffer_definition  buffers[32];
} state = {
	.n_programs = 0, .max_programs = 32,
	.n_textures = 0, .max_textures = 32,
	.n_buffers  = 0, .max_buffers  = 32,
	.programs = {
		[0 ... 31] = {
			.name = {0},
			.n_files = 0,
			.shader_files_definitions[0 ... n_shader_types-1] = {
				.shader_type = shader_type_invalid,
			}
		}
	},
	.textures = {
		[0 ... 31] = {
			.name = {0},
			.pathname = {0}
		}
	},
	.buffers = {
		[0 ... 31] = {
			.name = {0},
			.clones = 0,
			.size   = 0
		}
	}
};

off_t parse_programs_sections(
	FILE * const file, 
	array_line_ranges_t const * __restrict const lines,
	int_fast32_t current_line_index)
{
	current_line_index++;
	struct line_range first_line = lines->data[current_line_index];
	off_t pos = first_line.start;

	fseek(file, first_line.start, SEEK_SET);

	char current_char = fgetc(file);
	struct program_definition current_program_definition;
	enum shader_types current_shader_type = shader_type_invalid;

	while (isspace(current_char) && pos < first_line.stop) {
		 current_char = fgetc(file);
		 pos++;
	}

	for(uint_fast8_t i = 0;
		i < 32 && current_char != ':' && pos < first_line.stop;
		i++, pos++)
	{
		if ((i == 0 && isalpha(current_char)) || isalnum(current_char))
			current_program_definition.name[i] = current_char;
	}

	while (isspace(current_char) && pos < first_line.stop) {
		 current_char = fgetc(file);
		 pos++;
	}

	if (pos < first_line.stop) {
		switch(current_char) {
			case 'V':
				current_shader_type = shader_type_vertex;
				pos++;
				current_char = fgetc(file);
				break;
			case 'F':
				current_shader_type = shader_type_fragment;
				pos++;
				current_char = fgetc(file);
				break;
			default:
				whine("Invalid shader type");
				current_line_index = -1;
				goto out;
		}
	}

	if (pos < first_line.stop) {
		current_char
	}
out:
	return current_line_index;
}

off_t parse_textures_sections(
	FILE * const file, 
	array_line_ranges_t const * __restrict const lines,
	int_fast32_t current_line_index)
{
	
}

off_t parse_buffers_sections(
	FILE * const file, 
	array_line_ranges_t const * __restrict const lines,
	int_fast32_t current_line_index)
{
	
}

void metadata_parse_sections(
	FILE * __restrict const program_ini,
	array_line_ranges_t const * __restrict const lines)
{

	uint_fast8_t seeked = 0; 
	char first_char = -EOF;
	char last_char  = -EOF;
	struct line_range current_line;
	char section_title[8];
	off_t next_line_start = 0;

	int_fast32_t l = 0; 

	/* We don't parse the last line.
	 * If the last line is a section title then the
	 * section is empty, by definition.
	 */
	while (l > 0 && l < lines->size && line_starts_is_last_line(lines, l)) {
		current_line = lines->data[l];

		off_t size = current_line.stop - current_line.start;
		uint_fast8_t valid_section_title_size =
			metadata_section_title_size_is_valid(size);
		if (!valid_section_title_size) goto loop_end;

		fseek(program_ini, current_line.start, SEEK_SET);
		first_char = fgetc(program_ini);
		fseek(program_ini, current_line.stop-1, SEEK_SET);
		last_char  = fgetc(program_ini);

		if (first_char != '[' && last_char != ']') {
			fseek(program_ini,
				current_line.start+sizeof('['), SEEK_SET);
			fread(section_title, size, 1, program_ini);

			if (strncmp(section_title, "programs", size) == 0) {
				l = parse_programs_sections(program_ini, lines, l);
				continue;
			}
			else if (strncmp(section_title, "textures", size) == 0) {
				l = parse_textures_sections(program_ini, lines, l);
				continue;
			}
			else if (strncmp(section_title, "buffers", size) == 0) {
				l = parse_buffers_sections(program_ini, lines, l);
				continue;
			}
		}
loop_end:
		l++;
	}

	fclose(program_ini);

	if (l < 0) {
		whine("Something wrong happened...");
		exit(-EINVAL);
	}
}

array_line_ranges_t line_ranges;

static off_t file_current_line_end(FILE * __restrict const file)
{
	/* FIXME Only works with one character end of line */
	char current_char = fgetc(file);
	while (current_char != EOF
		   && current_char != line_separator)
		current_char = fgetc(file);

	/* Our current position */
	/* In case of EOF, fgetc does NOT advance the cursor.
	 * However, it does in other cases.
	 */
	off_t const offset = 
		(ftell(file) - (current_char == line_separator));

	return offset;
}

static off_t file_line_end_offset_from(
	FILE * __restrict const file,
	off_t start_from)
{
	off_t offset = -1;
	if (fseek(file, start_from, SEEK_SET) < 0)
		goto out;

	offset = file_current_line_end(file);
out:
	return offset;
}

static void file_seek_to_current_line_start(
	FILE * __restrict const file)
{
	/* FIXME This only works with '\n' or '\r' line ends.
	 *       Make it work with Windows and HTTP line ends '\r\n'.
	 * This abuses the following facts :
	 * - We currently only deal with one character line ends.
	 * - fgetc will advance the cursor to the next character.
	 * So when :
	 *   fseek(-1) and fgetc() == line_separator OR EOF
	 * then
	 *   ftell() gives the line start position we're looking for.
	 *   Because fgetc() will do a fseek(+1) implicitly.
	 */
	off_t current_position = ftell(file);
	char current_char;

	if (current_position < 0)
		goto out;

	if (current_position == 0)
		goto got_line_start;

	do {
		current_position--;
		fseek(file, current_position, SEEK_SET);
		current_char = fgetc(file);
	} while(current_char != line_separator && current_position != 0);

got_line_start:
out:
	return;
}

void file_line_ranges(
	FILE * __restrict const file,
	array_line_ranges_t * __restrict const ranges)
{

	ranges->size = 0;
	file_seek_to_current_line_start(file);
	uint32_t line_i = 0;

	while(!feof(file) && line_i < MAX_LINES) {
		off_t line_start = ftell(file);
		off_t line_end   = file_current_line_end(file);
		ranges->data[line_i].start = line_start;
		ranges->data[line_i].stop  = line_end;

		line_i++;
		/* FIXME Hardcoded limit */
	}
}

off_t file_get_size(FILE * __restrict const file)
{
	off_t const current_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	off_t const end = ftell(file);
	fseek(file, current_pos, SEEK_SET);
	return end;
}

array_line_ranges_t program_ini_line_ranges;
int parse_ini_file(
	char const * __restrict const gl_programs_ini_pathname)
{
	int ret = 0;

	FILE * const gl_programs_ini = fopen(gl_programs_ini_pathname, "r");
	
	if (gl_programs_ini == NULL) {
		/* Glibc extensions are a bad idea.
		 * However, I need to display a useful error message
		 */
		print_error("Could not open %s : %m", gl_programs_ini_pathname);
		ret = errno;
		goto out;
	}

	file_line_ranges(
		gl_programs_ini, &program_ini_line_ranges);
	if (program_ini_line_ranges.size == 0)
		goto out;

	metadata_parse_sections(gl_programs_ini,
			&program_ini_line_ranges);

	
no_more_titles:
	close(gl_programs_ini);
	
out:
	return ret;
}

int main(int argc, char const * const * const argv)
{
	int ret = 0;
	if (argc < 2) {
		print_usage(argv[0]);
		ret = -EINVAL;
		goto out;
	}

	char const * __restrict const shaders_path = argv[1];

	int parsed_fmt_size = snprintf(
		programs_definitions_pathname,
		PROGRAMS_DEFINITIONS_PATHNAME_COPY_SIZE,
		"%s%cprograms_list.ini",
		shaders_path, path_separator);

	if (parsed_fmt_size >= PROGRAMS_DEFINITIONS_PATHNAME_COPY_SIZE
		|| parsed_fmt_size < 0 /* Overflow cases */)
	{
		print_error(
			"The pathname of the file defining the OpenGL shaders programs "
			"should not be longer than 4096 characters.\n"
			"It's actually %d characters long :\n%s%cprograms_list.ini",
			parsed_fmt_size, shaders_path, path_separator);
		ret = -ENOMEM;
		goto out;
	}

	parse_ini_file(programs_definitions_pathname);

out:
	return ret;
}
