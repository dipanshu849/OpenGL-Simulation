#version 430 core

layout(location=0) in vec3 i_fragPos;
layout(location=2) in vec2 i_uv;
layout(location=3) in vec3 i_gouraudShadingResult;

out vec4 o_fragColor;

layout(binding=9) uniform sampler2D u_texture;

uniform int u_isPhong;
uniform vec3 u_lightColor;
uniform float u_lightAmbientStrength;

vec3 PhongShading() 
{
  vec3 ambient = vec3(0.0f);

  // Ambient
  ambient = 5.0 * u_lightAmbientStrength * u_lightColor;

  vec3 result = ambient;
  return result;
}


void main() 
{
  o_fragColor = texture(u_texture, i_uv);
  vec3 result = vec3(0.0, 0.0, 0.0); 


  if (u_isPhong == 1)
  {
    result = PhongShading() * vec3(o_fragColor);  
  }
  else
  {
    result = i_gouraudShadingResult * vec3(o_fragColor);
  }

  o_fragColor = vec4(result, 1.0);
}
