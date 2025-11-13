#include "../glad/glad.h"
#include "../glm/ext/matrix_transform.hpp"
#include "../glm/ext/matrix_clip_space.hpp"

#include "light.hpp"

Light::Light()
{
  attenuationLinear = 0.14f;
  attenuationQuad = 0.07f;

  mAmbientStrength = 0.6;
  mDiffuseStrength = 0.7;
  mSpecularStrength = 0.7; 

  mTargetDirection = glm::vec3(0.0f, -1.0f, 0.0f);
  mUpDirection = glm::vec3(0.0f, 0.0f, 1.0f);

  mInnerCutOffAngle = 50.0f;
  mOuterCutOffAngle = 60.0f;
  mInnerCutOffCosine = cos(glm::radians(mInnerCutOffAngle));
  mOuterCutOffCosine = cos(glm::radians(mOuterCutOffAngle));
}

Light::Light(glm::vec3 lightPos) : mPosition(lightPos)
{

}


glm::mat4 Light::mGetViewMatrix()
{
  glm::mat4 lightSpace = glm::lookAt(mPosition, mPosition + mTargetDirection, mUpDirection);
  return lightSpace;
}


glm::mat4 Light::mGetProjectionMatrix()
{
  glm::mat4 projection = glm::perspective(glm::radians(2 * mOuterCutOffAngle),(float)mShadowMap.mShadowMapWidth/mShadowMap.mShadowMapHeight, 0.1f, 100.0f);
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
