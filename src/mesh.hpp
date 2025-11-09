#ifndef MESH_HEADER
#define MESH_HEADER

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include "../glm/ext/matrix_transform.hpp"

#include <vector>

struct Mesh3D
{
  GLuint mVertexArrayObject = 0;

  GLuint mPositionVertexBufferObject = 0;
  GLuint mUvVertexBufferObject = 0;
  GLuint mNormalVertexBufferObject = 0;

  GLuint mTextureObject = 0;
  
  std::vector<float> mVertexData; 
  std::vector<float> mUvData; 
  std::vector<float> mNormalData;

  glm::vec3 mOffset = glm::vec3(0.0f);
  GLfloat mRotate = 0.0f; // it will rotate long y-axis
  glm::vec3 mScale = glm::vec3(0.0f);

  const char* name = "";
  const char* mModelPath = "";
  const char* mTexturePath = "";
};
#endif
