precision highp float;

attribute vec3 xyz;
attribute vec2 in_st;

uniform mat4 projection;

varying vec2 st;

void main() {
	gl_Position = projection * vec4(xyz, 1.0);
	st = in_st;
}

