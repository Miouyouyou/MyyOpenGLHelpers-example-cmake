precision highp float;

attribute vec2 xy;
attribute float z;

varying vec4 color;

uniform mat4 projection;
uniform vec4 pixel_offset;
uniform float steps;

vec4 hueToRGB(float h)
{
	vec4 rgb;
	h = fract(h) * 6.0;
	rgb.r = clamp(abs(3.0 - h)-1.0, 0.0, 1.0);
	rgb.g = clamp(2.0 - abs(2.0 - h), 0.0, 1.0);
	rgb.b = clamp(2.0 - abs(4.0 - h), 0.0, 1.0);
	rgb.a = 1.0;
	return rgb;
}

vec4 heat(float x)
{
	return hueToRGB(2.0/3.0-(2.0/3.0)*clamp(x,0.0,1.0));
}

void main() {
	vec4 px_position = vec4(xy, z, 1.0);
	gl_Position = projection * px_position;
	float discrete = floor(z / steps); //0.5 to round
	color = heat(discrete);
}
