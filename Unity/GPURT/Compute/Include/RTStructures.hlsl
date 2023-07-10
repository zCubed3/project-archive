#ifndef RT_STRUCTURES_HLSL
#define RT_STRUCTURES_HLSL

#define RT_ELIGHT_POINT 0
#define RT_ELIGHT_SPOT 1
#define RT_ELIGHT_SUN 2

#define RT_ESHAPE_MESH 0
#define RT_ESHAPE_SPHERE 1

//#define RT_HALF_PRECISION

#ifndef RT_HALF_PRECISION

#define rt_real_t float
#define rt_real2_t float2
#define rt_real3_t float3
#define rt_real4_t float4

#else

#define rt_real_t min16float
#define rt_real2_t min16float2
#define rt_real3_t min16float3
#define rt_real4_t min16float4

#endif

struct rt_scene_t {
    float seed;
    float stochastic;
    uint object_count;
    uint light_count;
    float4 fog_params;
};

struct rt_light_t {
    float3 position;
    float3 color;
    float range;
    float radius;
    uint type;
    float3 cosines;
    float3 forward;
    float unused;
};

struct rt_aabb_t {
    float4 min;
    float4 max;
};

struct rt_mesh_t {
    uint index_offset;
    uint index_count;
    uint unused;
    uint unused2;
};

struct rt_material_t {
    float3 color;
    float3 glow;
    float roughness;
    float metallic;
};

struct rt_object_t {
    float4x4 trs;
    float4x4 trs_inverse;
    float4 shape_data;
    rt_aabb_t aabb;
    rt_mesh_t mesh;
    rt_material_t material;
};

struct rt_index_t {
    uint4 v;
};

struct rt_vertex_t {
    float3 position;
    float3 normal;
    float2 uv0;
};

struct rt_camera_t {
    float4x4 camera_to_world;
    float4x4 world_to_camera;
    float4x4 inverse_projection_matrix;
    float4x4 projection;
};

struct ray_t {
    float3 origin;
    float3 direction;
    float2 padding;
};

struct fragment_t {
    rt_real4_t position;
    rt_real3_t normal;
    uint object;
};

ray_t ray_new(float3 origin, float3 direction) {
    ray_t ray = (ray_t)0;
    
    ray.origin = origin;
    ray.direction = normalize(direction);

    return ray;
}

ray_t ray_from_camera(rt_camera_t camera, float2 view) {
    ray_t ray = (ray_t)0;

    ray.origin = mul(camera.camera_to_world, float4(0, 0, 0, 1)).xyz;

    ray.direction = mul(camera.inverse_projection_matrix, float4(view, 0, 1)).xyz;
    ray.direction = mul(camera.camera_to_world, float4(ray.direction, 0)).xyz;
    ray.direction = normalize(ray.direction);

    return ray;
}

#endif