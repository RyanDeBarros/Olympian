#version 450 core

layout(location = 0) in vec2 iPosition;
layout(location = 1) in vec2 iTexCoord;

uniform mat3 uProjection;

layout(std430, binding = 0) readonly buffer TextureHandles {
	uvec2 uTextureHandles[];
};

struct GlyphInfo
{
	uint texSlot;
	uint modulationSlot;
};
layout(std430, binding = 1) readonly buffer GlyphInfos {
	GlyphInfo uGlyphInfo[];
};

struct Mat3
{
	float m00, m01, m02, m10, m11, m12, m20, m21, m22;
};
mat3 matrix(Mat3 m) {
	return mat3(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
}
layout(std430, binding = 2) readonly buffer QuadTransforms {
	Mat3 uTransforms[];
};

layout(std140, binding = 0) uniform Modulations {
	vec4 uModulation[1000]; // guaranteed 16KB / 16B = #1000
};

out vec2 tTexCoord;
flat out uint tTexSlot;
flat out vec4 tModulation;

void main() {
	GlyphInfo glyph = uGlyphInfo[gl_VertexID / 4];
	if (glyph.texSlot > 0) {
		gl_Position.xy = (uProjection * matrix(uTransforms[gl_VertexID / 4]) * vec3(iPosition, 1.0)).xy;
		tTexCoord = iTexCoord;
		tTexSlot = glyph.texSlot;
		tModulation = uModulation[glyph.modulationSlot];
	}
	else {
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // degenerate outside NDC
	}
}
