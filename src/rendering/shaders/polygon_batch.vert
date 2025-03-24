#version 450 core

layout(location = 0) in vec2 iPosition;
layout(location = 1) in vec4 iColor;

uniform mat3 uProjection;
uniform uint uDegree;

struct Mat3
{
	float m00, m01, m02, m10, m11, m12, m20, m21, m22;
};
mat3 matrix(Mat3 m) {
	return mat3(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
}
layout(std430, binding = 0) readonly buffer Transforms {
	Mat3 uTransforms[];
};

out vec4 tColor;

void main() {
	// TODO use compact EBO to avoid unnecessary drawing
	gl_Position.xy = (uProjection * matrix(uTransforms[gl_VertexID / uDegree]) * vec3(iPosition, 1.0)).xy;
	tColor = iColor;
}
