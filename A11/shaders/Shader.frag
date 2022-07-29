#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {
	float time;
} gubo;

// cosine based palette, 4 vec3 params
vec3 palette(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d){ // c = oscillations, d = phase, t in [0,1]
    return a + b * cos(6.28318 * (c*t + d));
}

void main() {
    
	vec3 a = vec3(0.5, 0.5, 0.5);
	vec3 b = vec3(0.5, 0.5, 0.5);
	vec3 c = vec3(1.0, 1.0, 1.0);
	vec3 d = vec3(0.30, 0.20, 0.20);

	vec3 palette = palette(gubo.time, a, b, c, d);

	outColor = vec4(palette, 1.0f);
}  

