#version 430 core

layout(location=0) in vec3 i_position;
layout(location=1) in vec2 i_uv;
layout(location=2) in vec3 i_normals;
layout(location=3) in vec3 i_tangents;
layout(location=4) in vec3 i_bitangents;

layout(location=0) out vec3 o_fragPos;
layout(location=1) out vec3 o_normals;
layout(location=2) out vec2 o_uv;
layout(location=3) out vec3 o_tangents;
layout(location=4) out vec3 o_bitangents;
layout(location=5) out vec3 o_gouraudShadingResult;
layout(location=6) out vec4 o_fragPosLightSpace[9];

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform mat4 u_lightProjectionViewMatrix[9];
uniform float u_lightAttenLinear;
uniform float u_lightAttenQuad;
uniform vec3 u_lightTargetDirection;
uniform float u_lightInnerCutOffAngle;
uniform float u_lightOuterCutOffAngle;
uniform float u_lightAmbientStrength;
uniform float u_lightDiffuseStrength;
uniform float u_lightSpecularStrength;
uniform vec3 u_dirLightPosition;

uniform vec3 u_viewPos; // for specular
uniform int u_isPhong;

const int numLights = 9;
const float distBwLightRow = 4.01f;
const float distBwLightCol = 4.1f;

float innerCutOff = cos(radians(u_lightInnerCutOffAngle));
float outerCutOff = cos(radians(u_lightOuterCutOffAngle));

vec3 GouraudShading(vec3 o_fragPos, vec3 o_normals) 
{
  vec3 ambient = vec3(0.0f);
  vec3 diffuse = vec3(0.0f);
  vec3 specular = vec3(0.0f);

  vec3 new_lightPos = vec3(0.0f);
  float refX = u_lightPos.x;
  float refY = u_lightPos.y;
  float refZ = u_lightPos.z;

  // Ambient
  ambient = u_lightAmbientStrength * u_lightColor;

  for (int i = 0; i < numLights; i++)
  {
    new_lightPos.x = refX -  (distBwLightCol * (i / 3));
    new_lightPos.y = refY;
    new_lightPos.z = refZ - (distBwLightRow * (i % 3)); 


    vec3 lightDir = normalize(new_lightPos - o_fragPos); // both in world space
                                                         // Now the light goes from frag to light source
                                                         // same as how normal goes
 
    {
      // attenuation
      float distance = length(o_fragPos - new_lightPos);
      float attenuation = 1.0 / (1.0 + (u_lightAttenLinear * distance) + (u_lightAttenQuad * distance * distance));

      // diffuse 
      vec3 norm = normalize(o_normals);
      float diff = max(dot(norm, lightDir), 0.0);
      diffuse += u_lightDiffuseStrength * diff * u_lightColor * attenuation;

      // specular 
      vec3 viewDir = normalize(u_viewPos - o_fragPos);
      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
      specular += 0.8 * u_lightSpecularStrength * spec * u_lightColor * attenuation;
    }
    ///
  }

  vec3 result = (ambient + diffuse + specular);
  return result;
}


void main()
{
  o_fragPos = vec3(u_model * vec4(i_position, 1.0));
  o_uv = i_uv;
  o_normals = normalize(mat3(transpose(inverse(u_model))) * i_normals);
  o_tangents = normalize(mat3(u_model) * i_tangents);
  o_bitangents = normalize(mat3(u_model) * i_bitangents);
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
