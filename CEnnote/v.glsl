#version 330 core

layout (location = 0) in vec2 aPos;

uniform mat4 p;
uniform vec2 off;

out vec2 oPos;

void main() {
	gl_Position = p * vec4(off + aPos, 0.0, 1.0);	
	oPos = aPos;
}
