precision highp float;

attribute vec4 xy;

uniform mat4 projection;

void main() {
	gl_Position = projection * xy;
}

