#version 450 core
#extension GL_NV_gpu_shader5 : enable

uniform mat3 uProjection;
uniform vec4 uGlobalModulation;
uniform float uTime;

struct TexData
{
	uvec2 handle;
	vec2 dimensions;
};
layout(std430, binding = 0) readonly buffer TextureData {
	TexData uTexData[];
};

struct QuadInfo
{
	uint16_t texSlot;
	uint16_t texCoordSlot;
	uint16_t colorSlot;
	uint16_t frameSlot;
	uint16_t isTextGlyph;
	uint16_t modTexSlot;
	uint16_t modTexCoordSlot;
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

uint16_t calc_frame(AnimFrameFormat anim) {
	int frame_offset = anim.delay_seconds == 0.0 ? 0 : int(floor((uTime - anim.starting_time) / anim.delay_seconds));
	return uint16_t((anim.starting_frame + frame_offset) % anim.num_frames);
}

out vec2 tTexCoord;
flat out uint16_t tTexSlot;
flat out vec4 tModulation;
flat out uint16_t tFramePlusOne;
flat out uint16_t tIsTextGlyph;
out vec2 tModTexCoord;
flat out uint16_t tModTexSlot;

void main() {
	QuadInfo quad = uQuadInfo[gl_VertexID >> 2];
	if (quad.texSlot > uint16_t(0)) { // TODO v4 do quad.texSlot - 1 --> 0th element in buffer shouldn't be empty.
		gl_Position.xy = (uProjection * matrix(uTransforms[gl_VertexID >> 2]) * vec3(calc_position(uTexData[quad.texSlot].dimensions), 1.0)).xy;
		tTexCoord = calc_tex_coords(uTexCoords[quad.texCoordSlot]);
		tTexSlot = quad.texSlot;
		tModulation = uGlobalModulation * uModulation[quad.colorSlot];
		tFramePlusOne = quad.frameSlot > uint16_t(0) ? uint16_t(1) + calc_frame(uAnims[quad.frameSlot]) : uint16_t(0);
		tIsTextGlyph = quad.isTextGlyph;
		tModTexCoord = calc_tex_coords(uTexCoords[quad.modTexCoordSlot]);
		tModTexSlot = quad.modTexSlot;
	} else {
		gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // degenerate outside NDC
	}
}
