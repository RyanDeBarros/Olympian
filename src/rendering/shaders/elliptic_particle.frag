#version 450 core

layout(location = 0) out vec4 oColor;

flat in vec4 tColor;
in vec2 tLocalPos;
flat in float tRx;
flat in float tRy;

void main() {
	if (pow(tLocalPos.x / tRx, 2) + pow(tLocalPos.y / tRy, 2) > 1.0)
		discard;
	oColor = tColor;
}
