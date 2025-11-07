#version 410 core

in vec3 o_fragPos;
in vec3 o_normals;
in vec2 o_uv;
in vec3 o_gouraudShadingResult;

out vec4 FragColor;

uniform sampler2D u_texture;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform vec3 u_viewPos;
uniform int u_isPhong;

const int numLights = 9;
const float distBwLightRow = 4.0f;
const float distBwLightCol = 4.0f;
const float attenLinear = 0.09f;
const float attenQuad = 0.032f;
const vec3 lightDirLocal = vec3(0.0f, -1.0f, 0.0f);
const float innerCutOff = cos(radians(60.0));
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
    vec3 lightDir = normalize(new_lightPos - o_fragPos); // both in world space
    float theta = dot(lightDir, normalize(-lightDirLocal)); // now both point in same direction [towards light source]
    float epsilon = innerCutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    // attenuation
    // [value of taken from learnopengl.com]
    float distance = length(o_fragPos - new_lightPos);
    float attenuation = 1.0 / (1.0 + (attenLinear * distance) + (attenQuad * distance * distance));
      
    // diffuse [point light]
    float diffuseStrength = 0.6;
    vec3 norm = normalize(o_normals);
    float diff = max(dot(norm, lightDir), 0.0);
    diffuse += diffuseStrength * diff * u_lightColor * attenuation * intensity;

    // specular 
    float specularStrength = 0.2;
    vec3 viewDir = normalize(u_viewPos - o_fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);
    specular += specularStrength * spec * u_lightColor * attenuation * intensity;
  }

  vec3 result = (ambient + diffuse + specular);
  return result;
}


void main() 
{
  FragColor = texture(u_texture, o_uv);
  vec3 result = vec3(0.0, 0.0, 0.0); 


  if (u_isPhong == 1)
  {
    result = PhongShading() * vec3(FragColor);  
  }
  else
  {
    result = o_gouraudShadingResult * vec3(FragColor);
  }

  FragColor = vec4(result, 1.0);
}
