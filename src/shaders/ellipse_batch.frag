#version 450 core

layout(location = 0) out vec4 oColor;

struct Dimension {
	float rx;
	float ry;
	float b;
	float fill_exp;
	float border_exp;
};
struct ColorGradient {
	vec4 fill_inner;
	vec4 fill_outer;
	vec4 border_inner;
	vec4 border_outer;
};

flat in ColorGradient tColor;
in vec2 tLocalPos;
flat in Dimension tDimension;

void main() {
	float diff = 1 - sqrt(pow(tLocalPos.x / tDimension.rx, 2) + pow(tLocalPos.y / tDimension.ry, 2));
	if (diff < 0.0)
		discard;
	if (diff >= tDimension.b) {
		oColor = mix(tColor.fill_inner, tColor.fill_outer, pow(clamp((1.0 - diff + tDimension.b) / (1.0 - tDimension.b), 0.0, 1.0), tDimension.fill_exp));	
	} else {
		oColor = mix(tColor.border_inner, tColor.border_outer, pow(clamp((tDimension.b - diff) / tDimension.b, 0.0, 1.0), tDimension.border_exp));
	}
}
