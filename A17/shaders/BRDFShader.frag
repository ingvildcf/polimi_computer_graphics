#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
	// Direct lights parameters (for diffuse and specular)
	vec3 lightDir0;
	vec3 lightColor0;
	vec3 lightDir1;
	vec3 lightColor1;
	vec3 lightDir2;
	vec3 lightColor2;
	vec3 lightDir3;
	vec3 lightColor3;

	// Ambient light parameters
	vec3 AmbColor;	// also Bottom color for Hemispheric light,
					//      Constant term for Spherical Harmonics light
	vec3 TopColor;	// also DyColor for Spherical Harmonics light
	vec3 DzColor;
	vec3 DxColor;
	
	// Other parameters (not relevant for the exercise)
	vec3 eyePos;
	vec4 selector;
} gubo;

/**** Modify from here *****/

layout(location = 2) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNorm;
layout(location = 0) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

// Method from A16
vec3 Lambert_Diffuse_BRDF(vec3 L, vec3 N, vec3 V, vec3 C) {
	// Lambert Diffuse BRDF model
	// in all BRDF parameters are:
	// vec3 L : light direction (d)
	// vec3 N : normal vector (n_x)
	// vec3 V : view direction (omega_r) - independent
	// vec3 C : main color (diffuse color, or specular color)
	
	float cos_alpha = dot(L, N); // angle between N and L
	
	// vec3 prev = C*max(cos_alpha, 0);

	return C*max(cos_alpha, 0);
}

// Method from A16
vec3 Oren_Nayar_Diffuse_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, float sigma) {
	// Directional light direction
	// additional parameter:
	// float sigma : roughness of the material

	float A = 1 - 0.5*pow(sigma, 2)/(pow(sigma, 2) + 0.33);
	float B = 0.45*pow(sigma, 2)/(pow(sigma, 2) + 0.09);
	
	float theta_i = radians(pow(cos(dot(L, N)), -1));
	float theta_r = radians(pow(cos(dot(V, N)), -1));
	
	float alpha = max(theta_i, theta_r);
	float beta = min(theta_i, theta_r);

	vec3 v_i = normalize(L - (dot(L, N))*N);
	vec3 v_r = normalize(V - (dot(V, N))*N);
	float G = max(0, dot(v_i, v_r));

	vec3 l = C * clamp(dot(L, N), 0, 1);

	return l*(A+B*G*sin(alpha)*tan(beta));
}

// Blinn specular reflection model
vec3 Blinn_Specular(vec3 L, vec3 N, vec3 V, vec3 C, float gamma) {
	
	vec3 h = normalize(L + V); // half vector

	vec3 f_specular = C * pow(clamp(dot(N, h), 0, 1), gamma); // Specular 
	
	return f_specular;
}

vec3 Case1_Color(vec3 N, vec3 V, vec3 Cd, vec3 Ca, float sigma) {
	// Oren Nayar Diffuse + Ambient
	// No Specular
	// One directional light (lightDir0 and lightColor0)
	//
	// Parameters are:
	//
	// vec3 N : normal vector
	// vec3 V : view direction
	// vec3 Cd : main color (diffuse color)
	// vec3 Ca : ambient color
	// float sigma : roughness of the material
	
	vec3 L = gubo.lightDir0;
	vec3 C = gubo.lightColor0;

	vec3 diff = Oren_Nayar_Diffuse_BRDF(L, N, V, Cd, sigma) * C; // Diffuse
	vec3 amb = Ca * gubo.AmbColor; // Ambient

	return diff + amb;
}

vec3 Case2_Color(vec3 N, vec3 V, vec3 Cd, vec3 Ca) {
	// Lambert Diffuse + Hemispheric
	// No Specular
	// One directional light (lightDir0 and lightColor0)
	// Hemispheric light oriented along the y-axis
	//
	// Parameters are:
	//
	// vec3 N : normal vector (n_x)
	// vec3 V : view direction (omega_r)
	// vec3 Cd : main (diffuse) color (l)
	// vec3 Ca : ambient color reflection (m_A)

	vec3 HemiDir = vec3(0.0f, 1.0f, 0.0f);

	vec3 L = gubo.lightDir0;
	vec3 diff = Lambert_Diffuse_BRDF(L, N, V, Cd);
	
	vec3 l_U = gubo.TopColor; // Ambient color top
	vec3 l_D = gubo.AmbColor; // Ambient color bottom

	vec3 l_A = ((dot(N, HemiDir) + 1)/2)*l_U + (1 - dot(N, HemiDir)/2)*l_D; // Ambient light color
	vec3 amb = l_A * Ca;

	return diff + amb;
}

vec3 Case3_Color(vec3 N, vec3 V, vec3 Cs, vec3 Ca, float gamma)  {
	// Spherical Harmonics
	// Blinn Specular
	// Four directional lights (lightDir0 to lightDir3, and lightColor0 to lightColor3)
	//
	// Parameters are:
	//
	// vec3 N : normal vector
	// vec3 V : view direction
	// vec3 Cs : specular color
	// vec3 Ca : ambient color
	// float gamma : Blinn exponent
	
	vec3 spec = vec3(0,0,0);
	
	spec += Blinn_Specular(gubo.lightDir0, N, V, Cs, gamma) * gubo.lightColor0;
	spec += Blinn_Specular(gubo.lightDir1, N, V, Cs, gamma) * gubo.lightColor1;
	spec += Blinn_Specular(gubo.lightDir2, N, V, Cs, gamma) * gubo.lightColor2;
	spec += Blinn_Specular(gubo.lightDir3, N, V, Cs, gamma) * gubo.lightColor3;
	
	vec3 l_C = gubo.AmbColor; // Basic color
	
	// Deviation terms along x, y, z
	vec3 delta_lx = gubo.DxColor; 
	vec3 delta_ly = gubo.TopColor;
	vec3 delta_lz = gubo.DzColor;

	// Spherical Harmonics
	vec3 l_A = l_C + N.x * delta_lx + N.y * delta_ly + N.z * delta_lz;
	vec3 amb = Ca * l_A;

	return spec + amb;
}

/**** To here *****/


void main() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos.xyz - fragPos);
	
	float AmbFact = 0.025;
	
	vec3 DifCol = texture(texSampler, fragTexCoord).rgb * gubo.selector.w +
				  vec3(1,1,1) * (1-gubo.selector.w);

	vec3 CompColor = gubo.selector.x *
						Case1_Color(Norm, EyeDir, DifCol, DifCol, 1.2f) +
					 gubo.selector.y *
						Case2_Color(Norm, EyeDir, DifCol, DifCol) +
					 gubo.selector.z *
						Case3_Color(Norm, EyeDir, vec3(1.0f,1.0f,1.0f), DifCol, 200.0f);
	
	outColor = vec4(CompColor, 1.0f);	
}