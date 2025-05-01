#version 450 core
#extension GL_ARB_bindless_texture : require

uniform vec4 uGlobalModulation;

layout(location = 0) out vec4 oColor;

in vec2 tTexCoord;
flat in uint tTexSlot;
flat in vec4 tTextColor;
in vec4 tModulation;

layout(std430, binding = 0) readonly buffer TextureHandles {
	uvec2 uTextureHandles[];
};

void main() {
	float alpha = texture(sampler2D(uTextureHandles[tTexSlot]), tTexCoord).r;
	oColor = uGlobalModulation * tModulation * mix(vec4(0.0), tTextColor, alpha);
}
