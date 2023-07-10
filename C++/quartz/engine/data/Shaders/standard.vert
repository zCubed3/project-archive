#version 330 core
layout(location = 0) in vec3 _pos;
layout(location = 1) in vec2 _uv;
layout(location = 2) in vec3 _normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

uniform float _time;

out vec3 vert_pos;
out vec3 view_pos;

out vec3 normal;
out vec3 normal_raw;
out vec3 normal_camera;

out vec2 uv;

void main()
{
  mat4 MVP = P * V * M;

  gl_Position = MVP * vec4(_pos, 1);

  vert_pos = (M * vec4(_pos, 1)).xyz;

  vec3 viewVertex = (V * M * vec4(_pos, 1)).xyz;

  mat3 mx3norm = mat3(M); 
  mx3norm = inverse(mx3norm);
  mx3norm = transpose(mx3norm);

  normal = (mx3norm * _normal);
  normal_raw = _normal;
  normal_camera = (V * M * vec4(_normal, 0)).xyz;

  uv = _uv;
}
