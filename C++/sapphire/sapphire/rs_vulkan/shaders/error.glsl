#ifdef PER_VERT

#ifdef VERTEX
layout(location = 0) in vec3 _VERT_POSITION;
layout(location = 1) in vec3 _VERT_NORMAL;
layout(location = 2) in vec2 _VERT_UV0;

layout(location = 0) out vec4 _VERT_COLOR;

void main() {
    gl_Position = OBJECT_DATA.model_view_projection * vec4(_VERT_POSITION, 1.0);

    vec3 world_position = vec3(OBJECT_DATA.model * vec4(_VERT_POSITION, 1.0));

    vec3 view = normalize(VIEW_DATA.camera_position.xyz - world_position);
    vec3 normal = vec3(OBJECT_DATA.model_inverse_transpose * vec4(_VERT_NORMAL, 0.0));

    float NdotV = max(0.0, dot(normal, view));

    _VERT_COLOR = vec4(1.0, 0.0, 0.0, 1.0);
    _VERT_COLOR *= NdotV * abs(sin(VIEW_DATA.time.x * 2));
}
#endif

#ifdef FRAGMENT
layout(location = 0) in vec4 _VERT_COLOR;

layout(location = 0) out vec4 _FRAG_COLOR;

void main () {
    _FRAG_COLOR = _VERT_COLOR;
}
#endif

#else

#ifdef VERTEX
layout(location = 0) in vec3 _VERT_POSITION;
layout(location = 1) in vec3 _VERT_NORMAL;
layout(location = 2) in vec2 _VERT_UV0;

layout(location = 0) out vec4 _WORLD_NORMAL;
layout(location = 1) out vec4 _WORLD_VIEW;

void main() {
    gl_Position = OBJECT_DATA.model_view_projection * vec4(_VERT_POSITION, 1.0);

    vec3 world_position = vec3(OBJECT_DATA.model * vec4(_VERT_POSITION, 1.0));

    _WORLD_VIEW = vec4(normalize(VIEW_DATA.camera_position.xyz - world_position), 0);
    _WORLD_NORMAL = vec4(vec3(OBJECT_DATA.model_inverse_transpose * vec4(_VERT_NORMAL, 0.0)), 0.0);
}
#endif

#ifdef FRAGMENT
layout(location = 0) in vec4 _WORLD_NORMAL;
layout(location = 1) in vec4 _WORLD_VIEW;

layout(location = 0) out vec4 _FRAG_COLOR;

void main () {
    float NdotV = max(0.0, dot(normalize(_WORLD_NORMAL.xyz), normalize(_WORLD_VIEW.xyz)));

    _FRAG_COLOR = vec4(1.0, 0.0, 0.0, 1.0);
    _FRAG_COLOR *= NdotV * abs(sin(VIEW_DATA.time.x * 2));
}
#endif

#endif