#ifndef SHADOW_MAP_HEADER
#define SHADOW_MAP_HEADER

#include "../glad/glad.h"
#include "../glm/ext/matrix_transform.hpp"

#include <vector>

#include "mesh.hpp"

class ShadowMap
{
  private:
    GLuint mFrameBufferObject = 0;
    GLuint mTextureObject = 0;
    GLuint mGraphicsPipelineShaderProgram = 0;

    void RenderOnFrameBuffer(std::vector<Mesh3D> meshes, glm::mat4, glm::mat4);
    void TransformObjects_N_SendUniformData(Mesh3D*, glm::mat4, glm::mat4);
 
 public:
   float mShadowMapWidth = 2048.0f;
   float mShadowMapHeight = 2048.0f;
   glm::vec3 mLightPos;
   void SetLightPosition(glm::vec3);
   void SetGraphicsPipeline(GLuint);
   void CreateShadowMapFrameBufferObject();
   void CreateShadowMapTextureObject();
   void BindShadowMapFrameBufferTextureObject();
   void GenShadowMap(std::vector<Mesh3D> meshes, glm::mat4, glm::mat4);

};
#endif
