#version 450 core

layout(location = 0) out vec4 oColor;

in vec4 tColor;
flat in vec2 tCenter;
in vec2 tWorldPos;
flat in vec2 tSize;

void main() {
	vec2 displ = tWorldPos - tCenter;
	float square = pow(displ.x / tSize.x, 2) + pow(displ.y / tSize.y, 2);
	if (square > 1.0)
		discard;

	oColor = tColor;
}
