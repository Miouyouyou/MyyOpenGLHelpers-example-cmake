enum myy_shader_standard_attribs {
	standard_xy = 5,
	standard_in_st = 6,
};
enum myy_shader_super_attribs {
	super_xy = 5,
	super_in_st = 6,
};
enum myy_shader_heatmap_attribs {
	heatmap_xy = 0,
	heatmap_z = 1,
};

struct {
	struct {
		GLuint id;
		struct {
			GLint fonts_texture;
		} uniform;
	} standard;
	struct {
		GLuint id;
		struct {
			GLint fonts_texture;
		} uniform;
	} super;
	struct {
		GLuint id;
		struct {
			GLint projection;
			GLint pixel_offset;
			GLint steps;
			GLint sampler;
		} uniform;
	} heatmap;

} __attribute__((packed, aligned(8))) myy_programs;
