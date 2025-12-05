#version 450 core
#extension GL_NV_gpu_shader5 : enable

layout(location = 0) in vec2 iPosition;
layout(location = 1) in vec4 iColor;
layout(location = 2) in uint iIndex;

uniform mat3 uProjection;
uniform mat3 uInvariantProjection;

struct MatInfo
{
	float m00, m01, m02, m10, m11, m12, m20, m21, m22;
	uint8_t cameraInvariant;
};

mat3 matrix(MatInfo m) {
	return mat3(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
}

layout(std430, binding = 0) readonly buffer PolygonTransforms {
	MatInfo uTransforms[];
};

out vec4 tColor;

void main() {
	mat3 projection = uTransforms[iIndex].cameraInvariant == uint8_t(0) ? uProjection : uInvariantProjection;
	gl_Position.xy = (projection * matrix(uTransforms[iIndex]) * vec3(iPosition, 1.0)).xy;
	tColor = iColor;
}
