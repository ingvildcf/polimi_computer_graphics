#version 450#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform GlobalUniformBufferObject {
	vec3 lightDir;
	vec4 lightColor;
	vec3 eyePos;
} gubo;

void main() {
	vec3 X = dFdx(fragPos);
	vec3 Y = dFdy(fragPos);
	vec3 Norm = normalize(cross(X,Y));

	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	Norm = dot(Norm, gubo.lightDir) > 0 ? Norm : -Norm;

	vec3 Diffuse = vec3(1.0f,0.3f,0.3f) * (dot(Norm, gubo.lightDir) * 0.6f + 0.4f);
	vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(gubo.lightDir, Norm)),0.0f), 64.0f));
	
	outColor = vec4((Diffuse + Specular) * gubo.lightColor.rgb, 1.0f);

}