#version 450 core

uniform mat3 uProjection;
uniform mat3 uTransform;

layout(location = 0) in vec2 iPosition;
layout(location = 1) in vec2 iRs;

struct Mat3
{
	float m00, m01, m02, m10, m11, m12, m20, m21, m22;
};
mat3 matrix(Mat3 m) {
	return mat3(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
}
layout(std430, binding = 0) readonly buffer ParticleTransforms {
	Mat3 uTransforms[];
};

layout(std430, binding = 1) readonly buffer ParticleColor {
	vec4 uColors[];
};

flat out vec4 tColor;
out vec2 tLocalPos;
flat out float tRx;
flat out float tRy;

void main() {
	gl_Position.xy = (uProjection * uTransform * matrix(uTransforms[gl_InstanceID]) * vec3(iPosition, 1.0)).xy;
	tColor = uColors[gl_InstanceID];
	tLocalPos = iPosition;
	tRx = iRs.x;
	tRy = iRs.y;
}
