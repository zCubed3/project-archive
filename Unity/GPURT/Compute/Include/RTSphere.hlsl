#ifndef RT_SPHERE_HLSL
#define RT_SPHERE_HLSL

#include "RTStructures.hlsl"

// http://three-eyed-games.com/2018/05/03/gpu-ray-tracing-in-unity-part-1/
bool ray_sphere_intersect(ray_t ray, inout fragment_t fragment, float4 sphere, float min_plane = 0, float max_plane = 100)
{
    // Calculate distance along the ray where the sphere is intersected
    float3 d = ray.origin - sphere.xyz;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.w * sphere.w;

    if (p2sqr < 0)
        return false;

    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;

    [branch]
    if (t > min_plane && t < max_plane)
    {
        fragment.position.w = t;
        fragment.position.xyz = ray.origin + t * ray.direction;
        fragment.normal = normalize(fragment.position - sphere.xyz);

        return true;
    }

    return false;
}

#endif