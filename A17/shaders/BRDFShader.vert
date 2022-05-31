#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
} ubo;

/**** Modify from here *****/
vec3 inPosition;
vec2 inTexCoord;
vec3 inNormal;

vec3 fragPos;
vec3 fragNorm;
vec2 fragTexCoord;

/**** To here *****/

void main() {
	gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);
	fragPos = (ubo.mMat * vec4(inPosition, 1.0)).xyz;
	fragNorm = mat3(ubo.nMat) * inNormal;
	fragTexCoord = inTexCoord;
}