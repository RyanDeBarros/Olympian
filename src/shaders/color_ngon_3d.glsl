//***VERT***

#version 440 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in mat4 iTransform;
layout(location = 5) in vec4 iColor;

uniform mat4 uProjection;

out vec4 tColor;

void main() {
	gl_Position = uProjection * iTransform * vec4(iPosition, 1.0);
	tColor = iColor;
}

//***FRAG***

#version 440 core

layout(location = 0) out vec4 oColor;

in vec4 tColor;

void main() {
	oColor[0] = pow(tColor[0], 3);
	oColor[1] = pow(tColor[1], 3);
	oColor[2] = pow(tColor[2], 3);
	oColor[3] = tColor[3];
}
