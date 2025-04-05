#version 450 core

uniform mat3 uProjection;

vec2 position(vec2 size, int corner) {
	switch (corner) {
	case 0:
		return vec2(-0.5 * size.x, -0.5 * size.y);
	case 1:
		return vec2( 0.5 * size.x, -0.5 * size.y);
	case 2:
		return vec2( 0.5 * size.x,  0.5 * size.y);
	case 3:
		return vec2(-0.5 * size.x,  0.5 * size.y);
	}
}
layout(std430, binding = 0) readonly buffer EllipseSize {
	vec2 uSizes[];
};

layout(std430, binding = 1) readonly buffer EllipseColor {
	vec4 uColors[];
};

struct Mat3
{
	float m00, m01, m02, m10, m11, m12, m20, m21, m22;
};
mat3 matrix(Mat3 m) {
	return mat3(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
}
layout(std430, binding = 2) readonly buffer EllipseTransforms {
	Mat3 uTransforms[];
};

out vec4 tColor;
flat out vec2 tCenter;
out vec2 tWorldPos;
flat out vec2 tSize;

void main() {
	mat3 tr = matrix(uTransforms[gl_VertexID / 4]);
	tWorldPos = (tr * vec3(position(uSizes[gl_VertexID / 4], gl_VertexID % 4), 1.0)).xy;
	gl_Position.xy = (uProjection * vec3(tWorldPos, 1.0)).xy;
	tColor = uColors[gl_VertexID / 4];
	tCenter = (tr * vec3(0.0, 0.0, 1.0)).xy;
	//tSize = uSizes[gl_VertexID / 4] * vec2(tr[2][0], tr[2][1]);
	tSize = (tr * vec3(position(uSizes[gl_VertexID / 4], 2), 1.0)).xy;
}
