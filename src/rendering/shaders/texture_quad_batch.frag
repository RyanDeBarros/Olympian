#version 450 core
#extension GL_ARB_bindless_texture : require

uniform vec4 uGlobalModulation;

layout(location = 0) out vec4 oColor;

in vec2 tTexCoord;
flat in uint tTexSlot;
in vec4 tModulation;

struct TexData
{
	uvec2 handle;
	vec2 dimensions;
};
layout(std430, binding = 0) readonly buffer TextureData {
	TexData uTexData[];
};

void main() {
	oColor = uGlobalModulation * tModulation * texture(sampler2D(uTexData[tTexSlot].handle), tTexCoord);
}
