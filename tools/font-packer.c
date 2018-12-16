#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h> // (u)int*_t

#include <sys/types.h> // open
#include <sys/stat.h>  // open
#include <fcntl.h>     // open

#include <unistd.h> // read, close

#include <vector.h>

#include <myy/helpers/strings.h>
#include <myy/helpers/file.h>

#ifndef MYY_C_TYPES
#define MYY_C_TYPES 1
typedef uint_fast8_t bool;
enum bool_value { false, true };
#endif

#define max(a,b) ((a) > (b) ? (a) : (b))

enum program_status {
	status_ok,
	status_not_enough_arguments,
	status_cannot_open_font,
	status_could_not_initialize_fonts,
	status_not_enough_initial_memory_for_faces,
	status_not_enough_initial_memory_for_bitmaps_metadata,
	status_not_enough_initial_memory_for_bitmaps,
	status_not_enough_initial_memory_for_codepoints,
	status_font_filepath_does_not_exist,
	status_chars_filepath_does_not_exist,
	status_could_not_load_codepoints_from_file,
	n_status
};

char const * __restrict const status_messages[n_status] = {
	[status_ok]                      = "",
	[status_not_enough_arguments]    =
		"Not enough arguments\n",
	[status_cannot_open_font]        =
		"The font could not be opened %m\n",
	[status_could_not_initialize_fonts] =
		"Failed to initialize the fonts with FreeType\n",
	[status_not_enough_initial_memory_for_faces] =
		"Not enough initial memory for Freetype Font Faces\n",
	[status_not_enough_initial_memory_for_bitmaps_metadata] =
		"Not enough initial memory for bitmaps metadata\n",
	[status_not_enough_initial_memory_for_bitmaps] =
		"Not enough initial memory for bitmaps\n",
	[status_not_enough_initial_memory_for_codepoints] =
		"Not enough initial memory for codepoints\n",
	[status_font_filepath_does_not_exist] =
		"The font filepath does not exist\n",
	[status_chars_filepath_does_not_exist] =
		"The chars filepath does not exist\n",
	[status_could_not_load_codepoints_from_file] =
		"Could not load the codepoints for the chars file\n",
};

struct bitmap_metadata {
	uint16_t width;
	uint16_t height;
	uint16_t stride;
	uint16_t size;
	uint8_t * data_address;
};

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
	if (FT_Get_Char_Index(face, utf32_code) == 0)
		printf("%u not found\n", utf32_code);

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

bool copy_char(FT_Face const face,
	myy_vector_t * __restrict const bitmaps,
	myy_vector_t * __restrict const bitmaps_metadata)
{
	bool ret = false;
	FT_Bitmap const bitmap = face->glyph->bitmap;

	struct bitmap_metadata metadata = {
		.width  = bitmap.width,
		.height = bitmap.rows,
		.stride = bitmap.pitch,
		.size   = bitmap.rows * bitmap.pitch,
	};

	ret = myy_vector_add(bitmaps_metadata,
		sizeof(metadata),
		(uint8_t const * __restrict) &metadata);

	if (!ret) {
		fprintf(stderr, "Could not add metadata\n");
		goto out;
	}

	ret = myy_vector_add(bitmaps, metadata.size,
		bitmap.buffer);
	if (!ret) {
		fprintf(stderr, "Could not generate the bitmap\n");
		myy_vector_move_tail_back(bitmaps_metadata,
			sizeof(metadata));
		goto out;
	}

out:
	return ret;
}

static FT_Face first_face_that_can_display(
	myy_vector_t const * __restrict const faces,
	uint32_t codepoint)
{
	myy_vector_for_each(faces, FT_Face, face, {
		printf("Searching %u in %s... ", codepoint, face->family_name);
		if (FT_Get_Char_Index(face, codepoint)) {
			printf("OK !\n");
			return face;
		}
		printf("Nope.\n", codepoint);
	});
	printf("No provided font can display : %u\n",
		codepoint);
	return NULL;
}

