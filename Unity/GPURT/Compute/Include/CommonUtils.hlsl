#ifndef COMMON_UTILS_HLSL
#define COMMON_UTILS_HLSL

#define COMMON_THREAD_SIZE 8

inline float2 id_to_uv(uint2 id, uint2 dimensions) {
    return (id.xy + 0.5) / dimensions;
}

#endif