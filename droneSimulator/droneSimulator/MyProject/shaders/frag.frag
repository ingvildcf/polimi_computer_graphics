#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragViewDir;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
	const vec3  specColor = vec3(1.0f, 1.0f, 1.0f);
	const float specPower = 150.0f;
	const vec3  L = vec3(-0.4830f, 0.8365f, -0.2588f);
	
	vec3 N = normalize(fragNorm);
	vec3 R = -reflect(L, N);
	vec3 V = normalize(fragViewDir);
	
	// Lambert diffuse
	//vec3 diffuse  = diffColor * max(dot(N,L), 0.0f);
	//no lo pongo porque en vez de este usamos OREN NARAYAR, y no podemos definir diffuse 2 veces

	// Phong specular
	vec3 specular = specColor * pow(max(dot(R,V), 0.0f), specPower);

	// Hemispheric ambient
	vec3 ambient  = (vec3(0.1f,0.1f, 0.1f) * (1.0f + N.y) + vec3(0.0f,0.0f, 0.1f) * (1.0f - N.y)) * diffColor;
	
	//OREN NAYAR
	float sigma = 1.5;
	vec3 C = diffColor;
	float i = acos(dot(L,N));
	float r = acos(dot(V,N));
	float alpha = max(i, r);
	float beta = min(i,r);
	float A = 1 - 0.5*(pow(sigma,2)/(pow(sigma,2) + 0.33));
	float B = 0.45*(pow(sigma,2)/(pow(sigma,2) + 0.09));
	vec3 veci = normalize(L - (L*N)*N);
	vec3 vecr = normalize(V - (V*N)*N);
	float G = max(0, dot(veci, vecr));
	vec3 l = C * clamp(dot(L,N), 0, 1);
	vec3 diffuse = l*(A+B*G*sin(alpha)*tan(beta)); 

	outColor = vec4(clamp(ambient + diffuse + specular, vec3(0.0f), vec3(1.0f)), 1.0f);
}