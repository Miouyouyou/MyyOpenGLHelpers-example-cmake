precision highp float;

attribute vec4 xyz;
attribute vec2 in_st;

varying vec2 st;

uniform mat4 projection;
uniform vec4 pixel_offset;

void main() {
	vec4 px_position = xyz + pixel_offset;
	gl_Position = projection * px_position;
	st = in_st;
}
