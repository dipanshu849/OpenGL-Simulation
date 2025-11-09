#version 430 core

layout(location=0) in vec3 i_position;
layout(location=1) in vec2 i_texCoordinates;
layout(location=2) in vec3 i_normals;

layout(location=0) out vec3 o_fragPos;
layout(location=1) out vec3 o_normals;
layout(location=2) out vec2 o_uv;
layout(location=3) out vec3 o_gouraudShadingResult;
layout(location=4) out vec4 o_fragPosLightSpace[9];

uniform mat4 u_model; // Local to world
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform mat4 u_lightProjectionViewMatrix[9];

uniform vec3 u_viewPos;
uniform int u_isPhong;



vec3 GouraudShading(vec3 o_fragPos, vec3 o_normals)
{
  // ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * u_lightColor;

  // diffuse [point light]
  vec3 norm = normalize(o_normals);
  vec3 lightDir = normalize(u_lightPos - o_fragPos); // both in world space
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * u_lightColor;

  // specular [left]
  float specularStrength = 0.5;
  vec3 viewDir = normalize(u_viewPos - o_fragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
  vec3 specular = specularStrength * spec * u_lightColor;


  vec3 result = (ambient + diffuse + specular);
  return result;
}

void main() {
  // Just to get coord of world space, as the light position 
  // is defined in world space
  o_fragPos = vec3(u_model * vec4(i_position, 1.0));

  // Similarly to get coord of world space for normals, but
  // the problem with normal scaling, when scaling in model
  // matrix is not uniform, the normals are no longer normals
  o_normals = normalize(mat3(transpose(inverse(u_model))) * i_normals);

  o_uv = i_texCoordinates;
  
  o_gouraudShadingResult = vec3(0.0, 0.0, 0.0);
  if (u_isPhong == 0)
  {
    o_gouraudShadingResult = GouraudShading(o_fragPos, o_normals);
  }

  for (int i = 0; i < 9; i++)
  {
    o_fragPosLightSpace[i] = u_lightProjectionViewMatrix[i] * vec4(o_fragPos, 1.0);
  }

  gl_Position = u_projection * u_view * vec4(o_fragPos, 1.0);
}
