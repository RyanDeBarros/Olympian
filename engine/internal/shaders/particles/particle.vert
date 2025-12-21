#version 450 core

uniform mat3 uProjection;
uniform mat3 uTransform;

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

	tColor = p.color;
}
