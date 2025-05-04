#version 450 core

layout(location = 0) out vec4 oColor;

flat in vec4 tColor;

void main() {
	oColor = tColor;
}
