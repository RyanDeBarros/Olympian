#version 450 core
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_bindless_texture : require

layout(location = 0) out vec4 oColor;

in vec2 tTexCoord;
flat in uint16_t tTexSlot;
flat in vec4 tModulation;
flat in uint16_t tFramePlusOne;
flat in uint16_t tIsTextGlyph;
in vec2 tModTexCoord;
flat in uint16_t tModTexSlot;

struct TexData
{
	uvec2 handle;
	vec2 dimensions;
};
layout(std430, binding = 0) readonly buffer TextureData {
	TexData uTexData[];
};

vec4 sampleModTex() {
	return tModTexSlot > uint16_t(0) ? texture(sampler2D(uTexData[tModTexSlot].handle), tModTexCoord) : vec4(1.0);
}

void main() {
	vec4 baseColor;
	if (tIsTextGlyph == uint16_t(0)) {
		if (tFramePlusOne == uint16_t(0))
			baseColor = texture(sampler2D(uTexData[tTexSlot].handle), tTexCoord);
		else
			baseColor = texture(sampler2DArray(uTexData[tTexSlot].handle), vec3(tTexCoord.x, tTexCoord.y, tFramePlusOne - uint16_t(1)));
	} else {
		float alpha = texture(sampler2D(uTexData[tTexSlot].handle), tTexCoord).r;
		baseColor = mix(vec4(0.0), vec4(1.0), alpha);
	}
	oColor = tModulation * sampleModTex() * baseColor;
}
