#ifndef RT_AABB_HLSL
#define RT_AABB_HLSL

#include "RTStructures.hlsl"

bool ray_aabb_intersect(ray_t ray, rt_aabb_t aabb, float tmin, float tmax) {
    float3 invD = rcp(ray.direction);
    float3 t0s = (aabb.min - ray.origin) * invD;
    float3 t1s = (aabb.max - ray.origin) * invD;

    float3 tsmaller = min(t0s, t1s);
    float3 tbigger = max(t0s, t1s);

    tmin = max(tmin, max(tsmaller[0], max(tsmaller[1], tsmaller[2])));
    tmax = min(tmax, min(tbigger[0], min(tbigger[1], tbigger[2])));

    return (tmin < tmax);
}

#endif