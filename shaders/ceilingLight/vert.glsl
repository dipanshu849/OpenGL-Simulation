#version 430 core

layout(location=0) in vec3 i_position;
layout(location=1) in vec2 i_texCoordinates;

layout(location=0) out vec3 o_fragPos;
layout(location=2) out vec2 o_uv;
layout(location=3) out vec3 o_gouraudShadingResult;

uniform mat4 u_model; // Local to world
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_lightColor;
uniform float u_lightAmbientStrength;
uniform int u_isPhong;


vec3 GouraudShading() 
{
  vec3 ambient = vec3(0.0f);

  // Ambient
  ambient = 5.0 * u_lightAmbientStrength * u_lightColor;

  vec3 result = ambient;
  return result;
}


void main() {
  // Just to get coord of world space, as the light position 
  // is defined in world space
  o_fragPos = vec3(u_model * vec4(i_position, 1.0));
  o_uv = i_texCoordinates;
  
  o_gouraudShadingResult = vec3(0.0, 0.0, 0.0);
  if (u_isPhong == 0)
  {
    o_gouraudShadingResult = GouraudShading();
  }

  gl_Position = u_projection * u_view * vec4(o_fragPos, 1.0);
}
