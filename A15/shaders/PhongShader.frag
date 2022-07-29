#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightDir; // Direction of light (spot, directional)
	vec3 lightPos; // Position of light (spot, point)
	vec3 lightColor; // Basic color of light
	vec3 eyePos;
	vec4 coneInOutDecayExp; // spot: .x = cos of outer angle, .y = cos of inner angle, .z = basic distance g (spot, point), .w = denominator beta (spot, point)
	vec4 selector;
} gubo;

/*
normalize()
pow()
dot()
length()
clamp()
max()
min(
*/

/* Direct light models - distant light source that uniformly influence the entire scene*/

// Directional light direction
vec3 direct_light_dir(vec3 pos) {
	vec3 d = gubo.lightDir; // Light direction
	return d;
}

// Directional light color
vec3 direct_light_color(vec3 pos) {
	vec3 l = gubo.lightColor; // Light color
	return l;
}

/* Point light models - sources that emit light from fixed point in space, have no direction*/

// Point light direction
vec3 point_light_dir(vec3 pos) {
	vec3 p = gubo.lightPos; // Light source position
	vec3 lx = normalize(p - pos); // Light direction
	return lx;
}

// Point light color
vec3 point_light_color(vec3 pos) {
	
	float g = gubo.coneInOutDecayExp.z; // Basic distance - light reduction = 1
	float beta = gubo.coneInOutDecayExp.w; // Decay factor
	vec3 l = gubo.lightColor; // Light color
	vec3 p = gubo.lightPos; // Light source position
	vec3 L = l*pow((g / length(p - pos)), beta); // Light color ----------- ???? formel slide 21+22

	return L;
}


/* Spot light models - projectors used to illuminate specific objects or locations*/

// Spot light direction
vec3 spot_light_dir(vec3 pos) {
	vec3 p = gubo.lightPos; // Light source position
	vec3 lx = normalize(p - pos); // Light direction
	return lx;
}

// Spot light color
vec3 spot_light_color(vec3 pos) {

	float g = gubo.coneInOutDecayExp.z; // Basic distance
	float beta = gubo.coneInOutDecayExp.w; // Decay factor
	vec3 d = gubo.lightDir; // Light direction
	vec3 l = gubo.lightColor; // Light color
	vec3 p = gubo.lightPos; // Light source position
	vec3 L_temp = l*pow((g/length(p - pos)), beta); // Formel slide 21+22

	float c_out = gubo.coneInOutDecayExp.x; // Cosine half outer cone angle
	float c_in = gubo.coneInOutDecayExp.y; // Cosine half inner cone angle
	
	float temp = clamp((dot(normalize(p - pos), d) - c_out)/(c_in - c_out), 0, 1);

	return L_temp*temp;
}


void main() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos.xyz - fragPos);
	
	vec3 lD;	// light direction from the light model
	vec3 lC;	// light color and intensity from the light model
	
	lD = direct_light_dir(fragPos) * gubo.selector.x +
	     point_light_dir(fragPos)  * gubo.selector.y +
	     spot_light_dir(fragPos)   * gubo.selector.z;
	lC = direct_light_color(fragPos) * gubo.selector.x +
	     point_light_color(fragPos)  * gubo.selector.y +
	     spot_light_color(fragPos)   * gubo.selector.z;

	vec3 Diffuse = (texture(texSampler, fragTexCoord).rgb * gubo.selector.w + vec3(1,1,1) * (1-gubo.selector.w)) * (max(dot(Norm, lD),0.0f) );
	vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(lD, Norm)),0.0f), 64.0f));
	
	outColor = vec4((Diffuse + Specular) * lC, 1.0f);	
}