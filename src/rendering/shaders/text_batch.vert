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
	uint foregroundColorSlot;
	uint backgroundColorSlot;
	uint modulationColorSlot;
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

layout(std140, binding = 0) uniform ForegroundColors {
	vec4 uForegroundColors[1000]; // guaranteed 16KB / 16B = #1000
};

layout(std140, binding = 1) uniform BackgroundColors {
	vec4 uBackgroundColors[1000]; // guaranteed 16KB / 64B = #1000
};

struct Modulation
{
	vec4 colors[4];
};
layout(std140, binding = 2) uniform Modulations {
	Modulation uModulation[250]; // guaranteed 16KB / 64B = #250
};

out vec2 tTexCoord;
flat out uint tTexSlot;
flat out vec4 tForegroundColor;
flat out vec4 tBackgroundColor;
out vec4 tModulation;

void main() {
	GlyphInfo glyph = uGlyphInfo[gl_VertexID / 4];
	if (glyph.texSlot > 0) {
		gl_Position.xy = (uProjection * matrix(uTransforms[gl_VertexID / 4]) * vec3(iPosition, 1.0)).xy;
		tTexCoord = iTexCoord;
		tTexSlot = glyph.texSlot;
		tForegroundColor = uForegroundColors[glyph.foregroundColorSlot];
		tBackgroundColor = uBackgroundColors[glyph.backgroundColorSlot];
		tModulation = uModulation[glyph.modulationColorSlot].colors[gl_VertexID % 4];
	}
	else {
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // degenerate outside NDC
	}
}
