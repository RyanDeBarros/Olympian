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

struct TexUVRect
{
	vec4 uvs[2];
};
layout(std140, binding = 0) uniform TextureCoords {
	TexUVRect uTexCoords[500]; // guaranteed 16KB / 32B
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

vec2 coords(TexUVRect rect) {
	switch (gl_VertexID) {
	case 0:
		return rect.uvs[0].xy;
	case 1:
		return rect.uvs[0].zw;
	case 2:
		return rect.uvs[1].xy;
	case 3:
		return rect.uvs[1].zw;
	}
}

void main() {
	gl_Position.xyz = uProjection * iTransform * vec3(position(uTexData[iTexSlot].dimensions), 1.0);
	tTexCoord = coords(uTexCoords[4 * iTexCoordSlot]);
	tTexSlot = iTexSlot;
}
