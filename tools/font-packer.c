#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h> // (u)int*_t

#include <sys/types.h> // open
#include <sys/stat.h>  // open
#include <fcntl.h>     // open

#include <unistd.h> // read, close

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

static void load_char(FT_Face face, uint32_t utf32_code)
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

	uint_fast32_t const height = face->glyph->bitmap.rows;
	uint_fast32_t const width  = face->glyph->bitmap.width;
	int_fast32_t  const stride  = face->glyph->bitmap.pitch;
	uint8_t const * __restrict bitmap_line =
		face->glyph->bitmap.buffer;
	for (uint_fast32_t h = 0; h < height;
	     h++, bitmap_line+=stride) {
		for (uint_fast32_t p = 0; p < width; p++) {
			print_pixel((uint_fast8_t) bitmap_line[p]);
		}
		write(fileno(stdout), "\n", 1);
	}

could_not_render_glyph:
could_not_load_character:
	return;

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
	printf("Usage : %s /path/to/font/file.ttf\n",
		program_name);
}

int main(int argc, char **argv) {
	enum program_status ret = status_ok;
	char const * __restrict font_filepath;

	FT_Library library;
	FT_Face face;

	if (argc < 2) {
		ret = status_not_enough_arguments;
		print_usage(argv[0]);
		goto not_enough_arguments;
	}

	font_filepath = argv[1];

	if (!fh_file_exist(font_filepath)) {
		ret = status_cannot_open_font;
		goto cannot_open_font;
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

	load_char(face, 28961);

ft_set_char_size_failed:
	FT_Done_Face(face);
ft_new_face_failed:
	FT_Done_FreeType( library );
freetype_init_failed:
cannot_open_font:
not_enough_arguments:
	fprintf(stderr, status_messages[ret]);
	return ret;
}
