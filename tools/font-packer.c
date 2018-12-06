#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h> // (u)int*_t

#include <sys/types.h> // open
#include <sys/stat.h>  // open
#include <fcntl.h>     // open

#include <unistd.h> // read, close

#include <vector.h>

#ifndef MYY_C_TYPES
#define MYY_C_TYPES 1
typedef uint_fast8_t bool;
enum bool_value { false, true };
#endif

static inline void print_pixel(uint_fast8_t pixel_value)
{
	struct {
		uint8_t size;
		uint8_t codepoints[4];
	} states[5] = {
		[0] = {1,{' '}},
		[1] = {3,{226, 150, 145}}, // ░
		[2] = {3,{226, 150, 146}}, // ▒
		[3] = {3,{226, 150, 147}}, // ▓
		[4] = {3,{226, 150, 136}}  // █
	};
	uint_fast8_t index = (pixel_value + 1) / 64;
	write(fileno(stdout), states[index].codepoints, states[index].size);
}

static bool load_char(FT_Face face, uint32_t utf32_code)
{
	int error = FT_Load_Char(face, utf32_code, FT_LOAD_DEFAULT);
	if (error != 0) {
                fprintf(stderr, "Could not get character %c : 0x%02x\n",
                        (uint8_t) utf32_code, error);
		goto could_not_load_character;
	}

	FT_GlyphSlot const glyph_slot = face->glyph;

	error = FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_NORMAL);

	if (error != 0) {
		fprintf(stderr, "Could not render character %c : 0x%02x\n",
			(uint8_t) utf32_code, error);
		goto could_not_render_glyph;
	}

could_not_render_glyph:
could_not_load_character:
	return error == 0;

}

enum program_status {
	status_ok,
	status_not_enough_arguments,
	status_cannot_open_font,
	status_ft_init_freetype_failed,
	status_ft_new_face_failed,
	status_ft_set_char_size_failed,
	n_status
};

char const * __restrict const status_messages[n_status] = {
	[status_ok]                      = "",
	[status_not_enough_arguments]    = "Not enough arguments\n",
	[status_cannot_open_font]        = "The font could not be opened %m\n",
	[status_ft_init_freetype_failed] = "FT_Init_FreeType failed\n",
	[status_ft_new_face_failed]      = "FT_New_Face failed\n",
	[status_ft_set_char_size_failed] = "FT_Set_Char_Size failed\n",
};

uint_fast8_t fh_file_exist(char const * __restrict const filepath)
{
	int fd = open(filepath, O_RDONLY);
	int could_open_file = (fd >= 0);
	if (could_open_file)	close(fd);
	return could_open_file;
}

static void print_usage(char const * __restrict const program_name)
{
	printf("Usage : %s /path/to/font/file.ttf /path/to/chars.txt\n",
		program_name);
}

struct added_bitmap {
	uint16_t width;
	uint16_t height;
	uint16_t stride;
	uint16_t bitmap_length;
};

bool copy_char(FT_Face const face,
	myy_vector_t * __restrict const vector)
{
	FT_GlyphSlot const glyph_slot = face->glyph;

	struct added_bitmap bitmap_metadata = {
		.width  = glyph_slot.bitmap.width,
		.height = glyph_slot.bitmap.rows,
		.stride = glyph_slot.bitmap.pitch,
		.size   = glyph_slot.bitmap.rows * glyph_slot.bitmap.pitch
	};

	return 
		myy_vector_add(bitmaps, sizeof(bitmap_metadata),
			(uint8_t const * __restrict) &bitmap_metadata)
		&& myy_vector_add(bitmaps, bitmap_metadata.size,
			glyph_slot.bitmap.buffer);
}

static bool chars_to_codepoints(
	myy_vector_t * __restrict const codepoints,
	uint8_t const * __restrict const utf8_chars)
{
	uint8_t * __restrict const cursor =
		(uint8_t const * __restrict) codepoints.begin;
	uint8_t * __restrict const end =
		(uint8_t const * __restrict) codepoints.last;

	
	while(cursor < end) {
		struct utf8_codepoint codepoint = utf8_codepoint_and_size(
			codepoints, utf8_chars);
		bool added = myy_vector_add(
			codepoints, sizeof(uint32_t), codepoints.raw);
		if (!added) break;
		cursor += codepoint.size;
	}

	return cursor >= end;
}

static int compare_codepoints(void const * pa, void const * pb)
{
	uint32_t const a = *((uint32_t const *) a);
	uint32_t const b = *((uint32_t const *) b);

	return a - b;
}

