precision highp float;

uniform sampler2D sampler;

varying vec4 color;

void main() {
	gl_FragColor = color;  
}
