#include "../glad/glad.h"
#include "../glm/ext/matrix_transform.hpp"
#include "../glm/ext/matrix_clip_space.hpp"

#include <string>
#include <map>

#include "shadowMap.hpp"
#include "mesh.hpp"


void ShadowMap::SetLightPosition(glm::vec3 lightPos)
{
  mLightPos.x = lightPos.x;
  mLightPos.y = lightPos.y;
  mLightPos.z = lightPos.z;
}


void ShadowMap::SetGraphicsPipeline(GLuint shaderID)
{
  mGraphicsPipelineShaderProgram = shaderID; 
}


void ShadowMap::CreateShadowMapFrameBufferObject()
{
  glGenFramebuffers(1, &mFrameBufferObject);
  glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);
}


void ShadowMap::CreateShadowMapTextureObject()
{
  glGenTextures(1, &mTextureObject);
  glBindTexture(GL_TEXTURE_2D, mTextureObject);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mShadowMapWidth, mShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}


void ShadowMap::BindShadowMapFrameBufferTextureObject()
{
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mTextureObject, 0);
  glDrawBuffer(GL_NONE); // we don't need color buffer
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
} 


void ShadowMap::GenShadowMap(const std::map<std::string, Mesh3D>& meshes, glm::mat4 lightViewMatrix, glm::mat4 lightProjectionMatrix)
{
  glEnable(GL_DEPTH_TEST);  
  glCullFace(GL_FRONT);

  glViewport(0, 0, mShadowMapWidth, mShadowMapHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);
  glClear(GL_DEPTH_BUFFER_BIT);

  glUseProgram(mGraphicsPipelineShaderProgram);

  ShadowMap::RenderOnFrameBuffer(meshes, lightViewMatrix, lightProjectionMatrix);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void ShadowMap::RenderOnFrameBuffer(const std::map<std::string, Mesh3D>& meshes, glm::mat4 lightViewMatrix, glm::mat4 lightProjectionMatrix)
{
  for (const auto& pair: meshes)
  {
    Mesh3D mesh = pair.second;
    ShadowMap::TransformObjects_N_SendUniformData(&mesh, lightViewMatrix, lightProjectionMatrix);  
    glBindVertexArray(mesh.mVertexArrayObject);
    glDrawArrays(GL_TRIANGLES, 0, mesh.mVertexData.size() / 3);
  }
  glBindVertexArray(0);
}


void ShadowMap::TransformObjects_N_SendUniformData(Mesh3D* mesh, glm::mat4 lightViewMatrix, glm::mat4 lightProjectionMatrix)
{
  // Local to world
  GLint location = glGetUniformLocation(mGraphicsPipelineShaderProgram, "u_model");
  glm::mat4 model = glm::translate(glm::mat4(1.0f), mesh->mOffset);
  model = glm::rotate(model, glm::radians(mesh->mRotate), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::scale(model, mesh->mScale);
  glUniformMatrix4fv(location, 1, GL_FALSE, &model[0][0]);


  // World to LIGHT
  location = glGetUniformLocation(mGraphicsPipelineShaderProgram, "u_view");
  glUniformMatrix4fv(location, 1, GL_FALSE, &lightViewMatrix[0][0]);    


  // Light view 
  location = glGetUniformLocation(mGraphicsPipelineShaderProgram, "u_projection");
  glUniformMatrix4fv(location, 1, GL_FALSE, &lightProjectionMatrix[0][0]);
}
