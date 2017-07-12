precision highp float;

uniform sampler2D sampler;

varying vec2 st;

void main() {
	gl_FragColor = vec4(1.0,1.0,1.0, texture2D(sampler, st).a);
}
