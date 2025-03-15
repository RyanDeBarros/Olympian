#version 450 core

layout(location = 0) in uint iTexSlot;
layout(location = 1) in uint iTexCoordSlot;
layout(location = 2) in mat3 iTransform;

uniform mat3 uProjection;

struct TexData
{
	uvec2 handle;
	vec2 dimensions;
};
layout(std430, binding = 0) buffer TextureData {
	TexData uTexData[];
};

layout(std430, binding = 1) buffer TextureCoords {
	vec2 uTexCoords[];
};

out vec2 tTexCoord;
flat out uint tTexSlot;

vec2 position(vec2 dimensions) {
	switch (gl_VertexID) {
	case 0:
		return vec2(-dimensions[0] / 2, -dimensions[1] / 2);
	case 1:
		return vec2(dimensions[0] / 2, -dimensions[1] / 2);
	case 2:
		return vec2(dimensions[0] / 2, dimensions[1] / 2);
	case 3:
		return vec2(-dimensions[0] / 2, dimensions[1] / 2);
	}
}

void main() {
	gl_Position.xyz = uProjection * iTransform * vec3(position(uTexData[iTexSlot].dimensions), 1.0);
	tTexCoord = uTexCoords[4 * iTexCoordSlot + gl_VertexID];
	tTexSlot = iTexSlot;
}
