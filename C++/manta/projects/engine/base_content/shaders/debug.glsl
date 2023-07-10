#version 330

#ifdef VERT
    layout(location = 0) in vec3 _vertex;
    layout(location = 1) in vec3 _normal;
    layout(location = 2) in vec2 _uv;
    layout(location = 3) in vec4 _tangent;

    uniform mat4 MANTA_MVP;
    uniform mat4 MANTA_M;
    uniform mat4 MANTA_M_I;
    uniform mat4 MANTA_M_IT;

    out vec2 uv;

    out vec3 obj_pos;
    out vec3 world_pos;

    out vec3 obj_normal;
    out vec3 world_normal;

    out vec3 obj_tangent;
    out vec3 world_tangent;

    out vec3 world_bitangent;

    void main() {
        gl_Position = MANTA_MVP * vec4(_vertex, 1.0);

        obj_pos = _vertex;
        world_pos = (MANTA_M * vec4(_vertex, 1.0)).xyz;

        obj_normal = _normal;
        world_normal = (MANTA_M_IT * vec4(_normal, 0.0)).xyz;

        obj_tangent = _tangent.xyz;
        world_tangent = (MANTA_M * vec4(obj_tangent, 0.0)).xyz;

        world_bitangent = cross(world_normal, world_tangent) * _tangent.w;

        uv = _uv;
    }
#endif

#ifdef FRAG
    out vec4 col;

    uniform float MANTA_TIME;
    uniform vec3 MANTA_CAM_POS;

    in vec2 uv;

    in vec3 obj_pos;
    in vec3 world_pos;

    in vec3 obj_normal;
    in vec3 world_normal;

    in vec3 obj_tangent;
    in vec3 world_tangent;

    in vec3 world_bitangent;

    void main() {
        float time = floor(abs(sin(MANTA_TIME / 10)) * 8.0);

        mat3 tan2World = mat3(
            normalize(world_tangent),
            normalize(world_bitangent),
            normalize(world_normal)
        );

        col = vec4(0, 0, 0, 1);

        // Position
        if (time == 0) {
            col = vec4(obj_pos, 1);
        }

        if (time == 1) {
            col = vec4(world_pos, 1);
        }

        // UVs
        if (time == 2) {
            col = vec4(uv, 0, 1);
        }

        // Normals
        if (time == 3) {
            col = vec4(normalize(obj_normal), 1);
        }

        if (time == 4) {
            col = vec4(normalize(world_normal), 1);
        }

        // Tangents
        if (time == 5) {
            col = vec4(normalize(obj_tangent), 1);
        }

        if (time == 6) {
            col = vec4(normalize(world_tangent), 1);
        }

        if (time == 7) {
            col = vec4(normalize(world_bitangent), 1);
        }

        if (time == 8) {
            vec3 view = normalize(MANTA_CAM_POS - world_pos);
            col = vec4(tan2World * view, 1);
        }
    }
#endif