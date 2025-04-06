#version 450 core

uniform mat3 uProjection;
uniform uint uBufOffset;

layout(location = 0) in vec2 iPosition;

struct Mat3
{
	float m00, m01, m02, m10, m11, m12, m20, m21, m22;
};
mat3 matrix(Mat3 m) {
	return mat3(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
}
layout(std430, binding = 0) readonly buffer EllipseTransforms {
	Mat3 uTransforms[];
};

layout(std430, binding = 1) readonly buffer EllipseColor {
	vec4 uColors[];
};

out vec4 tColor;

void main() {
	uint instance = gl_InstanceID + uBufOffset;
	gl_Position.xy = (uProjection * matrix(uTransforms[instance]) * vec3(iPosition, 1.0)).xy;
	tColor = uColors[instance];
}
