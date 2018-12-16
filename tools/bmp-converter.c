#include <stdint.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include "bmp.h"

// http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
static uint32_t n_zeroes_right(uint32_t v)
{
	/* Myy : I'm not sure that putting this into some static address,
	 * instead of putting this in the heap, is "that" faster.
	 * You'll still get a cache miss on the first time, while you'll
	 * rarely do with the stack.
	 */
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
	return channel_value * new_channel_max / old_channel_max;
}

static void write_input_to_565(
	int const in_fd, int const out_fd,
	uint_fast8_t const input_bpp,
	uint_fast32_t input_size,
	uint32_t const red_mask, uint32_t const green_mask,
	uint32_t const blue_mask)
{

	uint_fast8_t const red_shift   = n_zeroes_right(red_mask);
	uint_fast8_t const green_shift = n_zeroes_right(green_mask);
	uint_fast8_t const blue_shift  = n_zeroes_right(blue_mask);

	uint_fast8_t const out_red_shift   = 11;
	uint_fast8_t const out_green_shift = 5;
	uint_fast8_t const out_blue_shift  = 0;

	uint32_t const n_bytes_per_read = input_bpp / 8;
	uint32_t pixel;
	uint_fast8_t r, g, b;

	while(input_size) {
		/* Input */
		int const read_bytes =
			read(in_fd, &pixel, n_bytes_per_read);
		if (read_bytes < n_bytes_per_read)
			goto err;

		r = (pixel &   red_mask) >>   red_shift;
		g = (pixel & green_mask) >> green_shift;
		b = (pixel &  blue_mask) >>  blue_shift;
		if (input_size <= n_bytes_per_read)
			printf("rgb : %u,%u,%u\n", r, g, b);

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

		if (input_size <= n_bytes_per_read)
			printf("rgb_out : %u,%u,%u\n", r, g, b);
		pixel = (
			  (r <<   out_red_shift)
			| (g << out_green_shift)
			| (b <<  out_blue_shift));

		/* Little-Endian magic */
		write(out_fd, &pixel, 2);
		
			
		input_size -= n_bytes_per_read;
	}
	return;

err:
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
	int const in_fd, int const out_fd)
{
	struct bitmap_common_header header;
	struct bitmap_info_header_v5 metadata;

	read(in_fd,
		 (uint8_t * __restrict) &header,
		 sizeof(header));

	print_header(&header);
	if (header.complete_size <= BITMAP_V5_METADATA_SIZE) {
		printf(
			"File size inferior to a header...\n"
			"Expected %u. Got %u\n",
			BITMAP_V5_METADATA_SIZE, header.complete_size);
		goto err;
	}

	read(in_fd, &metadata, sizeof(metadata));
	if (metadata.info_header_size
	    != sizeof(struct bitmap_info_header_v5))
	{
		printf("Expected a header of size %u. Got %u\n",
			sizeof(struct bitmap_info_header_v5),
			metadata.info_header_size);
		goto err;
	}

	write_input_to_565(
		in_fd, out_fd,
		metadata.bpp,
		metadata.data_size,
		metadata.red_mask,
		metadata.blue_mask,
		metadata.green_mask);
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

void main(int argc, char **argv)
{
	if (argc < 3) {
		print_usage(argv[0]);
	}

	char const * __restrict const input_filepath  = argv[1];
	char const * __restrict const output_filepath = argv[2];

	int const in_fd = open(input_filepath, O_RDONLY);
	if (in_fd < 0) return;

	int const out_fd = open(output_filepath, O_WRONLY|O_CREAT, 00664);
	if (out_fd < 0) return;

	convert_bitmap_to_565(in_fd, out_fd);

	close(out_fd);
	close(in_fd);
}

/*
	

	char const * __restrict const file_format     = argv[3];





	
	close(output_filepath);
*/
