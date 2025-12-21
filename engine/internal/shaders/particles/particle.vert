#version 450 core

uniform mat3 uProjection;
uniform mat3 uTransform;
uniform uint uReverseDrawOrder;

struct Particle {
	float timeElapsed;
	float lifetime;
	uint attached;
	mat3 localTransform;
	vec4 color;
	vec2 velocity;
};

layout(std430, binding = 0) readonly buffer ParticlesOut {
    Particle particlesOut[];
};

struct DrawArraysIndirectCommand {
	uint count;
	uint primCount;
	uint first;
	uint baseVertex;
};

layout(std430, binding = 1) buffer DrawCommand {
	DrawArraysIndirectCommand cmd;
};

struct ParticleSystemData {
	uint maxTimeElapsedBits;
};

layout(std430, binding = 2) buffer PSData {
	ParticleSystemData particleSystemData;
};

const vec2 quad[4] = vec2[](
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5),
    vec2(-0.5,  0.5),
    vec2( 0.5,  0.5)
);

out vec4 tColor;

void main() {
	Particle p = particlesOut[gl_InstanceID];

	mat3 transform = uProjection;
	if (p.attached == uint(1))
		transform *= uTransform;

	gl_Position.xy = (transform * p.localTransform * vec3(quad[gl_VertexID], 1.0)).xy;
	gl_Position.z = p.timeElapsed / uintBitsToFloat(particleSystemData.maxTimeElapsedBits);
	if (uReverseDrawOrder == uint(1))
		gl_Position.z = -gl_Position.z;

	tColor = p.color;
}
