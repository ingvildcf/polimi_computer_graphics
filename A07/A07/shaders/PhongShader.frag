#version 450#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightDir;
	vec4 lightColor;
	vec3 eyePos;
} gubo;

void main() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);

	vec3 Diffuse = texture(texSampler, fragTexCoord).rgb * (max(dot(Norm, gubo.lightDir),0.0f) * 0.9f + 0.1f);
	vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(gubo.lightDir, Norm)),0.0f), 64.0f));
	
	outColor = vec4((Diffuse + Specular) * gubo.lightColor.rgb, 1.0f);
	
//	outColor = vec4(texture(texSampler, fragTexCoord).rgb * gubo.lightColor.rgb * (max(dot(Norm, gubo.lightDir),0.0f) * 0.9f + 0.1f), 1.0f);
//	outColor = vec4(texture(texSampler, fragTexCoord).rgb * (max(dot(Norm, gubo.lightDir),0.0f) * 0.9f + 0.1f), 1.0f);
//	outColor = vec4(vec3(max(dot(Norm, gubo.lightDir),0.0f) * 0.85f + 0.15f), 1.0f);
//	outColor = vec4((Norm + 1.0f) / 2.0f, 1.0f);
}