#ifdef VERTEX
layout(location = 0) in vec3 _VERT_POSITION;
layout(location = 1) in vec3 _VERT_NORMAL;
layout(location = 2) in vec2 _VERT_UV0;

void main() {
    gl_Position = OBJECT_DATA.model_view_projection * vec4(_VERT_POSITION, 1.0);
}
#endif

#ifdef FRAGMENT
void main () {

}
#endif