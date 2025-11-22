#version 430 core

layout(location=0) in vec3 i_fragPos;
layout(location=1) in vec3 i_normals;
layout(location=2) in vec2 i_uv;
layout(location=3) in vec3 i_tangents;
layout(location=4) in vec3 i_bitangents;
layout(location=5) in vec3 i_gouraudShadingResult;
layout(location=6) in vec4 i_fragPosLightSpace[9];

out vec4 o_fragColor;

layout(binding=0) uniform sampler2D u_ShadowMaps[9];
layout(binding=9) uniform sampler2D u_texture;

uniform vec3 u_viewPos;
uniform int u_isPhong;
uniform vec3 u_color;

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
uniform vec3 u_dirLightPosition;
uniform vec2 u_poissionSamplingPoints[32];

const int numLights = 9;
const float distBwLightRow = 4.01f;
const float distBwLightCol = 4.1f;

float innerCutOff = cos(radians(u_lightInnerCutOffAngle));
float outerCutOff = cos(radians(u_lightOuterCutOffAngle));

float gTexelSizeWidth = 1.0 / 4096.0f;
float gTexelSizeHeight = 1.0 / 4096.0f;
vec2 gTexelSize = vec2(gTexelSizeWidth, gTexelSizeHeight);


float calculateLightIntensity(vec3 shadowCoordinate, sampler2D shadowMap, vec3 lightDir)
{
  if (shadowCoordinate.x < 0.0 || shadowCoordinate.x > 1.0 ||
      shadowCoordinate.y < 0.0 || shadowCoordinate.y > 1.0 ||
      shadowCoordinate.z > 1.0)
  {
    return 1.0;
  }

  if (shadowCoordinate.z < 0.0)
    return 0.0;

  int shadowSum = 0;
  float filterR = 4.0;
  vec2 spread = gTexelSize * filterR;

  for (int i = 0; i < 32; i++)
  {
    vec2 offSet = u_poissionSamplingPoints[i] * spread; 
    float depth = texture(shadowMap, shadowCoordinate.xy + offSet).r;
    float currentDepth = shadowCoordinate.z;

    if (currentDepth > depth + 0.0005) // saves from shadow acne
    {
      shadowSum += 1; // it is in shadow
    }
  }
 
  return (1.0 - (shadowSum / 32.0)); // So, if all are in shadow we return 0, means it have 0 light on it;
}


vec3 PhongShading(vec3 normals) 
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

    vec3 lightDir = normalize(new_lightPos - i_fragPos); // both in world space
                                                         // goes from frag to light source

    vec3 shadowCoordinate = (i_fragPosLightSpace[i].xyz / i_fragPosLightSpace[i].w) * 0.5 + 0.5;

    float lightIntensity = calculateLightIntensity(shadowCoordinate, u_ShadowMaps[i], lightDir); 

    {
      vec3 fragPositionInLightViewSpace = i_fragPosLightSpace[i].xyz / i_fragPosLightSpace[i].w;
      float innerSquare = 0.6f;
      float outerSquare = 1.0f;
      float epsilon = outerSquare - innerSquare;
      float currOffset = max(abs(fragPositionInLightViewSpace.x), abs(fragPositionInLightViewSpace.y));

      float blending = clamp((outerSquare - currOffset) / epsilon, 0.5, 1.0);

      // attenuation
      float distance = length(i_fragPos - new_lightPos);
      float attenuation = 1.0 / (1.0 + (0.09 * distance) + (0.032 * distance * distance));

      // diffuse 
      float diff = max(dot(normals, lightDir), 0.0);
      diffuse += 0.4 * u_lightDiffuseStrength * diff * u_lightColor * attenuation * blending * (lightIntensity + 0.01);

      // specular 
      vec3 viewDir = normalize(u_viewPos - i_fragPos);
      vec3 reflectDir = reflect(-lightDir, normals);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
      specular += 0.3 * u_lightSpecularStrength * spec * u_lightColor * attenuation * blending * lightIntensity;
    }
  }

  // DIRECTIONAL LIGHT
  {
    //vec3 lightDir = normalize(u_dirLightPosition - i_fragPos); // both in world space
    //float theta = dot(lightDir, normalize(-u_lightTargetDirection)); // now both point in same direction [towards light source]

    vec3 lightDir = normalize(-vec3(-1.0f, 0.5, 0.0));
    float r = 255.0 / 255.0;
    float g = 255.0 / 255.0;
    float b = 255.0 / 255.0;
    vec3 sunLightColor = vec3(r, g, b);

    float dist = 1.0 - (5.0 - i_fragPos.y); 
    float blend = clamp(dist, 0.0, 1.0);

    float distHorizontal = (15.0 + i_fragPos.x) / 14.0;
    float blend2 = clamp(distHorizontal, 0.1, 1.0);

    // diffuse 
    float diff = max(dot(normals, lightDir), 0.0);
    diffuse += blend2 * blend * 2.0 * u_lightDiffuseStrength * diff * sunLightColor;

    // specular 
    vec3 viewDir = normalize(u_viewPos - i_fragPos);
    vec3 reflectDir = reflect(-lightDir, normals);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    specular += blend2 * blend * 0.2 * u_lightSpecularStrength * spec * sunLightColor;
  }

  vec3 result = (ambient + diffuse + specular);
  return result;
}


void main() 
{
  float r = u_color.r / 255.0;
  float g = u_color.g / 255.0;
  float b = u_color.b / 255.0;
  o_fragColor = vec4(r, g, b, 1.0);
  vec3 result = vec3(0.0, 0.0, 0.0); 

  vec3 N = normalize(i_normals);
  vec3 T = normalize(i_tangents);
  T = normalize(T - dot(T, N) * N);
  vec3 B = cross(N, T);
  if (dot(cross(N, T), normalize(i_bitangents)) < 0.0) {
      B = B * -1.0;
  }

  vec3 bumpMapNormal = texture(u_texture, i_uv).xyz;
  bumpMapNormal = 2.0 * bumpMapNormal - vec3(1.0); // going from color space to normal space

  mat3 TBN = mat3(T, B, N);
  // transfrom from tangent space to world space
  vec3 newNormals = normalize(TBN * bumpMapNormal);

  if (u_isPhong == 1)
  {
    result = PhongShading(newNormals) * vec3(o_fragColor);  
  }
  else
  {
    result = i_gouraudShadingResult * vec3(o_fragColor);
  }

  o_fragColor = vec4(result, 1.0);
}
