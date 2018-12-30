precision highp float;

uniform sampler2D fonts_texture;

varying vec2 st;

void main() {
	gl_FragColor = 
		vec4(0.0,0.0,0.0, texture2D(fonts_texture, st).a);
}
