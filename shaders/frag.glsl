#version 430 core

layout(location=0) in vec3 i_fragPos;
layout(location=1) in vec3 i_normals;
layout(location=2) in vec2 i_uv;
layout(location=3) in vec3 i_gouraudShadingResult;
layout(location=4) in vec4 i_fragPosLightSpace[9];

out vec4 o_fragColor;

layout(binding=0) uniform sampler2D u_ShadowMaps[9];
layout(binding=9) uniform sampler2D u_texture;

uniform vec3 u_viewPos;
uniform int u_isPhong;

uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform float u_lightAttenLinear;
uniform float u_lightAttenQuad;
uniform vec3 u_lightTargetDirection;
uniform float u_lightInnerCutOffAngle;
uniform float u_lightOuterCutOffAngle;
uniform float u_lightAmbientStrength;
uniform float u_lightDiffuseStrength;
uniform float u_lightSpecularStrength;

const int numLights = 9;
const float distBwLightRow = 4.0f;
const float distBwLightCol = 4.0f;

float innerCutOff = cos(radians(u_lightInnerCutOffAngle));
float outerCutOff = cos(radians(u_lightOuterCutOffAngle));


float calculateLightIntensity(vec3 shadowCoordinate, sampler2D shadowMap)
{
  if (shadowCoordinate.x < 0.0 || shadowCoordinate.x > 1.0 ||
      shadowCoordinate.y < 0.0 || shadowCoordinate.y > 1.0 ||
      shadowCoordinate.z > 1.0)
  {
    return 1.0;
  }

  if (shadowCoordinate.z < 0.0)
    return 0.0;

  float depth = texture(shadowMap, shadowCoordinate.xy).r;
  float currentDepth = shadowCoordinate.z;

  if (currentDepth > depth + 0.00008) // saves from shadow acne
  {
    return 0.0; // it is in shadow
  }
  return 1.0;  
}


vec3 PhongShading() 
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
    vec3 shadowCoordinate = (i_fragPosLightSpace[i].xyz / i_fragPosLightSpace[i].w) * 0.5 + 0.5;

    float lightIntensity = calculateLightIntensity(shadowCoordinate, u_ShadowMaps[i]); 

    if (lightIntensity == 1.0f)
    {
      new_lightPos.x = refX -  (distBwLightCol * (i / 3));
      new_lightPos.y = refY;
      new_lightPos.z = refZ - (distBwLightRow * (i % 3)); 

      // spot light [inner and outcone]
      vec3 lightDir = normalize(new_lightPos - i_fragPos); // both in world space
      float theta = dot(lightDir, normalize(-u_lightTargetDirection)); // now both point in same direction [towards light source]
      float epsilon = innerCutOff - outerCutOff;
      float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

      // attenuation
      float distance = length(i_fragPos - new_lightPos);
      float attenuation = 1.0 / (1.0 + (u_lightAttenLinear * distance) + (u_lightAttenQuad * distance * distance));
        

      // diffuse 
      vec3 norm = normalize(i_normals);
      float diff = max(dot(norm, lightDir), 0.0);
      diffuse += u_lightDiffuseStrength * diff * u_lightColor * attenuation * intensity;

      // specular 
      vec3 viewDir = normalize(u_viewPos - i_fragPos);
      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);
      specular += u_lightSpecularStrength * spec * u_lightColor * attenuation * intensity;
    }

  }

  vec3 result = (ambient + diffuse + specular);
  return result;
}

//void main()
//{
//vec3 shadowCoordinate = (i_fragPosLightSpace[6].xyz / i_fragPosLightSpace[8].w) * 0.5 + 0.5;
//vec3 shadowMapColor = texture(u_ShadowMaps[8], shadowCoordinate.xy).rrr;
//
//}

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
