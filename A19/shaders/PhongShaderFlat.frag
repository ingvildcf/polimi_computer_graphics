#version 450#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) flat in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightDir;
	vec4 lightColor;
	vec3 eyePos;
	vec4 extras;
} gubo;

void main() {
	vec3 Norm = normalize(fragNorm);

	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	vec3 NormP = dot(Norm, gubo.lightDir) > 0 ? Norm : -Norm;

	vec3 Diffuse = texture(texSampler, fragUV).rgb * (dot(NormP, gubo.lightDir) * 0.6f + 0.4f);
	vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(gubo.lightDir, NormP)),0.0f), 64.0f));
	
	outColor = vec4((Diffuse + Specular) * gubo.lightColor.rgb, 1.0f) * gubo.extras.x +
			   vec4((Norm + vec3(1,1,1)) / 2.0f, 1.0) * gubo.extras.y;

}