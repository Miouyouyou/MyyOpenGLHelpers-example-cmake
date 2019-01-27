precision highp float;

attribute vec3 xy;
attribute vec2 in_st;

uniform mat4 projection;
uniform vec2 texture_projection;
uniform vec3 text_offset;
uniform vec3 global_offset;

varying vec2 st;

void main() {
	gl_Position = projection * vec4(xy+text_offset+global_offset, 1.0);
	st = in_st * texture_projection;
}

