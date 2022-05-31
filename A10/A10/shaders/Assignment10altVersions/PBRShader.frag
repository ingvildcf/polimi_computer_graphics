#version 450#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec4 fragTan;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightDir;
	vec4 lightColor;
	vec3 eyePos;
} gubo;

layout(binding = 3) uniform sampler2D normSampler;
layout(binding = 4) uniform sampler2D matSampler;
layout(binding = 5) uniform samplerCube skybox;
layout(binding = 6) uniform sampler2D envBRDF;

vec2 BRDFLut(float ndotv, float rough) {
	float x = 1 - rough;
    float y = ndotv;
 
    float b1 = -0.1688;
    float b2 = 1.895;
    float b3 = 0.9903;
    float b4 = -4.853;
    float b5 = 8.404;
    float b6 = -5.069;
    float bias = clamp(min( b1 * x + b2 * x * x, b3 + b4 * y + b5 * y * y + b6 * y * y * y ), 0.0, 1.0);
 
    float d0 = 0.6045;
    float d1 = 1.699;
    float d2 = -0.5228;
    float d3 = -3.603;
    float d4 = 1.404;
    float d5 = 0.1939;
    float d6 = 2.661;
    float delta = clamp( d0 + d1 * x + d2 * y + d3 * x * x + d4 * x * y + d5 * y * y + d6 * x * x * x, 0.0, 1.0);
    float scale = delta - bias;
    
    return vec2(scale, bias);
}
// ----------------------------------------------------------------------------
const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------

void main() {
	vec3 Norm = normalize(fragNorm);
	vec3 Tan = normalize(fragTan.xyz - Norm * dot(fragTan.xyz, Norm));
	vec3 Bitan = cross(Norm, Tan) * fragTan.w;
	vec3 V = normalize(gubo.eyePos - fragPos);
	mat3 tbn = mat3(Tan, Bitan, Norm);
	vec4 nMap = texture(normSampler, fragTexCoord);
	vec3 N = normalize(tbn * (nMap.rgb * 2.0 - 1.0));
//	vec3 N = Norm;
	
	vec3 R = reflect(-V, N);
	vec4 MRAO = texture(matSampler, fragTexCoord);
	vec3 albedo = texture(texSampler, fragTexCoord).rgb;
//	vec3 albedo = vec3(1.0);
	float ao = MRAO.b;
	float roughness = MRAO.g;
	float metallic = MRAO.r;

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	{
        // calculate per-light radiance
        vec3 L = gubo.lightDir;
        vec3 H = normalize(V + L);
        vec3 radiance = gubo.lightColor.rgb;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	                
            
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo = (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }  








    const float MAX_REFLECTION_LOD = 8.0;

    vec3 prefilteredColor = textureLod(skybox, R,  roughness * MAX_REFLECTION_LOD).rgb;    
	vec3 F = F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - max(dot(N, V), 0.0), 0.0, 1.0), 5.0);
	vec2 envBRDF = BRDFLut(max(dot(N, V), 0.0), roughness);
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);	
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;	  
	  
	vec3 irradiance = textureLod(skybox, N,  MAX_REFLECTION_LOD - 1.0).rgb;
	vec3 diffuse    = irradiance * albedo;
	  	  
	vec3 ambient = (kD * diffuse + specular) * ao;
	vec3 color = ambient + Lo;
	color = color / (color + vec3(1.0));
//	color = pow(color, vec3(1.0/2.2));
 	
	outColor = vec4(clamp(vec3(1.0,1.0,1.0)-color, 0.0, 1.0), 1.0);
}