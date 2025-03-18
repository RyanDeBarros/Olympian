#version 450 core

layout(location = 0) in vec2 iPosition;
layout(location = 1) in vec4 iColor;
layout(location = 2) in mat3 iTransform; // TODO use SSBO and index the transform

uniform mat3 uProjection;

out vec4 tColor;

void main() {
	gl_Position.xy = (uProjection * iTransform * vec3(iPosition, 1.0)).xy;
	tColor = iColor;
}
