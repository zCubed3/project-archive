#ifndef RT_COMMON_HLSL
#define RT_COMMON_HLSL

#define EPSILON 1e-8

#ifndef PI
#define PI 3.141592654
#endif

#define GOLDEN_RATIO_CONJUGATE 0.61803398875

#define RT_NORMAL_BIAS 0.001
#define RT_SHADOW_BIAS 0.000001

#define RT_NEAR_PLANE 0.1
#define RT_FAR_PLANE 100.0
#define RT_DEPTH_ACCEPT 0.001

#define RT_GI_POW 1.0

float rand(float2 st) { return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123); }
float rand(float n) { return frac(sin(n) * 43758.5453123); }

/*
inline float2 rand_dir(float2 st, float2 spread = 1.0) { 
    float x = ((rand(st.x) - 0.5) * 2.0); 
    float y = ((rand(st.y + 1) - 0.5) * 2.0); 

    return float2(x, y) * spread;
}
*/

float mod289(float x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
float4 mod289(float4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
float4 perm(float4 x) { return mod289(((x * 34.0) + 1.0) * x); }

float noise(float3 p){
    float3 a = floor(p);
    float3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    float4 b = a.xxyy + float4(0.0, 1.0, 0.0, 1.0);
    float4 k1 = perm(b.xyxy);
    float4 k2 = perm(k1.xyxy + b.zzww);

    float4 c = k2 + a.zzzz;
    float4 k3 = perm(c);
    float4 k4 = perm(c + 1.0);

    float4 o1 = frac(k3 * (1.0 / 41.0));
    float4 o2 = frac(k4 * (1.0 / 41.0));

    float4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    float2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

// https://gist.github.com/Piratkopia13/46c5dda51ed59cfe69b242deb0cf40ce
// Modified: signatures to match style

// Generates a seed for a random number generator from 2 inputs plus a backoff
uint rand_seed(uint val0, uint val1, uint backoff = 16) {
    uint v0 = val0, v1 = val1, s0 = 0;

    [unroll]
    for (uint n = 0; n < backoff; n++)
    {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

// Takes our seed, updates it, and returns a pseudorandom float in [0..1]
float rand_next(inout uint s) {
    s = (1664525u * s + 1013904223u);
    return float(s & 0x00FFFFFF) / float(0x01000000);
}

inline float2 rand_dir(inout uint seed, float2 spread = 1.0) { 
    float x = ((rand_next(seed) - 0.5) * 2.0); 
    float y = ((rand_next(seed) - 0.5) * 2.0); 

    return float2(x, y) * spread;
}

float3x3 angle_axis_3x3(float angle, float3 axis) {
    float c, s;
    sincos(angle, s, c);

    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return float3x3(
        t * x * x + c,      t * x * y - s * z,  t * x * z + s * y,
        t * x * y + s * z,  t * y * y + c,      t * y * z - s * x,
        t * x * z - s * y,  t * y * z + s * x,  t * z * z + c
    );
}

float3 rand_cone(inout uint randSeed, float3 direction, float coneAngle) {
    float cosAngle = cos(coneAngle);

    // Generate points on the spherical cap around the north pole [1].
    // [1] See https://math.stackexchange.com/a/205589/81266
    float z = rand_next(randSeed) * (1.0f - cosAngle) + cosAngle;
    float phi = rand_next(randSeed) * 2.0f * PI;

    float x = sqrt(1.0f - z * z) * cos(phi);
    float y = sqrt(1.0f - z * z) * sin(phi);
    float3 north = float3(0, 0, 1);

    // Find the rotation axis `u` and rotation angle `rot` [1]
    float3 axis = normalize(cross(north, normalize(direction)));
    float angle = acos(dot(normalize(direction), north));

    // Convert rotation axis and angle to 3x3 rotation matrix [2]
    float3x3 R = angle_axis_3x3(angle, axis);

    return mul(R, float3(x, y, z));
}

// Calculates a cone angle within a spherical radius
float get_cone_angle(float3 origin, float3 light_vec, float3 light_pos, float light_radius) {
    // Calculate a vector perpendicular to L
    float3 perpL = cross(light_vec, float3(0.f, 1.0f, 0.f));
    
    // Handle case where L = up -> perpL should then be (1,0,0)
    if (all(perpL == 0.0f)) {
        perpL.x = 1.0;
    }

    // Use perpL to get a vector from worldPosition to the edge of the light sphere
    float3 toLightEdge = normalize((light_pos + perpL * light_radius) - origin);

    // Angle between L and toLightEdge. Used as the cone angle when sampling shadow rays
    return acos(dot(light_vec, toLightEdge)) * 2.0f;
}

float remap(float x, float lower, float upper) {
    return (x + lower) / (upper - lower);
}

#endif