static void add_addresses_of_each_bitmap(
	myy_vector_t const * __restrict const bitmaps_metadata,
	myy_vector_t const * __restrict const bitmaps)
{
	uint8_t * __restrict cursor =
		myy_vector_data(bitmaps);

	int i = 0;
	myy_vector_for_each_ptr(
		bitmaps_metadata, struct bitmap_metadata, metadata,
		{
			metadata->data_address = cursor;
			cursor += metadata->size;
			i++;
		}
	);
	printf("%d bitmaps\n", i);
}

void compute_each_bitmap_individually(
	myy_vector_t const * __restrict const faces,
	myy_vector_t const * __restrict const codepoints,
	myy_vector_t * __restrict const bitmaps,
	myy_vector_t * __restrict const bitmaps_metadata)
{
	myy_vector_for_each(codepoints, uint32_t, codepoint, {
		FT_Face face =
			first_face_that_can_display(faces, codepoint);
		if (face != NULL && load_char(face, codepoint))
			copy_char(face, bitmaps, bitmaps_metadata);
	});
	/* Since the bitmaps have flexible sizes and the
	 * vector used to store them can expand when adding
	 * data to it, and change its base address when
	 * expanding, we compute the addresses of each bitmap
	 * at the end.
	 * The computing is done by scanning every metadata
	 * adding the address of the current cursor in the
	 * bitmaps metadata, and advance the cursor by the
	 * bitmap size.
	 */
	add_addresses_of_each_bitmap(bitmaps_metadata,
		bitmaps);
}

struct global_statistics {
	size_t total_height;
	size_t max_width;
};

struct global_statistics bitmaps_statistics(
	myy_vector_t const * __restrict const bitmaps_metadata)
{
	struct global_statistics stats = {0, 0};

	myy_vector_for_each(
		bitmaps_metadata, struct bitmap_metadata, metadata,
		{
			/* Compute statistics */
			stats.total_height += metadata.height;
			stats.max_width    = max(stats.max_width, metadata.width);
		}
	);

	return stats;
}

void print_bitmaps(
	myy_vector_t const * __restrict const bitmaps_metadata)
{

	printf("Miaou");
	size_t total_height = 0;
	size_t max_width    = 0;

	myy_vector_for_each(
		bitmaps_metadata, struct bitmap_metadata, metadata,
		{
			uint8_t const * __restrict pixels =
				metadata.data_address;

			/* Compute statistics */
			total_height += metadata.height;
			max_width    = max_width > metadata.width
				? max_width
				: metadata.width;

			/* Print the pixels */
			printf("\n");
			for (uint16_t h = 0; h < metadata.height; h++)
			{
				for (uint16_t w = 0; w < metadata.width; w++)
				{
					print_pixel(pixels[w]);
				}
				printf("\n");
				pixels += metadata.stride;
			}
			printf("\n");
		}
	);

	printf(
		"Total height : %lu\n"
		"Max width    : %lu\n",
		total_height, max_width);
}



uint_fast8_t fh_file_exist(char const * __restrict const filepath)
{
	int fd = open(filepath, O_RDONLY);
	int could_open_file = (fd >= 0);
	if (could_open_file)	close(fd);
	return could_open_file;
}

static void print_usage(char const * __restrict const program_name)
{
	printf(
		"Usage : %s /path/to/font/file.ttf [/path/to/other/font.otf, ...] /path/to/chars.txt\n",
		program_name);
}

static bool chars_to_codepoints(
	myy_vector_t * __restrict const codepoints,
	uint8_t const * __restrict const utf8_chars,
	size_t const utf8_chars_size)
{
	uint8_t const * __restrict cursor = utf8_chars;
	uint8_t const * __restrict const end =
		cursor + utf8_chars_size;

	uint32_t i = 0;
	while(cursor < end) {
		struct utf8_codepoint codepoint =
			utf8_codepoint_and_size(cursor);

		bool added = myy_vector_add(
			codepoints, sizeof(uint32_t),
			(uint8_t const * __restrict) &codepoint.raw);

		if (!added) break;

		cursor += codepoint.size;
		i++;
	}

	return cursor >= end;
}

