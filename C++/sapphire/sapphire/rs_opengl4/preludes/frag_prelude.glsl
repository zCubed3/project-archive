//
// Fragment prelude
//
#version 460

#define FRAG
#define FRAGMENT
#define FRAGMENT_STAGE

layout (std140) uniform SAPPHIRE_VIEW_DATA {
    // Camera data
    mat4 projection;
    mat4 view;
    mat4 view_projection;
    vec4 camera_position;

    // View-world data
    vec4 time;
} VIEW_DATA;

layout (std140) uniform SAPPHIRE_OBJECT_DATA {
    mat4 model_view_projection;
    mat4 model;
    mat4 model_inverse;
    mat4 model_inverse_transpose;
} OBJECT_DATA;

//
// API defines
//
#ifndef OPENGL4
#define OPENGL4
#endif
