#ifndef RT_SKY_HLSL
#define RT_SKY_HLSL

half3 simple_sky(ray_t ray) {
    // Super simple sky
    //const half3 BOTTOM_COLOR = half3(1, 0, 0);
    //const half3 MIDDLE_COLOR = half3(0, 1, 0);
    //const half3 TOP_COLOR = half3(0, 0, 1);

    const half3 BOTTOM_COLOR = half3(0, 0, 0);
    const half3 MIDDLE_COLOR = half3(0.4, 0.4, 1);
    const half3 TOP_COLOR = half3(0.5, 0.5, 1);

    const float BOTTOM_POWER = 5;
    const float TOP_POWER = 1;

    float blend = dot(ray.direction, float3(0, 1, 0)) + 1;

    half3 middle = lerp(BOTTOM_COLOR, MIDDLE_COLOR, pow(saturate(blend), BOTTOM_POWER));
    
    half3 sample = lerp(middle, TOP_COLOR, pow(saturate(blend - 1), TOP_POWER));

    return float3(0.0, 0.0, 0.1);
    return sample;
}

#endif