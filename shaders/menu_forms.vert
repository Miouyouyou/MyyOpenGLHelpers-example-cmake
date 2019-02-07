precision highp float;

attribute vec4 xyz;
attribute vec4 in_color;

varying vec4 color;

uniform mat4 projection;

/* Translate all the elements to these coordinates. */
uniform vec4 global_offset;

void main() {
	gl_Position = projection * (xyz + global_offset);
	color = in_color;
}

