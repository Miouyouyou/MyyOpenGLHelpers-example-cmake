precision highp float;

attribute vec2 xy;
attribute vec2 in_st;

uniform mat4 projection;
uniform vec2 texture_projection;

varying vec2 st;

void main() {
	gl_Position = projection * vec4(xy, 0.9, 1.0);
	st = in_st * texture_projection;
}

