#version 450 core

uniform mat3 uProjection;

struct Dimension {
	float rx;
	float ry;
	float b;
	float fill_exp;
	float border_exp;
};
vec2 position(Dimension dim, int corner) {
	switch (corner) {
	case 0:
		return vec2(-dim.rx, -dim.ry);
	case 1:
		return vec2( dim.rx, -dim.ry);
	case 2:
		return vec2( dim.rx,  dim.ry);
	case 3:
		return vec2(-dim.rx,  dim.ry);
	}
}
layout(std430, binding = 0) readonly buffer EllipseDimension {
	Dimension uDimensions[];
};

struct ColorGradient {
	vec4 fill_inner;
	vec4 fill_outer;
	vec4 border_inner;
	vec4 border_outer;
};
layout(std430, binding = 1) readonly buffer EllipseColor {
	ColorGradient uColors[];
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

flat out ColorGradient tColor;
out vec2 tLocalPos;
flat out Dimension tDimension;

void main() {
	tDimension = uDimensions[gl_VertexID / 4];
	tLocalPos = position(tDimension, gl_VertexID % 4);
	gl_Position.xy = (uProjection * matrix(uTransforms[gl_VertexID / 4]) * vec3(tLocalPos, 1.0)).xy;
	tColor = uColors[gl_VertexID / 4];
}
