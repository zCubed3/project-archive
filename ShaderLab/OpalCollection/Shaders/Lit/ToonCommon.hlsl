#ifndef TOON_COMMON_HLSL
#define TOON_COMMON_HLSL

#include "UnityCG.cginc"
#include "UnityLightingCommon.cginc"
#include "AutoLight.cginc"
#include "UnityStandardBRDF.cginc"
#include "UnityPBSLighting.cginc"
#include "AutoLight.cginc"

#if !defined (SHADOWS_SCREEN) && !defined (SHADOWS_DEPTH) && !defined (SHADOWS_CUBE)

#define TOON_TRANSFER_SHADOW(i, info)
#define TOON_WRAP_SHADOW(info, OUT)

#else

#define SHADOW_WIDTH float4



#define TOON_TRANSFER_SHADOW(i, info) (info._ShadowCoord = i._ShadowCoord)
#define TOON_WRAP_SHADOW(info, OUT) \
    ToonShadowWrapper OUT = (ToonShadowWrapper)0; \
    OUT._ShadowCoord = info.shadowCoord;

#endif

#define TOON_REMAP_RANGE(x, lower, upper) ((x + lower) / (upper - lower))

struct ToonInput {
    float3 positionWS;
    float3 normalWS;
    float3 viewWS;

    SHADOW_COORDS(3)
};

struct ToonMaterial {
    half4 albedo;
    float roughness;
    float metallic;
    float specular;
    int bands;

    half3 ambient;
};

half4 ShadeToon(ToonInput info, ToonMaterial mat) {    
    float3 light = _WorldSpaceLightPos0;
    if (_WorldSpaceLightPos0.w > 0) {
        light = normalize(_WorldSpaceLightPos0 - info.positionWS);
    }

    float3 halfway = normalize(info.viewWS + light);

    float NDotV = saturate(dot(info.viewWS, info.normalWS));
    float NDotL = saturate(dot(light, info.normalWS));
    float NDotH = saturate(dot(halfway, info.normalWS));

    //TOON_WRAP_SHADOW(info, wrapper);
    UNITY_LIGHT_ATTENUATION(atten, info, info.positionWS);

    int bands = max(mat.bands, 1);
    float roughness = saturate(TOON_REMAP_RANGE(mat.roughness, 0.04, 1.0));
    float metallic = saturate(TOON_REMAP_RANGE(mat.metallic, 0.04, 1.0));
    float specular = mat.specular;
    half4 albedo = mat.albedo;

    float smoothness = 1.0 - roughness;

    half4 scatter = lerp(1.0, albedo, metallic);
    half4 conserve = lerp(1.0, 0, metallic);

    float attenuation = ceil(NDotL * atten * bands) / bands;
    half4 radiance = _LightColor0 * attenuation;

    half4 specularDisc = ceil(pow(NDotH, max(smoothness * 10000, 1))) * specular * scatter;
    
    half4 indirect = half4(mat.ambient, albedo.a);
    half4 direct = albedo * radiance * conserve;
    half4 highlight = specularDisc * radiance;

    half4 color = max(indirect, direct + highlight);

    UNITY_APPLY_FOG(i.fogCoord, color);

    return color;
}

#endif