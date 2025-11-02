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
    
    // diffuse [point light]
    float diffuseStrength = 0.2;
    vec3 norm = normalize(o_normals);
    vec3 lightDir = normalize(new_lightPos - o_fragPos); // both in world space
    float diff = max(dot(norm, lightDir), 0.0);
    diffuse += diffuseStrength * diff * u_lightColor;

    // specular 
    float specularStrength = 0.1;
    vec3 viewDir = normalize(u_viewPos - o_fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);
    specular += specularStrength * spec * u_lightColor;
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
