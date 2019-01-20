precision highp float;

/* location = 5 */ attribute vec2 xy;
/* location = 6 */ attribute vec2 in_st;

uniform mat4 projection;
uniform mat4 tex_projection;

varying vec2 st;

void main() {
	gl_Position = projection * vec4(xy, 32.0, 1.0);
	st = clamp(in_st, vec2(21.125, 38.875), vec2(1923.125, 1938.875))  * vec2(1.0/64.0, 1.0/2048.0);
}
 
