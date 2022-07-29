#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightDir0;
	vec3 lightColor0;
	vec3 lightDir1;
	vec3 lightColor1;
	vec3 lightDir2;
	vec3 lightColor2;
	vec3 lightDir3;
	vec3 lightColor3;
	vec3 eyePos;
	vec4 selector;
} gubo;

/**** Modify from here *****/


vec3 Lambert_Diffuse_BRDF(vec3 L, vec3 N, vec3 V, vec3 C) {
	// Lambert Diffuse BRDF model
	// in all BRDF parameters are:
	// vec3 L : light direction (d)
	// vec3 N : normal vector (n_x)
	// vec3 V : view direction (omega_r) - independent
	// vec3 C : main color (diffuse color, or specular color)
	
	float cos_alpha = dot(N, L); // angle between N and L

	return C * max(cos_alpha, 0);
}

vec3 Oren_Nayar_Diffuse_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, float sigma) {
	// Directional light direction
	// additional parameter:
	// float sigma : roughness of the material

	float A = 1 - 0.5*pow(sigma, 2)/(pow(sigma, 2) + 0.33);
	float B = 0.45*pow(sigma, 2)/(pow(sigma, 2) + 0.09);
	
	float theta_i = radians(pow(cos(dot(L, N)), -1)); // ang between d (L) and n
	float theta_r = radians(pow(cos(dot(V, N)), -1)); // ang between omega_r (V) and n
	
	float alpha = max(theta_i, theta_r);
	float beta = min(theta_i, theta_r);

	// Projections of omega_r (V) and d (L) on the plane perpendicular to n
	vec3 v_r = normalize(V - (dot(V, N))*N);
	vec3 v_i = normalize(L - (dot(L, N))*N);

	float G = max(0, dot(v_i, v_r)); // cos(gamma), gamma = ang between v_r and v_i

	vec3 l = C * clamp(dot(L, N), 0, 1);

	return l*(A+B*G*sin(alpha)*tan(beta));
}

vec3 Phong_Specular_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, float gamma)  {
	// Phong Specular BRDF model
	// additional parameter:
	// float gamma : exponent of the cosine term
	
	vec3 R = -reflect(L, N); // = 2*N.dot(L.dot(N)) - L; // reflection
	
	float cos_alpha = pow(clamp(dot(V, R), 0, 1), gamma);

	return C*cos_alpha;
}

vec3 Toon_Diffuse_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, vec3 Cd, float thr) {
	// Toon Diffuse Brdf
	// additional parameters:
	// vec3 Cd : color to be used in dark areas
	// float thr : color threshold

	vec3 f_diffuse = vec3(0.0f); // Darkest areas
	
	float LN = dot(L, N);

	if (LN > 0 && LN < thr){
		f_diffuse = Cd; // Dark areas
	}
	else if (LN >= thr){ 
		f_diffuse = C; // Light areas
	}

	return f_diffuse;
}

vec3 Toon_Specular_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, float thr)  {
	// Directional light direction
	// additional parameter:
	// float thr : color threshold

	vec3 R = -reflect(L, N); //2*N*dot(L, N) - L;

	vec3 f_specular = C; // Light areas

	if (dot(V, R) < thr){
		f_specular = vec3(0.0f); // Dark areas
	}

	return f_specular;
}	


/**** To here *****/




vec3 BRDF_D(vec3 L, vec3 N, vec3 V, vec3 C) {
	return gubo.selector.x * Lambert_Diffuse_BRDF(L, N, V, C) +
		   (1.0 - gubo.selector.x) * (1.0 - gubo.selector.y) * 
		   			Toon_Diffuse_BRDF(L, N, V, C, 0.2f * C, 0.5) +
		   (1.0 - gubo.selector.x) * (gubo.selector.y) * 
		   			Oren_Nayar_Diffuse_BRDF(L, N, V, C, 1.5);
}

vec3 BRDF_S(vec3 L, vec3 N, vec3 V, vec3 C) {
	return Phong_Specular_BRDF(L, N, V, C, 200.0f) * gubo.selector.z +
		   (1.0 - gubo.selector.x) * (1.0 - gubo.selector.y) * 
		   			Toon_Specular_BRDF(L, N, V, vec3(1,1,1), 0.97f);
}

void main() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos.xyz - fragPos);
	
	float AmbFact = 0.025;
	
	vec3 DiffColor = texture(texSampler, fragTexCoord).rgb * gubo.selector.w + vec3(1,1,1) * (1-gubo.selector.w);
	vec3 Diffuse = vec3(0,0,0);
	
	Diffuse += BRDF_D(gubo.lightDir0, Norm, EyeDir, DiffColor) * gubo.lightColor0;
	Diffuse += BRDF_D(gubo.lightDir1, Norm, EyeDir, DiffColor) * gubo.lightColor1;
	Diffuse += BRDF_D(gubo.lightDir2, Norm, EyeDir, DiffColor) * gubo.lightColor2;
	Diffuse += BRDF_D(gubo.lightDir3, Norm, EyeDir, DiffColor) * gubo.lightColor3;
	
	vec3 Specular = vec3(0,0,0);
	Specular += BRDF_S(gubo.lightDir0, Norm, EyeDir, vec3(1,1,1)) * gubo.lightColor0;
	Specular += BRDF_S(gubo.lightDir1, Norm, EyeDir, vec3(1,1,1)) * gubo.lightColor1;
	Specular += BRDF_S(gubo.lightDir2, Norm, EyeDir, vec3(1,1,1)) * gubo.lightColor2;
	Specular += BRDF_S(gubo.lightDir3, Norm, EyeDir, vec3(1,1,1)) * gubo.lightColor3;

	vec3 Ambient = AmbFact * DiffColor;
	
	outColor = vec4(Diffuse + Specular + Ambient, 1.0f);	
}