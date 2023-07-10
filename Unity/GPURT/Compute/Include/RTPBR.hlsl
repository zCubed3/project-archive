#ifndef RT_PBR_HLSL
#define RT_PBR_HLSL

#include "RTCommon.hlsl"

//
// Area lighting
//
float3 sphere_area_light(float3 reflection, float3 light_vec, float radius) 
{
    float3 centerToRay = dot(light_vec, reflection) * reflection - light_vec;
    return light_vec + centerToRay * saturate(radius / length(centerToRay));
}

// https://github.com/turanszkij/WickedEngine/blob/62d1d02691286cc6c25da61294bfb416d018782b/WickedEngine/lightingHF.hlsli#L368
float3 sphere_light(float cos_theta, float sin_sigma_sqr) {
	float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

	float illuminance = 0.0f;
	// Note: Following test is equivalent to the original formula. 
	// There is 3 phase in the curve: cosTheta > sqrt(sinSigmaSqr), 
	// cosTheta > -sqrt(sinSigmaSqr) and else it is 0 
	// The two outer case can be merge into a cosTheta * cosTheta > sinSigmaSqr 
	// and using saturate(cosTheta) instead. 
	if (cos_theta * cos_theta > sin_sigma_sqr) {
		illuminance = PI * sin_sigma_sqr * saturate(cos_theta);
	} else {
		float x = sqrt(1.0f / sin_sigma_sqr - 1.0f); // For a disk this simplify to x = d / r 
		float y = -x * (cos_theta / sin_theta);
		float sin_theta_sqrt_y = sin_theta * sqrt(1.0f - y * y);
		illuminance = (cos_theta * acos(y) - x * sin_theta_sqrt_y) * sin_sigma_sqr + atan(sin_theta_sqrt_y / x);
	}

	return max(illuminance, 0.0f);
}

//
// Approximations
//
// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
float3 fresnel_schlick(float cosTheta, float3 F0, float fPow) {
   return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), fPow);
}

float3 fresnel_schlick_roughness(float cosTheta, float3 F0, float roughness) {
   return F0 + (max((1.0 - roughness).xxx, F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//
// Scattering
//
float distribution_ggx(float3 N, float3 H, float a)
{
   float a2     = a*a;
   float NdotH  = max(dot(N, H), 0.0);
   float NdotH2 = NdotH*NdotH;

   float nom    = a2;
   float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
   denom        = PI * denom * denom;

   return nom / denom;
}

float geometry_schlick_ggx(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometry_smith(float3 N, float3 V, float3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometry_schlick_ggx(NdotV, k);
    float ggx2 = geometry_schlick_ggx(NdotL, k);

    return ggx1 * ggx2;
}

#endif