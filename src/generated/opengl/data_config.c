#include <src/generated/opengl/data_config.h>
#include <myy/helpers/file.h>

void glhShadersPackLoader
(struct glsl_programs_shared_data * __restrict const data)
{
	fh_WholeFileToBuffer("data/shaders.pack", data);
}
