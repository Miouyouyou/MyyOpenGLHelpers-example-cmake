precision highp float;

/* location = 5 */ attribute vec2 xy;
/* location = 6 */ attribute vec2 in_st;

varying vec2 st;

void main() {
	gl_Position = vec4(xy, 0.9, 1.0);
	st = in_st;
}
