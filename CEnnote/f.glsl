#version 330 core

uniform int id;
uniform float aspectRatio;
uniform vec3 frameColor;

in vec2 oPos;

out vec4 fragColor;

void main() {
	vec4 color = vec4(1.0, 0.0, 0.0, 1.0);

	if (id == 0) {
		color = vec4(frameColor, 1.0);
		if (abs(oPos.x) >= 0.95 || abs(oPos.y) >= 0.95 * aspectRatio) {
			color *= 0.8;
		}
		
	}
	if (id == 1) {
		color = vec4(0.12, 0.123, 0.121, 1.0);
	}

	fragColor = color;
}
