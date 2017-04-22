#version 310 es

precision highp float;

layout(location = 0) in vec4 xyz;
layout(location = 1) in vec2 in_st;

out vec2 st;

layout(location = 1) uniform mat4 projection;
layout(location = 2) uniform vec4 pixel_offset;

void main() {
	vec4 px_position = xyz + pixel_offset;
	gl_Position = projection * px_position;
	st = in_st;
}
