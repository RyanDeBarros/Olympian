#version 450 core
#extension GL_ARB_bindless_texture : require

uniform vec4 uGlobalModulation;

layout(location = 0) out vec4 oColor;

in vec2 tTexCoord;
flat in uint tTexSlot;
flat in vec4 tModulation;
flat in uint tFramePlusOne;
flat in uint tIsTextGlyph;

struct TexData
{
	uvec2 handle;
	vec2 dimensions;
};
layout(std430, binding = 0) readonly buffer TextureData {
	TexData uTexData[];
};

void main() {
	if (tIsTextGlyph == 0) {
		if (tFramePlusOne == 0)
			oColor = uGlobalModulation * tModulation * texture(sampler2D(uTexData[tTexSlot].handle), tTexCoord);
		else
			oColor = uGlobalModulation * tModulation * texture(sampler2DArray(uTexData[tTexSlot].handle), vec3(tTexCoord.x, tTexCoord.y, tFramePlusOne - 1));
	} else {
		float alpha = texture(sampler2D(uTexData[tTexSlot].handle), tTexCoord).r;
		oColor = uGlobalModulation * mix(vec4(0.0), tModulation, alpha);
	}
}
