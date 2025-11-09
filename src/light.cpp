#include "../glad/glad.h"
#include "../glm/ext/matrix_transform.hpp"
#include "../glm/ext/matrix_clip_space.hpp"

#include "light.hpp"

Light::Light()
{

}

Light::Light(glm::vec3 lightPos) : mPosition(lightPos)
{
  float attenuationLinear = 0.14f;
  float attenuationQuad = 0.07f;

  float mAmbientStrength = 0.2;
  float mDiffuseStrength = 0.7;
  float mSpecularStrength = 2.0; 

  glm::vec3 mTargetDirection = glm::vec3(0.0f, -1.0f, 0.0f);

  float innerCutOffAngle = 75.0f;
  float innerCutOffCosine = cos(glm::radians(innerCutOffAngle));
  float outerCutOffCosine = cos(glm::radians(90.0f));
}


glm::mat4 Light::mGetViewMatrix()
{
  glm::mat4 lightSpace = glm::lookAt(mPosition, mTargetDirection, mUpDirection);
  return lightSpace;
}


glm::mat4 Light::mGetProjectionMatrix()
{
  glm::mat4 projection = glm::perspective(glm::radians(2 * mInnerCutOffAngle), mShadowMap.mShadowMapWidth/mShadowMap.mShadowMapHeight, 0.1f, 100.0f);
  return projection;
}

void Light::mGenShadowMap(std::vector<Mesh3D> meshes)
{
  mShadowMap.SetLightPosition(mPosition);
  GLuint shaderID = mShader.mCreateGraphicsPipeline("shaders/shadow/vert.glsl", "shaders/shadow/frag.glsl");
  mShadowMap.SetGraphicsPipeline(shaderID);
  mShadowMap.CreateShadowMapFrameBufferObject();
  mShadowMap.CreateShadowMapTextureObject();
  mShadowMap.BindShadowMapFrameBufferTextureObject();

  glm::mat4 lightViewSpace = mGetViewMatrix();
  glm::mat4 lightProjectionSpace = mGetProjectionMatrix();

  mShadowMap.GenShadowMap(meshes, lightViewSpace, lightProjectionSpace);
}
