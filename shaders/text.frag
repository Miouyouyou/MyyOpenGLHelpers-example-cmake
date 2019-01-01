precision highp float;

uniform sampler2D fonts_texture;

varying vec2 st;

void main() {
	gl_FragColor = 
		vec4(vec3(texture2D(fonts_texture, st).a), 1.0);
}