static int compare_codepoints(void const * pa, void const * pb)
{
	uint32_t const a = *((uint32_t const *) pa);
	uint32_t const b = *((uint32_t const *) pb);

	return a - b;
}

static void uniq(
	myy_vector_t * __restrict const codepoints_data)
{
	size_t vector_size =
		myy_vector_last_offset(codepoints_data)/sizeof(uint32_t);

	uint32_t * const sorted_codepoints =
		(uint32_t *) codepoints_data->begin;
	uint32_t const * const codepoints =
		(uint32_t const *) codepoints_data->begin;

	uint32_t uniques = 1;

	for (uint32_t before = 0, i = 1; i < vector_size; before++, i++)
	{
		uint32_t codepoint = codepoints[i];
		if (codepoint != codepoints[before])
			sorted_codepoints[uniques++] = codepoint;
	}

	codepoints_data->tail =
		(uintptr_t) (sorted_codepoints + uniques);
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
		codepoints, mapping_result.address, mapping_result.length);

	fh_UnmapFileFromMemory(mapping_result);

	if (!ret)
		goto could_not_convert_all_codepoints;

	qsort(myy_vector_data(codepoints),
		myy_vector_last_offset(codepoints)/sizeof(uint32_t),
		sizeof(uint32_t),
		compare_codepoints);

	uniq(codepoints);

	printf("%lu\n", myy_vector_last_offset(codepoints)/sizeof(uint32_t));
could_not_convert_all_codepoints:
cannot_map_file:
cannot_open_chars:
	return ret;
}

static bool fonts_deinit(
	FT_Library * library,
	myy_vector_t * __restrict const faces)
{
	printf("In\n");
	myy_vector_for_each(faces, FT_Face, face_to_delete, {
		FT_Done_Face(face_to_delete);
	});
	printf("Between\n");
	FT_Done_FreeType(*library);
	printf("Out\n");
}

static bool fonts_init(
	FT_Library * library,
	char const * __restrict const * __restrict const fonts_filepath,
	uint32_t n_fonts,
	myy_vector_t * __restrict const faces)
{

	if (FT_Init_FreeType( library ) != 0)
		goto ft_init_failed;


	for (uint32_t i = 0; i < n_fonts; i++) {
		FT_Face face;
		if (FT_New_Face( *library, fonts_filepath[i], 0, &face) != 0)
			goto could_init_all_fonts;

		/* Add the face one initialized, so that we can free it
		 * ASAP if we fail afterwards
		 */
		myy_vector_add(faces, sizeof(faces), (uint8_t *) &face);

		if (FT_Set_Char_Size(face, 16*64, 0, 96, 0) != 0)
			goto could_not_set_char_size_on_all_fonts;

	}

	return true;

could_not_set_char_size_on_all_fonts:
could_init_all_fonts:
	fonts_deinit(library, faces);
ft_init_failed:
	return false;
}

static int compare_bitmaps_by_width(void const * pa, void const * pb)
{
	struct bitmap_metadata * a = (struct bitmap_metadata *) pa;
	struct bitmap_metadata * b = (struct bitmap_metadata *) pb;

	return a->width - b->width;
}

static void sort_bitmaps_metadata_by_width(
	myy_vector_t * __restrict const metadata)
{
	qsort(myy_vector_data(metadata),
		myy_vector_last_offset(metadata)
		/ sizeof(struct bitmap_metadata),
		sizeof(struct bitmap_metadata),
		compare_bitmaps_by_width);
}



/* TODO : Padding is hard coded right now.
 * Provide the arguments for left and right padding.
 * Use them in the utility functions
 */
static inline uint8_t * add_padding_left(uint8_t * __restrict texture)
{
	*texture++ = 0;
	return texture;
}

/* TODO : Padding is hard coded right now.
 * Provide the arguments for left and right padding.
 * Use them in the utility functions
 */
