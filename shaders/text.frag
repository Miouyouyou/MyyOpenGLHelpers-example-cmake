precision highp float;

uniform sampler2D fonts_texture;

uniform float niouik;

varying vec2 st;

void main() {
	gl_FragColor = texture2D(fonts_texture, st);
}

