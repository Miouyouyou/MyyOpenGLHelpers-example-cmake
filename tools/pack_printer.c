#include <stdio.h>
#include <stdint.h>

#include <myy/helpers/file.h>

#define MYYP_SIGNATURE 0x5059594D

struct pack_header {
	/* Must be 0x5059594D */
	uint32_t signature;
	uint32_t data_offset;
	uint32_t metadata_offset;
	uint32_t n_programs;
};

void print_header(
	struct pack_header const * __restrict const header)
{
	printf(
		"Signature       : 0x%08x\n"
		"Data offset     : 0x%04x\n"
		"Metadata offset : 0x%04x\n"
		"N programs      : %d\n",
		header->signature,
		header->data_offset,
		header->metadata_offset,
		header->n_programs);
}

struct program_metadata {
	uint32_t name_offset;
	uint32_t metadata_offset;
};

void print_remaining_metadata(
	uint8_t const * __restrict const metadata_section,
	uint8_t const * __restrict const data_section);
void print_program_metadata(
	struct program_metadata const * __restrict const metadata,
	uint8_t const * __restrict const data_section,
	uint8_t const * __restrict const metadata_section)
{
	printf(
		"Name offset     : 0x%08x (%s)\n"
		"Metadata offset : 0x%08x\n",
		metadata->name_offset,
		(data_section + metadata->name_offset),
		metadata->metadata_offset);

	print_remaining_metadata(
		metadata_section + metadata->metadata_offset,
		data_section);
}

struct program_definition_header {
	uint32_t section_size;
	uint32_t offset_in_struct;
};

struct shader_metadata {
	uint32_t shader_type;
	uint32_t data_size;
	uint32_t data_offset;
};

struct shaders_metadata {
	uint32_t section_size;
	uint32_t n_shaders;
	struct shader_metadata shader[];
};

struct attribute_metadata {
	uint32_t name_offset;
	uint32_t bind_id;
};

struct attributes_metadata {
	uint32_t section_size;
	uint32_t n_attributes;
	struct attribute_metadata attribute[];
};

struct uniform_metadata {
	uint32_t struct_offset;
	uint32_t name_offset;
};

struct uniforms_metadata {
	uint32_t section_size;
	uint32_t n_uniforms;
	struct uniform_metadata uniform[];
};

void print_remaining_metadata(
	uint8_t const * __restrict const metadata_section,
	uint8_t const * __restrict const data_section)
{
	uint8_t const * __restrict cursor = metadata_section;

	struct program_definition_header const * __restrict const
		program_header =
		(struct program_definition_header const * __restrict)
		cursor;

	printf(
		"-- Program definition header\n"
		"Section size        : 0x%04x\n"
		"Offset in structure : 0x%04x\n",
		program_header->section_size,
		program_header->offset_in_struct);

	cursor += program_header->section_size;

	struct shaders_metadata const * __restrict const shaders_mdata =
		(struct shaders_metadata const * __restrict)
		cursor;

	printf(
		"-- Shaders metadata\n"
		"Section_size : 0x%04x\n"
		"N shaders    : %d\n",
		shaders_mdata->section_size,
		shaders_mdata->n_shaders);

	for (uint_fast32_t i = 0; i < shaders_mdata->n_shaders; i++)
	{
		struct shader_metadata shader_mdata = shaders_mdata->shader[i];
		printf(
			"\t-- Shader :\n"
			"\tType        : 0x%04x\n"
			"\tData size   : 0x%04x\n"
			"\tData offset : 0x%04x\n",
			shader_mdata.shader_type,
			shader_mdata.data_size,
			shader_mdata.data_offset);
		printf(
			"\t--DATA--\n"
			"%s\n"
			"\t==DATA==\n",
			(data_section + shader_mdata.data_offset));
	}

	cursor += shaders_mdata->section_size;

	struct attributes_metadata const * __restrict const
		attributes_mdata =
		(struct attributes_metadata const * __restrict)
		cursor;

	printf(
		"-- Attributes metadata\n"
		"Section_size : 0x%04x\n"
		"N attributes : %d\n",
		attributes_mdata->section_size,
		attributes_mdata->n_attributes);

	for (uint_fast32_t i = 0; i < attributes_mdata->n_attributes; i++)
	{
		struct attribute_metadata attribute_mdata =
			attributes_mdata->attribute[i];
		printf(
			"\t-- Attribute :\n"
			"\tName offset : 0x%04x (%s)\n"
			"\tBind ID     : %d\n",
			attribute_mdata.name_offset,
			(data_section + attribute_mdata.name_offset),
			attribute_mdata.bind_id);
	}

	cursor += attributes_mdata->section_size;

	struct uniforms_metadata const * __restrict const
		uniforms_mdata =
		(struct uniforms_metadata const * __restrict)
		cursor;

	printf(
		"-- Uniforms metadata\n"
		"Section_size : 0x%08x\n"
		"N uniforms : %d\n",
		uniforms_mdata->section_size,
		uniforms_mdata->n_uniforms);

	for (uint_fast32_t i = 0; i < uniforms_mdata->n_uniforms; i++)
	{
		struct uniform_metadata uniform_mdata =
			uniforms_mdata->uniform[i];
		printf(
			"\t-- Uniform :\n"
			"\tStruct offset : %d\n"
			"\tName offset   : 0x%04x (%s)\n",
			uniform_mdata.struct_offset,
			uniform_mdata.name_offset,
			(data_section + uniform_mdata.name_offset));
	}
}

void print_usage(char const * __restrict const binary_name)
{
	char const * __restrict const printed_name =
		(binary_name != NULL)
		? binary_name
		: "pack_printer";

	printf("Usage : %s /path/to/shaders.pack", printed_name);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		print_usage(argv[0]);
		goto out;
	}

	struct myy_fh_map_handle mapped = fh_MapFileToMemory(argv[1]);
	if (mapped.ok) {
		struct pack_header const * __restrict const header =
			(struct pack_header const * __restrict)
			mapped.address;
		uint8_t const * __restrict data_section;
		uint8_t const * __restrict metadata_section;
		struct program_metadata * __restrict programs;
		if (header->signature != MYYP_SIGNATURE) {
			printf("Bad signature");
			goto unmap_file;
		}
		print_header(header);

		data_section     = mapped.address + header->data_offset;
		metadata_section = mapped.address + header->metadata_offset;
		programs         = mapped.address + sizeof(*header);

		for (uint_fast32_t i = 0; i < header->n_programs; i++) {
			print_program_metadata(
				programs+i,
				data_section,
				metadata_section);
		}
unmap_file:
		fh_UnmapFileFromMemory(mapped);
	}
out:
	return 0;
}
