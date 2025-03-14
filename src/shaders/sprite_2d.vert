#version 440 core

layout(location = 0) in vec2 iPosition;
layout(location = 1) in mat3 iTransform;
layout(location = 4) in vec2 iTexCoord;
layout(location = 5) in uint iTexSlot;

uniform mat3 uProjection;

out vec2 tTexCoord;
flat out uint tTexSlot;

void main() {
	gl_Position.xyz = uProjection * iTransform * vec3(iPosition, 1.0);
	tTexCoord = iTexCoord;
	tTexSlot = iTexSlot;
}
