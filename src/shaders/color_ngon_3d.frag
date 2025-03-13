#version 440 core

layout(location = 0) out vec4 oColor;

in vec4 tColor;

void main() {
	oColor[0] = pow(tColor[0], 3);
	oColor[1] = pow(tColor[1], 3);
	oColor[2] = pow(tColor[2], 3);
	oColor[3] = tColor[3];
}
