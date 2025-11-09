#version 410 core

layout(location=0) in vec3 i_fragPos;
layout(location=1) in vec3 i_normals;
layout(location=2) in vec2 i_uv;
layout(location=3) in vec3 i_gouraudShadingResult;
// layout(location=4) in vec4 i_shadowCoordinate;

out vec4 o_fragColor;

uniform sampler2D u_texture;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform vec3 u_viewPos;
uniform int u_isPhong;

const int numLights = 9;
const float distBwLightRow = 4.0f;
const float distBwLightCol = 4.0f;

const float attenLinear = 0.14f;
const float attenQuad = 0.07f;

const vec3 lightDirLocal = vec3(0.0f, -1.0f, 0.0f);

const float innerCutOff = cos(radians(75.0));
const float outerCutOff = cos(radians(90.0));

vec3 PhongShading() 
{
  // ambient
  float ambientStrength = 0.2;
  vec3 ambient = ambientStrength * u_lightColor;

  vec3 diffuse = vec3(0.0f);
  vec3 specular = vec3(0.0f);

  vec3 new_lightPos = vec3(0.0f);
  float refX = u_lightPos.x;
  float refY = u_lightPos.y;
  float refZ = u_lightPos.z;

  for (int i = 0; i < numLights; i++)
  {
    new_lightPos.x = refX -  (distBwLightCol * (i / 3));
    new_lightPos.y = refY;
    new_lightPos.z = refZ - (distBwLightRow * (i % 3)); 

    // spot light [inner and outcone]
    vec3 lightDir = normalize(new_lightPos - i_fragPos); // both in world space
    float theta = dot(lightDir, normalize(-lightDirLocal)); // now both point in same direction [towards light source]
    float epsilon = innerCutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    // attenuation
    // [value of taken from learnopengl.com]
    float distance = length(i_fragPos - new_lightPos);
    float attenuation = 1.0 / (1.0 + (attenLinear * distance) + (attenQuad * distance * distance));
      
    // diffuse [point light]
    float diffuseStrength = 0.8;
    vec3 norm = normalize(i_normals);
    float diff = max(dot(norm, lightDir), 0.0);
    diffuse += diffuseStrength * diff * u_lightColor * attenuation * intensity;

    // specular 
    float specularStrength = 1.0;
    vec3 viewDir = normalize(u_viewPos - i_fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);
    specular += specularStrength * spec * u_lightColor * attenuation * intensity;
  }

  vec3 result = (ambient + diffuse + specular);
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