static void uniq(
	myy_vector_t * __restrict const codepoints)
{
	size_t vector_size =
		myy_vector_last_offset(&codepoints)/sizeof(uint32_t);

	uint32_t * __restrict sorted_codepoints =
		(uint32_t const * __restrict) codepoints.begin;

	uint32_t uniques = 1;

	for (uint32_t before = 0, i = 1; i < vector_size; i++)
	{
		uint32_t codepoint = sorted_codepoints[i];
		if (codepoint != sorted_codepoints[before])
			sorted_codepoints[uniques] = codepoint[i]
	}

	codepoints.last =
		sorted_codepoints + uniques;
}

static bool load_codepoints_from_chars_file(
	myy_vector_t * __restrict const codepoints,
	char const * __restrict const chars_filepath)
{
	bool ret = false;

	if (!fh_file_exist(chars_filepath))
		goto cannot_open_chars;

	struct myy_fh_map_handle mapping_result = 
		fh_MapFileToMemory(chars_filepath);

	if (!mapping_result.ok)
		goto cannot_map_file;

	ret = chars_to_codepoints(
		codepoints, mapping_result.address);

	fh_UnmapFileFromMemory(chars_filepath);

	if (!ret)
		goto could_not_convert_all_codepoints;

	qsort(codepoints,
		myy_vector_last_offset(&codepoints),
		sizeof(uint32_t),
		compare_codepoints);

	uniq(codepoints);

could_not_convert_all_codepoints:
cannot_map_file:
cannot_open_chars:
	return ret;
}

int main(int argc, char **argv) {
	enum program_status ret = status_ok;
	char const * __restrict font_filepath;
	char const * __restrict chars_filepath;

	bool bool_ret;
	FT_Library library;
	FT_Face face;
	myy_vector_t const bitmaps =
		myy_vector_init(1*1024*1024);
	myy_vector_t const codepoints =
		myy_vector_init(1*1024*1024);

	if (!myy_vector_valid(&bitmaps)
	    || !myy_vector_valid(&codepoints))
	{
		ret = status_not_enough_initial_memory;
		goto not_enough_initial_memory;
	}

	if (argc < 3) {
		ret = status_not_enough_arguments;
		print_usage(argv[0]);
		goto not_enough_arguments;
	}

	font_filepath  = argv[1];
	chars_filepath = argv[2];

	if (!fh_file_exist(font_filepath)) {
		ret = status_font_filepath_does_not_exist;
		goto font_filepath_does_not_exist;
	}

	if (!fh_file_exist(chars_filepath)) {
		ret = status_chars_filepath_does_not_exist;
		goto chars_filepath_does_not_exist;
	}

	bool_ret = load_codepoints_from_chars_file(
		&codepoints, chars_filepath);
	if (!bool_ret)
	{
		ret = status_could_not_load_codepoints_from_file;
		goto could_not_load_codepoints_from_file;
	}

	if (FT_Init_FreeType( &library ) != 0)
	{
		ret = status_ft_init_freetype_failed;
		goto freetype_init_failed;
	}

	if (FT_New_Face( library, font_filepath, 0, &face) != 0)
	{
		ret = status_ft_new_face_failed;
		goto ft_new_face_failed;
	}

	if (FT_Set_Char_Size(face, 16*64, 0, 96, 0) != 0)
	{
		ret = status_ft_set_char_size_failed;
		goto ft_set_char_size_failed;
	}

	/**
	 * 1. During the bitmap generations, compute the maximum
	 *    width and height and determine how many columns
	 *    are needed.
	 * 
	 *    The height must be a multiple of 2 lower or equivalent
	 *    to 4096.
	 * 
	 * 2. Store the bitmaps by tuple of "columns" into a single
	 *    bitmap and generate the coordinates of each character
	 *    during the operation.
	 */
	struct compute_result result =
		compute_each_bitmap_individually(face, &codepoints, &bitmaps);
	generate_the_big_bitmap(&result, &codepoints, &bitmaps);
	/* 	myy_vector_foreach(codepoint, {
		if (load_char(face, codepoints))
			copy_char(face, bitmaps);
	}); */

ft_set_char_size_failed:
	FT_Done_Face(face);
ft_new_face_failed:
	FT_Done_FreeType( library );
freetype_init_failed:
	myy_vector_free_content(&chars);
cannot_allocate_memory_for_chars:
	fh_UnmapFileFromMemory(mapping_result);
cannot_read_chars:
cannot_open_chars:
cannot_open_font:
	myy_vector_free_content(&bitmaps);
cannot_allocate_memory_for_bitmaps:
not_enough_arguments:
	fprintf(stderr, status_messages[ret]);
	return ret;
}