static inline uint8_t * add_padding_right(uint8_t * __restrict texture)
{
	*texture++ = 0;
	return texture;
}

/* TODO : Padding is hard coded right now.
 * Provide the arguments for left and right padding.
 * Use them in the utility functions
 */
static inline uint8_t * add_lower_padding(
	uint8_t * __restrict texture,
	uint16_t texture_width)
{
	while(texture_width--)
		*texture++ = 0;

	return texture;
}

/* TODO : Padding is hard coded right now.
 * Provide the arguments for left and right padding.
 * Use them in the utility functions
 */
static inline uint8_t * add_upper_padding(
	uint8_t * __restrict texture,
	uint16_t texture_width)
{
	while(texture_width--)
		*texture++ = 0;

	return texture;
}

/* TODO : Padding is hard coded right now.
 * Provide the arguments for left and right padding.
 * Use them in the utility functions
 */
static inline void blit(
	uint8_t * __restrict dst,
	uint16_t const dst_width,
	struct bitmap_metadata bitmap_infos)
{
	uint8_t const * __restrict bitmap_cursor =
		(uint8_t const * __restrict)
		bitmap_infos.data_address;

	for (uint16_t h = 0; h < bitmap_infos.height;
	     h++,
	     bitmap_cursor += bitmap_infos.stride)
	{
		uint8_t * __restrict const start = dst;

		dst = add_padding_left(dst);

		for (uint_fast16_t p = 0; p < bitmap_infos.width; p++)
			dst[p] = bitmap_cursor[p];

		add_padding_right(dst+bitmap_infos.width);
		dst = start + dst_width;
	}
}


static void generate_bitmap(
	myy_vector_t * bitmaps_metadata,
	uint8_t * __restrict texture,
	uint32_t const total_height,
	uint32_t const max_height)
{
	uint32_t const padding = 1;
	uint32_t const sides_padding = padding * 2;

	/* TODO
	 * This completely ignores the maximal padded width
	 * of any character
	 * If the maximum is 37 for example, you wouldn't
	 * be able to fit a single character into one column.
	 */
	uint32_t const column_min_width = 32; /* px */

	uint32_t const n_columns =
		(max_height + total_height + sides_padding) / 4096;
	uint32_t const total_width = column_min_width << (n_columns);

	uint32_t h = 0;
	uint32_t current_line_width = 0;
	uint32_t current_line_max_height = 0;

	uint8_t * __restrict const texture_start = texture;

	texture = add_lower_padding(texture, total_width);
	h += padding;

	myy_vector_for_each(bitmaps_metadata,
		struct bitmap_metadata, metadata,
		{
			uint32_t const padded_width =
				metadata.width + sides_padding;
			uint32_t const added_width =
				current_line_width + padded_width;
			if (added_width < total_width)
			{
				blit(texture, total_width, metadata);
				current_line_max_height =
					max(current_line_max_height, metadata.height);
				current_line_width = added_width;
				texture += padded_width;
			}
			else if (metadata.width <= total_width)
			{
				h += current_line_max_height + sides_padding;
				texture = texture_start + (h * total_width);
				blit(texture, total_width, metadata);
				texture += padded_width;
				current_line_width = padded_width;
				current_line_max_height = metadata.height;
			}
		}
	);

	h += current_line_max_height + padding;

	uint32_t const n_pixels = h * total_width;
	uint16_t const mask = total_width - 1;
	for (uint_fast32_t p = 0; p < n_pixels; p++)
	{
		if ((p & mask) == mask) printf("\n");
		print_pixel(texture_start[p]);
	}

	printf("total_width : %u\ntotal_height : %u\n", total_width, h);
}

#define BITMAP_AVERAGE_SIZE (20*25)
/* TODO :
 * - Generate the big bitmap by packing as much elements on a
 *   single line.
 *   The whole logic is :
 *   - Pack all the characters with no width or no height into
 *     the same place.
 *   - Pack all characters with a width below 8 together
 *   - Pack the remaining characters, taking a character and
 *     packing it with a the character that will mostly fill
 *     the remaining space.
 */
