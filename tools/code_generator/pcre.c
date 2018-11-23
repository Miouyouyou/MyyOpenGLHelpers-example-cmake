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

#define print_error(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#define whine(fmt, ...)       fprintf(stderr, fmt "\n", ##__VA_ARGS__)

char const path_separator = '/';
char const line_separator = '\n';

#define PROGRAMS_DEFINITIONS_PATHNAME_SIZE 4096
#define PROGRAMS_DEFINITIONS_PATHNAME_COPY_SIZE \
	(PROGRAMS_DEFINITIONS_PATHNAME_SIZE - 1)

char programs_definitions_pathname[PROGRAMS_DEFINITIONS_PATHNAME_SIZE]
	= {0};

void print_usage(char const * __restrict const program_name)
{
	char const * __restrict const printed_program_name =
		program_name != NULL ? program_name : "code_generator";
	printf(
		"Usage : %p path/to/shaders/folder",
		printed_program_name);
}

int parse_programs_names(
	char const * __restrict const gl_programs_ini_pathname)
{
	
	return 0;
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

	parse_programs_names(programs_definitions_pathname);

out:
	return ret;
}

#define LOOKING_FOR_TITLE 0x0
#define PARSING_TITLE     0x1
#define PARSING_PROGRAMS  0x2
#define PARSING_TEXTURES  0x4
#define PARSING_BUFFERS   0x8

#define PROGRAMS_GO_TO_FIRST_CHAR      0x0
#define PROGRAMS_PARSE_TITLE           0x1
#define PROGRAMS_PARSE_SHADER_TYPE     0x2
#define PROGRAMS_PARSE_SHADER_PATHNAME 0x4

#define TEXTURES_GO_TO_FIRST_CHAR 0x0
#define TEXTURES_PARSE_TITLE      0x1
#define TEXTURES_PARSE_FILEPATH   0x2

#define BUFFERS_GO_TO_FIRST_CHAR 0x0
#define BUFFERS_PARSE_NAME       0x1
#define BUFFERS_PARSE_CLONES     0x2
#define BUFFERS_PARSE_TYPE       0x4
#define BUFFERS_PARSE_SIZE       0x8

#define IS_NEXT_LINE(first_char, second_char) (second_char == '\n')

#ifndef __cplusplus
enum bool_value { false, true };
typedef enum bool_value bool;
#endif

static inline bool is_title(
	char const * __restrict const buffer,
	char const * __restrict const title,
	uint_fast32_t const buffer_size)
{
	return strncmp(title, buffer, buffer_size) == 0;
}

uint8_t scratch_buffer[4096];
int parse_programs_ini(
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

	char previous_char = -1;
	char current_char = fgetc(gl_programs_ini);

	uint8_t * current_line = scratch_buffer;
	uint_fast32_t state = LOOKING_FOR_TITLE;
	uint_fast32_t sub_state = 0;
	uint_fast32_t line_start = 1;

	uint32_t stored_characters = 0;
	uint32_t max_characters = 4090;
	

	while(current_char != EOF) {
		switch(state) {
			case LOOKING_FOR_TITLE:
				if (!line_start) {
					whine(
						"%s",
						"First characters of the INI file should be a [title]\n"
						"Also no spaces are allowed before the [title]\n");
					ret = -EINVAL;
					goto malformed_ini_file;
				}
				if (current_char == '[') {
					state = PARSING_TITLE;
					stored_characters = 0;
				}
			break;
			case PARSING_TITLE:
				if (current_char == ']') {
					if (is_title("programs", (char *) current_line, stored_characters))
					{
						state = PARSING_PROGRAMS;
						sub_state = PROGRAMS_GO_TO_FIRST_CHAR;
					}
					else if (is_title("textures", (char *) current_line, stored_characters))
					{
						state = PARSING_TEXTURES;
						sub_state = TEXTURES_GO_TO_FIRST_CHAR;
					}
					else if (is_title("buffers", (char *) current_line, stored_characters))
					{
						state = PARSING_BUFFERS;
						sub_state = BUFFERS_GO_TO_FIRST_CHAR;
					}
					else {
						whine("Invalid section : %s", current_line);
						ret = -EINVAL;
						goto malformed_ini_file;
					}
					break;
				}
				if (IS_NEXT_LINE(previous_char, current_char)) {
					whine("Line feed in section title : %s", current_line);
					ret = -EINVAL;
					goto malformed_ini_file;
				}
				current_line[stored_characters++] = current_char;
				if (stored_characters >= max_characters) {
					current_line[max_characters-1] = 0;
					whine("Title %s too long. More than %d characters long",
						current_line, max_characters);
					ret = -EINVAL;
					goto malformed_ini_file;
				}
			break;
			case PARSING_PROGRAMS: {
				switch(sub_state) {
				case PROGRAMS_GO_TO_FIRST_CHAR:
					if (line_start && current_char == '[') {
						state = PARSING_TITLE;
					}
					else if (isalpha(current_char)) {
						sub_state = PROGRAMS_PARSE_TITLE;
						memset(current_line, 0, max_characters);
						stored_characters = 0;
						continue; // Restart to PROGRAMS_PARSE_TITLE
					}
					break;
				case PROGRAMS_PARSE_TITLE:
					if (isalnum(current_char)) {
						current_line[stored_characters+1] = current_char;
						if (stored_characters >= max_characters-1) {
							current_line[max_characters-1] = 0;
							whine("Title %s too long. More than %d characters long",
								current_line, max_characters);
							ret = -EINVAL;
							goto malformed_ini_file;
						}
					}
					else if (current_char == ':') {
						/* FIXME: Store title */
						sub_state = PROGRAMS_PARSE_SHADER_TYPE;
						memset(current_line, 0, max_characters);
						stored_characters = 0;
					}
					else {
						whine(
							"Malformed shader program name.\n"
							"Program names use the C identifier naming convention.\n"
							"One ASCII letter followed by zero or more \n"
							"alphanumeric characters");
						ret = -EINVAL;
						goto malformed_ini_file;
					}
					break;
				case PROGRAMS_PARSE_SHADER_TYPE:
					if (isalpha(current_char)) {
						current_line[stored_characters++] = current_char;
						if (stored_characters >= max_characters) {
							current_line[max_characters-1] = 0;
							whine("Title %s too long. More than %d characters long",
								current_line, max_characters);
							ret = -EINVAL;
							goto malformed_ini_file;
						}
					}
					else if (current_char == ',') {
						
					}
					else if (!isspace(current_char)) {
					}
					break;
				case PROGRAMS_PARSE_SHADER_PATHNAME:
					
					break;
				}
			}
			break;
			case PARSING_TEXTURES: {
				switch(sub_state) {
					case TEXTURES_GO_TO_FIRST_CHAR:
						break;
					case TEXTURES_PARSE_TITLE:
						break;
					case TEXTURES_PARSE_FILEPATH:
						break;
				}
			break;
			case PARSING_BUFFERS: {
				switch(sub_state) {
					case BUFFERS_GO_TO_FIRST_CHAR:
						break;
					case BUFFERS_PARSE_NAME:
						break;
					case BUFFERS_PARSE_CLONES:
						break;
					case BUFFERS_PARSE_TYPE:
						break;
					case BUFFERS_PARSE_SIZE:
						break;
				}
			}
			break;
		}
		line_start = IS_NEXT_LINE(previous_char, current_char);
		previous_char = current_char;
		current_char = fgetc(gl_programs_ini);
	}

malformed_ini_file:
	fclose(gl_programs_ini);
out:
	return ret;
}
