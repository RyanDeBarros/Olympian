#version 450 core

uniform mat3 uTransform;

struct Particle {
	float timeElapsed;
	float lifetime;
	vec2 position;
	vec2 velocity;
	float rotation;
	vec2 size;
	vec4 color;
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

	mat2 rotation = mat2(vec2(cos(p.rotation), sin(p.rotation)), vec2(-sin(p.rotation), cos(p.rotation)));
	vec2 offset = quad[gl_VertexID] * p.size;
	vec2 localPosition = p.position + rotation * offset;
	gl_Position.xy = (uTransform * vec3(localPosition, 1.0)).xy;

	tColor = p.color;
}
