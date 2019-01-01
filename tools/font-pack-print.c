#include <myy/helpers/file.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/strings.h>

#include <stdio.h>

void print_codepoint_and_metadata(
	struct myy_packed_fonts_glyphdata const * __restrict const glyphs,
	uint32_t const * __restrict const codepoints,
	uint_fast32_t const index)
{
	struct myy_packed_fonts_glyphdata const glyph = glyphs[index];
	uint32_t codepoint = codepoints[index];
	uint8_t codepoint_str[5] = {0};
	utf32_to_utf8_string(codepoint, codepoint_str);
	printf(
		"Codepoint  : %d (%s)\n"
		"Glyph_data\n"
		"\tTexture (%d←→%d) (%d↓↑%d)\n"
		"\tOrigin offset (→%d, ↓%d)\n"
		"\tAdvance (→%d, ↓%d)\n",
		codepoint, codepoint_str,
		glyph.tex_left,     glyph.tex_right,
		glyph.tex_bottom,   glyph.tex_top,
		glyph.offset_x_px,  glyph.offset_y_px,
		glyph.advance_x_px, glyph.advance_y_px);
}

void print_codepoint_and_metadata_of(
	struct myy_packed_fonts_glyphdata const * __restrict const glyphs,
	uint32_t const * __restrict const codepoints,
	uint32_t n_codepoints,
	char const * __restrict utf8_character)
{
	uint32_t searched_codepoint =
		utf8_codepoint_and_size(utf8_character).raw;

	for (uint32_t i = 0; i < n_codepoints; i++) {
		if (codepoints[i] == searched_codepoint) {
			print_codepoint_and_metadata(glyphs, codepoints, i);
			break;
		}
	}
}

int main()
{
	struct myy_fh_map_handle mapping =
		fh_MapFileToMemory("font_pack_meta.dat");
	if (mapping.ok) {
		struct myy_packed_fonts_info_header const * __restrict const
			header =
			(struct myy_packed_fonts_info_header * __restrict)
			mapping.address;
		uint32_t const * __restrict codepoints =
			(mapping.address + header->codepoints_start_offset);
		struct myy_packed_fonts_glyphdata const * __restrict const
			glyphs =
			(struct myy_packed_fonts_glyphdata * __restrict)
			(mapping.address + header->glyphdata_start_offset);
		printf(
			"Signature\n"
			"-> Provided : 0x%08x\n"
			"-> Expected : 0x%08x\n"
			"Codepoints stored      : %u\n"
			"Codepoints offset      : 0x%04x\n"
			"Glyphs metadata offset : 0x%04x\n"
			"Filenames offset       : 0x%04x\n",
			header->signature,
			MYYF_SIGNATURE,
			header->n_stored_codepoints,
			header->codepoints_start_offset,
			header->glyphdata_start_offset,
			header->texture_filenames_offset);

		print_codepoint_and_metadata_of(glyphs, codepoints, header->n_stored_codepoints, "g");
		fh_UnmapFileFromMemory(mapping);
	}
}
