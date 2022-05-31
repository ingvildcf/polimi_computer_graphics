#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 worldMat;
	mat4 viewMat;
	mat4 prjMat;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec4 fragTan;
layout(location = 3) out vec2 fragTexCoord;

void main() {
	gl_Position = ubo.worldMat * vec4(inPosition, 1.0);
	fragPos = (ubo.viewMat * vec4(inPosition, 1.0)).xyz;
	fragNorm = mat3(ubo.prjMat) * inNormal;
	fragTan = vec4(mat3(ubo.prjMat) * inTangent.xyz, inTangent.w);
	fragTexCoord = inTexCoord;
}	

