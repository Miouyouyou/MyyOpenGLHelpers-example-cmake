#include <stdint.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include "bmp.h"

#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/memory.h>
#include <myy/helpers/file.h>

// http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
static uint32_t n_zeroes_right(uint32_t v)
{
	/* Myy : The original version defined the array as "static"
	 * I'm not sure that putting this into some static address,
	 * instead of putting this in the heap, is "that" faster.
	 * You'll still get a cache miss on the first time, while you'll
	 * rarely do with the stack.
	 * 
	 * That's why I'm defining it like this. But without looking
	 * at the generated code, these are just random thoughts.
	 */
	/* Note : I still find this hack insane */
	uint8_t const MultiplyDeBruijnBitPosition[32] = 
	{
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return MultiplyDeBruijnBitPosition[
		((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
}

static inline uint_fast32_t max_from_left_shift(uint_fast8_t shift)
{
	return (1 << shift) - 1;
}

static inline uint_fast32_t convert_channel(
	uint_fast32_t const channel_value,
	uint_fast32_t const new_channel_max,
	uint_fast32_t const old_channel_max)
{
	/* Since integer division truncates, better do the
	 * multiplication first
	 */
	return channel_value * new_channel_max / old_channel_max;
}

#define TARGET_PIXEL (1024*32+33)

static void write_input_to_565(
	uint8_t const * __restrict cursor, uint16_t * __restrict out,
	uint_fast8_t const input_bpp,
	uint_fast32_t input_size,
	uint32_t const red_mask, uint32_t const green_mask,
	uint32_t const blue_mask)
{

	uint8_t const * __restrict const input_end = cursor + input_size;
	uint_fast8_t const red_shift   = n_zeroes_right(red_mask);
	uint_fast8_t const green_shift = n_zeroes_right(green_mask);
	uint_fast8_t const blue_shift  = n_zeroes_right(blue_mask);

	uint_fast8_t const out_red_shift   = 11;
	uint_fast8_t const out_green_shift = 5;
	uint_fast8_t const out_blue_shift  = 0;

	uint32_t pixel;
	uint_fast32_t const n_bytes_per_pixel = input_bpp / 8;
	uint_fast8_t r, g, b;

	uint_fast32_t n = 0;
	while(cursor < input_end) {
		/* Input */
		pixel = 0;
		for (uint_fast32_t i = 0; i < n_bytes_per_pixel; i++) {
			uint32_t const value = (cursor[i] << (i << 3));
			if (n == TARGET_PIXEL)
				printf("cursor[%lu] = %02x << %lu << 3 = %08x\n",
					i, cursor[i], i, value);
			pixel |= value;
		}
		cursor += n_bytes_per_pixel;

		r = (pixel &   red_mask) >>   red_shift;
		g = (pixel & green_mask) >> green_shift;
		b = (pixel &  blue_mask) >>  blue_shift;

		if (n == TARGET_PIXEL)
			printf(
				"pixel : %08x\n"
				"rgb : \n"
				"\tr: %08x & %08x >> %hhu -> %02x\n"
				"\tg: %08x & %08x >> %hhu -> %02x\n"
				"\tb: %08x & %08x >> %hhu -> %02x\n",
				pixel,
				pixel, red_mask, red_shift, r,
				pixel, green_mask, green_shift, g,
				pixel, blue_mask, blue_shift, b);

		/* Output */
		r = convert_channel(
			r,
			31,
			(red_mask >> red_shift));
		g = convert_channel(
			g,
			63,
			(green_mask >> green_shift));
		b = convert_channel(
			b,
			31,
			(blue_mask >> blue_shift));

		if (n == TARGET_PIXEL)
			printf("rgb_out : %u,%u,%u\n", r, g, b);
		pixel = (
			  (r <<   out_red_shift)
			| (g << out_green_shift)
			| (b <<  out_blue_shift));

		*out++ = pixel;
		n++;
	}
	return;
}

static void print_header(
	struct bitmap_common_header const * __restrict const header)
{
	printf(
		"Signature     : %c%c\n"
		"Complete size : %u\n"
		"Reserved1     : %u\n"
		"Reserved2     : %u\n"
		"Offset        : %x\n",
		header->signature[0], header->signature[1],
		header->complete_size,
		header->reserved1,
		header->reserved2,
		header->data_offset);
}

static void convert_bitmap_to_565(
	uint8_t const * __restrict const input, int const out_fd)
{
	struct bitmap_common_header header;
	struct bitmap_info_header_v5 metadata;
	uint16_t * __restrict out_bitmap;
	uint8_t const * __restrict cursor = input;

	header = *((struct bitmap_common_header *) cursor);

	print_header(&header);
	if (header.complete_size <= BITMAP_V5_METADATA_SIZE) {
		printf(
			"File size inferior to a header...\n"
			"Expected %lu. Got %u\n",
			BITMAP_V5_METADATA_SIZE, header.complete_size);
		goto err;
	}

	cursor += sizeof(header);

	metadata = *((struct bitmap_info_header_v5 *) cursor);

	if (metadata.info_header_size
	    != sizeof(struct bitmap_info_header_v5))
	{
		printf("Expected a header of size %zu. Got %u\n",
			sizeof(struct bitmap_info_header_v5),
			metadata.info_header_size);
		goto err;
	}

	cursor += sizeof(metadata);

	struct myy_raw_texture_header myyraw_header = {
		.signature = MYYT_SIGNATURE,
		.width     = metadata.width,
		.height    = metadata.height,
		.gl_target = GL_TEXTURE_2D,
		.gl_format = GL_RGB,
		.gl_type   = GL_UNSIGNED_SHORT_5_6_5,
		.alignment = 2,
		.reserved  = 0
	};

	write(out_fd, &myyraw_header, sizeof(myyraw_header));
	out_bitmap =
		(uint16_t * __restrict)
		allocate_temporary_memory(metadata.width * metadata.height * sizeof(uint16_t));
	write_input_to_565(
		cursor, out_bitmap,
		metadata.bpp,
		metadata.data_size,
		metadata.red_mask,
		metadata.green_mask,
		metadata.blue_mask);
	write(out_fd, out_bitmap, metadata.width * metadata.height * sizeof(uint16_t));
	return;
err:
	return;
}

static void print_usage(
	char const * __restrict const program_name)
{
	char const * __restrict displayed_name =
		(program_name != NULL ? program_name : "bmp-converter");
	printf(
		"Usage : %s /path/to/file /path/to/output RGB[A]nnn[n]\n"
		"Example : %s texture.bmp texture.raw RGBA5551\n",
		displayed_name, displayed_name);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		print_usage(argv[0]);
	}

	char const * __restrict const input_filepath  = argv[1];
	char const * __restrict const output_filepath = argv[2];

	struct myy_fh_map_handle handle = fh_MapFileToMemory(input_filepath);
	if (!handle.ok) return -1;

	int const out_fd = open(output_filepath, O_WRONLY|O_CREAT, 00664);
	if (out_fd < 0) return out_fd;

	convert_bitmap_to_565(handle.address, out_fd);

	close(out_fd);
	fh_UnmapFileFromMemory(handle);

	return 0;
}
