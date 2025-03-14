#version 440 core

layout(location = 0) out vec4 oColor;

in vec2 tTexCoord;
flat in uint tTexSlot;

layout(binding = 0) uniform sampler2D uTextures[32]; // TODO template

void main() {
	oColor = texture(uTextures[tTexSlot], tTexCoord);
}
