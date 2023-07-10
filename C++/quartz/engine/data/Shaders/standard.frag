#version 330 core

#define LIGHT_LIMIT 32

in vec3 vert_pos;
in vec3 view_pos;

in vec2 uv;

in vec3 normal;
in vec3 normal_raw;
in vec3 normal_camera;

out vec4 color;

uniform float _time;

uniform float roughness;

uniform int light_count;
uniform vec3 light_positions[LIGHT_LIMIT];
uniform vec3 light_colors[LIGHT_LIMIT];
uniform float light_strengths[LIGHT_LIMIT];
uniform float light_ranges[LIGHT_LIMIT];
uniform int light_types[LIGHT_LIMIT];

uniform vec3 camera_position;

uniform vec4 color_albedo;
uniform float gloss;

struct LightCalculation 
{
  vec3 diffuse;
  vec3 specular;
};

float getFresnel() { return clamp(dot(normalize(normal_camera), normalize(camera_position - vert_pos)), 0, 1); }

vec3 blinnPhongSpecular(vec3 lightDirection, vec3 lightColor, float shiny, float attenuation) 
{
  vec3 halfwayDirection = normalize(normalize(lightDirection) + normalize(camera_position - vert_pos));
  float spec = clamp(pow(max(dot(normalize(normal), halfwayDirection), 0.0), shiny) * attenuation, 0.0, 1.0);

  return lightColor * spec;
}

float sphereAttenuation(vec3 position, float range, float strength) 
{
    vec3 direction = position - vert_pos;
    float dist = length(direction);

    float falloff = pow(1 - dist / range, 1.0 / strength);
    return max(dot(normalize(normal), normalize(direction)) * falloff, 0.0);
}

LightCalculation calculateLight(int index) 
{
  LightCalculation ret;

  ret.diffuse = vec3(0);
  ret.specular = vec3(0);

  bool doSpecular = gloss > 0;

  if (light_types[index] == 0) 
  {
    ret.diffuse = light_colors[index] * clamp(dot(normalize(normal), normalize(light_positions[index])), 0, 1);

    if (doSpecular)
      ret.specular = blinnPhongSpecular(clamp(light_positions[index], 0, 1), light_colors[index], mix(0, 256, gloss), 1);
  }

  if (light_types[index] == 1) 
  {
    float sphereAtten = sphereAttenuation(light_positions[index], light_ranges[index], light_strengths[index]);
    ret.diffuse = clamp(light_colors[index] * sphereAtten, 0, 1);

    if (doSpecular)
      ret.specular = blinnPhongSpecular(light_positions[index] - vert_pos, light_colors[index], mix(0, 256, gloss), sphereAtten);
  }

  return ret;
}

void main()
{
  vec3 lightColor = vec3(0, 0, 0);
  vec3 lightSpecular = vec3(0, 0, 0);

  float fresnel = getFresnel();

  for (int l = 0; l < light_count; l++) 
  {
    LightCalculation calcLight = calculateLight(l);

    lightColor += calcLight.diffuse;
    lightSpecular += calcLight.specular;
  }

  vec3 finalColor = lightColor + lightSpecular;

  color = (color_albedo * vec4(finalColor, 1));
}
