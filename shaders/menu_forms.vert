precision highp float;

attribute vec4 xyz;
attribute vec4 in_color;

varying vec4 color;

uniform mat4 projection;
uniform vec4 pos_offset;

void main() {
	gl_Position = projection * (xyz + pos_offset);
	color = in_color;
}

