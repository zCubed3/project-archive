#ifndef RT_TRIANGLE_HLSL
#define RT_TRIANGLE_HLSL

#include "RTCommon.hlsl"
#include "RTStructures.hlsl"

//
// Barycentric interpolation
//
#define BARYCENTRIC_INTERPOLATION(v1, v2, v3, u, v) ((v1 * (1 - u - v)) + (v2 * u) + (v3 * v))

#define DEFINE_BARY_FUNCTION(TYPE) \
inline TYPE bary_interpolate(TYPE v1, TYPE v2, TYPE v3, float u, float v) { \
    return BARYCENTRIC_INTERPOLATION(v1, v2, v3, u, v); \
}

DEFINE_BARY_FUNCTION(float)
DEFINE_BARY_FUNCTION(float2)
DEFINE_BARY_FUNCTION(float3)
DEFINE_BARY_FUNCTION(float4)

inline rt_vertex_t bary_interpolate(rt_vertex_t v0, rt_vertex_t v1, rt_vertex_t v2, float u, float v, bool normalize_vt = true) {
    rt_vertex_t vt = (rt_vertex_t)0;

    vt.position = bary_interpolate(v0.position, v1.position, v2.position, u, v);
    vt.normal = bary_interpolate(v0.normal, v1.normal, v2.normal, u, v);
    vt.uv0 = bary_interpolate(v0.uv0, v1.uv0, v2.uv0, u, v);

    [branch]
    if (normalize_vt)
        vt.normal = normalize(vt.normal);

    return vt;
}

//
// Ray->Triangle intersection
//

#define TRI_TEST_HALF_PRECISION

#ifndef TRI_TEST_HALF_PRECISION

#define tri_real_t float
#define tri_real3_t float3

#else

#define tri_real_t min16float 
#define tri_real3_t min16float3

#endif

struct tri_intersection_t {
    tri_real3_t t;
    tri_real_t u, v;
};

bool ray_triangle_intersect(ray_t ray, tri_real3_t v1, tri_real3_t v2, tri_real3_t v3, out tri_intersection_t intersect) {
    intersect = (tri_intersection_t)0;
    
    tri_real3_t edge1, edge2;

    edge1 = v2 - v1;
    edge2 = v3 - v1;

    tri_real3_t perpendicular;
    tri_real_t determinant;

    perpendicular = cross(ray.direction, edge2);
    determinant = dot(edge1, perpendicular);

    // Ignore this face if it reaches near Epsilon
    if (determinant < EPSILON)
        return false;

    tri_real_t ideterminant = 1.0 / determinant;

    tri_real3_t tvec = ray.origin - v1;

    intersect.u = dot(tvec, perpendicular) * ideterminant;

    if (intersect.u < 0.0f || intersect.u > 1.0f)
        return false;

    tri_real3_t q = cross(tvec, edge1);

    intersect.v = dot(ray.direction, q) * ideterminant;

    if (intersect.v < 0.0f || intersect.u + intersect.v > 1.0f)
        return false;

    intersect.t = dot(edge2, q) * ideterminant;
    
    return dot(edge2, q) * ideterminant > EPSILON;
    
    //return true;
    //return false;
}

#endif