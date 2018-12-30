struct program_definition_header {
	uint32_t shaders_section_offset;
	uint32_t offset_in_struct;
};

struct shaders_metadata {
	uint32_t section_size;
	uint32_t n_shaders;
	struct {
		uint32_t shader_type;
		uint32_t data_size;
		uint32_t data_offset;
	} shader[];
};

struct attributes_metadata {
	uint32_t section_size;
	uint32_t n_attributes;
	struct {
		uint32_t name_offset;
		uint32_t bind_id;
	} attribute[];
};

struct uniforms_metadata {
	uint32_t section_size;
	uint32_t n_uniforms;
	struct {
		uint32_t struct_offset;
		uint32_t name_offset;
	};
};

bool myy_glpack_compile_program
(uint8_t const * __restrict const data,
 struct program_definition_header const * __restrict const mdata,
 uint8_t * __restrict const runtime_data)
{
	struct shaders_metadata const * __restrict shaders_mdata;
	struct attributes_metadata const * __restrict attributes_mdata;
	struct uniforms_metadata const * __restrict uniforms_mdata;

	uint8_t * __restrict temp_buffer;
	GLuint * __restrict const shaders_id;
	int_fast32_t n_shader_id = 0;

	uint8_t const * __restrict mdata_cursor =
		(uint8_t const * __restrict) mdata;
	GLuint * const linked_id = runtime_data->offset_in_struct;
	GLuint program;

	struct glh_status link_status;

	program = glCreateProgram();

	if (program == 0) {
		goto glcreateprogram_failed_miserably;
	}

	mdata_cursor += mdata->section_size;

	shaders_mdata =
		(struct shaders_metadata const * __restrict)
		mdata_cursor;

	shaders_id =
		(GLuint * __restrict)
		malloc(sizeof(GLuint) * shaders_mdata->n_shaders);

	if (shaders_id == NULL)
		goto could_not_alloc_shaders_id;

	for (n_shader_id = 0;
	     n_shader_id < shaders_mdata->n_shaders;
	     n_shader_id++)
	{
		
		struct glh_status status = glhCompileShader(
			program,
			shaders_mdata->shader[n_shader_id].shader_type,
			shaders_mdata->shader[n_shader_id].data_size,
			(data + shaders_mdata->shader[n_shader_id].data_offset)
		);
		
		if (status.ok) 
			shaders_id[n_shader_id] = status.id;
		else {
			goto one_shader_failed_to_compile;
		}
	}

	mdata_cursor += shaders_mdata->section_size;
	attributes_mdata =
		(struct attributes_metadata const * __restrict)
		mdata_cursor;

	for (uint32_t i = 0; i < attributes_mdata->n_attributes; i++) {
		glBindAttribLocation(
			program,
			(data + attributes_mdata->attribute[i].name_offset),
			attributes_mdata->attribute[i].bind_id
		);
	}

	link_status = glhLinkProgram(program);
	if (!link_status.ok) {
		glDeleteProgram(program);
		for (int32_t j = n_shader_id - 1; j >= 0; j--)
			glDeleteShader(shaders_id[j]);

		free(shaders_id);
		goto could_not_link_program;
	}

	/* Now that everything went fine : 
	 * delete previously set program id.
	 * 
	 * No need to worry about the previously written uniforms ids.
	 * We're going to overwrite them a few lines below.
	 * Every "GetUniformLocation" miss will just generate
	 * -1 afterwards. Using -1 with "Uniform*" just generate
	 * no result.
	 */
	if (*linked_id)
		glDeleteProgram(*linked_id);

	*linked_id = program;

	mdata_cursor += attributes_mdata->section_size;
	uniforms_mdata =
		(struct uniforms_metadata const * __restrict)
		mdata_cursor;

	for (uint32_t i = 0; i < uniforms_mdata->n_uniforms; i++) {
		GLuint * __restrict const uniform_id =
			(GLuint * __restrict)
			(runtime_data+uniforms_mdata->offset_in_struct);
		*uniform_id = glGetUniformLocation(
			program,
			(data + uniforms_mdata->uniforms[i].name_pos));
	}

	return true;

could_not_link_program:
one_shader_failed_to_compile:
	for (int32_t j = n_shader_id - 1; j >= 0; j--)
		glDeleteShader(shaders_id[j]);
	free(shaders_id);
could_not_alloc_shaders_id:
	glDeleteProgram(program);
glcreateprogram_failed_miserably:
	return false;
}