int main(int const argc, char const * const * const argv)
{
	enum program_status ret = status_ok;
	int n_fonts;
	char const * const * __restrict fonts_filepath;
	char const * __restrict chars_filepath;

	bool bool_ret;
	FT_Library library;
	myy_vector_t faces = myy_vector_init(sizeof(FT_Face)*16);
	myy_vector_t bitmaps_metadata =
		myy_vector_init(1024*sizeof(struct bitmap_metadata));
	myy_vector_t bitmaps =
		myy_vector_init(1024*BITMAP_AVERAGE_SIZE);
	myy_vector_t codepoints =
		myy_vector_init(1*1024*1024);
	/* The texture isn't supposed to expand endlessly.
	 * The maximum authorized size is 4Kx4K.
	 * After that, you'll need another one.
	 */
	uint8_t * __restrict texture =
		(uint8_t * __restrict) malloc(4096*4096);
	memset(texture, 0, 4096*4096);

	if (!myy_vector_is_valid(&faces))
	{
		ret = status_not_enough_initial_memory_for_faces;
		goto not_enough_initial_memory_for_faces;
	}
	if (!myy_vector_is_valid(&bitmaps_metadata)) 
	{
		ret = status_not_enough_initial_memory_for_bitmaps_metadata;
		goto not_enough_initial_memory_for_bitmaps_metadata;
	}

	if (!myy_vector_is_valid(&bitmaps))
	{
		ret = status_not_enough_initial_memory_for_bitmaps;
		goto not_enough_initial_memory_for_bitmaps;
	}

	if (!myy_vector_is_valid(&codepoints))
	{
		ret = status_not_enough_initial_memory_for_codepoints;
		goto not_enough_initial_memory_for_codepoints;
	}

	if (argc < 3) {
		ret = status_not_enough_arguments;
		print_usage(argv[0]);
		goto not_enough_arguments;
	}

	n_fonts = argc-2;
	fonts_filepath = argv+1;
	chars_filepath = argv[argc-1];

	printf("Fonts filepath :\n");
	for (uint32_t i = 0; i < n_fonts; i++) {
		printf("\t%s\n", fonts_filepath[i]);
	}

	printf("Chars filepath : %s\n", chars_filepath);

	for (uint_fast32_t i = 0; i < n_fonts; i++) {
		if (!fh_file_exist(fonts_filepath[i])) {
			fprintf(stderr, "%s not_found\n", fonts_filepath[i]);
			ret = status_font_filepath_does_not_exist;
			goto font_filepath_does_not_exist;
		}
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

	if (!fonts_init(&library, fonts_filepath, n_fonts, &faces))
	{
		ret = status_could_not_initialize_fonts;
		goto could_not_initialize_fonts;
	}

	compute_each_bitmap_individually(
		&faces, &codepoints, &bitmaps, &bitmaps_metadata);
	sort_bitmaps_metadata_by_width(&bitmaps_metadata);
	//print_bitmaps(&bitmaps_metadata);
	struct global_statistics global_metadata =
		bitmaps_statistics(&bitmaps_metadata);
	generate_bitmap(&bitmaps_metadata, texture,
		global_metadata.max_width, global_metadata.total_height);

	fonts_deinit(&library, &faces);
could_not_initialize_fonts:
could_not_load_codepoints_from_file:
chars_filepath_does_not_exist:
font_filepath_does_not_exist:
not_enough_arguments:
	myy_vector_free_content(codepoints);
not_enough_initial_memory_for_codepoints:
	myy_vector_free_content(bitmaps);
not_enough_initial_memory_for_bitmaps:
	myy_vector_free_content(bitmaps_metadata);
not_enough_initial_memory_for_bitmaps_metadata:
	myy_vector_free_content(faces);
not_enough_initial_memory_for_faces:
	fprintf(stderr, "%s", status_messages[ret]);
	return ret;
}
