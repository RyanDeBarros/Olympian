#version 450 core
#extension GL_ARB_bindless_texture : require

layout(location = 0) out vec4 oColor;

in vec2 tTexCoord;
flat in uint tTexSlot;

struct TexData
{
	uvec2 handle;
	vec2 dimensions;
};
layout(std430, binding = 0) readonly buffer TextureData {
	TexData uTexData[];
};

void main() {
	oColor = texture(sampler2D(uTexData[tTexSlot].handle), tTexCoord);
}
