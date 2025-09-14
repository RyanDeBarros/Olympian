#version 450 core

uniform mat3 uProjection;
uniform float uTime;

struct TexData
{
	uvec2 handle;
	vec2 dimensions;
};
layout(std430, binding = 0) readonly buffer TextureData {
	TexData uTexData[];
};

// TODO v4 can implement clever compression techniques to reduce size of info when adding more properties.

struct QuadInfo
{
	uint texSlot;
	uint texCoordSlot;
	uint colorSlot;
	uint frameSlot;
	uint isTextGlyph;
};
layout(std430, binding = 1) readonly buffer QuadInfos {
	QuadInfo uQuadInfo[];
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

layout(std430, binding = 3) readonly buffer TextureCoords {
	vec4 uTexCoords[];
};

// TODO v4 const size allocation for uniform buffers could be overkill for framebuffer draws. Make it a template variable.
// TODO v4 utilize optional modulation textures in addition to solid color modulation.

layout(std140, binding = 1) uniform Modulations {
	vec4 uModulation[1000]; // guaranteed 16KB / 16B = #1000
};

struct AnimFrameFormat
{
	uint starting_frame;
	uint num_frames;
	float starting_time;
	float delay_seconds;
};
layout(std140, binding = 2) uniform Anims {
	AnimFrameFormat uAnims[1000]; // guaranteed 16KB / 16B = #1000
};

vec2 calc_position(vec2 dimensions) {
	return dimensions * (vec2(((gl_VertexID + 1) >> 1) & 1, (gl_VertexID >> 1) & 1) - 0.5);
}

vec2 calc_tex_coords(vec4 uvs) {
	return vec2(uvs[((gl_VertexID + 1) >> 1) & 1], uvs[2 + ((gl_VertexID >> 1) & 1)]);
}

uint calc_frame(AnimFrameFormat anim) {
	int frame_offset = anim.delay_seconds == 0.0 ? 0 : int(floor((uTime - anim.starting_time) / anim.delay_seconds));
	return (anim.starting_frame + frame_offset) % anim.num_frames;
}

out vec2 tTexCoord;
flat out uint tTexSlot;
flat out vec4 tModulation;
flat out uint tFramePlusOne;
flat out uint tIsTextGlyph;

void main() {
	QuadInfo quad = uQuadInfo[gl_VertexID >> 2];
	if (quad.texSlot > 0) { // TODO v4 don't use 1-indexed buffers
		gl_Position.xy = (uProjection * matrix(uTransforms[gl_VertexID >> 2]) * vec3(calc_position(uTexData[quad.texSlot].dimensions), 1.0)).xy;
		tTexCoord = calc_tex_coords(uTexCoords[quad.texCoordSlot]);
		tTexSlot = quad.texSlot;
		tModulation = uModulation[quad.colorSlot];
		tIsTextGlyph = quad.isTextGlyph;
		tFramePlusOne = quad.frameSlot > 0 ? 1 + calc_frame(uAnims[quad.frameSlot]) : 0;
	} else {
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // degenerate outside NDC
	}
}
