#version 450 core

uniform mat3 uProjection;

struct TexData
{
	uvec2 handle;
	vec2 dimensions;
};
layout(std430, binding = 0) readonly buffer TextureData {
	TexData uTexData[];
};

struct QuadTexInfo
{
	uint texSlot;
	uint texCoordSlot;
};
layout(std430, binding = 1) readonly buffer QuadTextures {
	QuadTexInfo uQuadTextures[];
};
struct Mat3
{
	float m00, m01, m02, m10, m11, m12, m20, m21, m22;
};
mat3 matrix(Mat3 m) {
	return mat3(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
}
layout(std430, binding = 2) readonly buffer Transforms {
	Mat3 uTransforms[];
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
	switch (gl_VertexID % 4) {
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
	switch (gl_VertexID % 4) {
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
	QuadTexInfo quad_texture = uQuadTextures[gl_VertexID / 4];
	if (quad_texture.texSlot > 0) {
		gl_Position.xyz = uProjection * matrix(uTransforms[gl_VertexID / 4]) * vec3(position(uTexData[quad_texture.texSlot].dimensions), 1.0);
		tTexCoord = coords(uTexCoords[quad_texture.texCoordSlot]);
		tTexSlot = quad_texture.texSlot;
	}
	else {
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // degenerate outside NDC
	}
}
