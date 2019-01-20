precision highp float;

uniform sampler2D tex;

varying vec2 st;

void main() {
	gl_FragColor = vec4(vec3(texture2D(tex, st).a), 1.0);
}
