#version 440 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in mat4 iTransform;
layout(location = 5) in vec4 iColor;

uniform mat4 uProjection;

out vec4 tColor;

void main() {
	gl_Position = uProjection * iTransform * vec4(iPosition, 1.0);
	tColor = iColor;
}
