#ifdef VERT
layout(location = 0) in vec3 _vertex;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec2 _uv;

uniform mat4 MANTA_MAT_P;
uniform mat4 MANTA_MAT_V;
uniform mat4 MANTA_MAT_M;

out vec3 normal;

void main() {
    gl_Position = MANTA_MAT_P * MANTA_MAT_V * MANTA_MAT_M * vec4(_vertex, 1); 
    normal = _normal;
}

#endif

#ifdef FRAG
in vec3 normal;
out vec4 color;

void main() {
    color = vec4(normal, 1);
}

#endif