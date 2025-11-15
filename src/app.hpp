#ifndef OBJECTS_HEADER
#define OBJECTS_HEADER

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include "../glm/ext/matrix_transform.hpp"

#include <vector>
#include <map>

#include "camera.hpp"
#include "light.hpp"
#include "mesh.hpp"

struct App
{
  int mScreenWidth = 1080;
  int mScreenHeight = 720;
  int mIsPhong = 0;
  const char* mTitle = "CL-3";
  GLfloat mCameraSpeed = 10.0f;
  GLfloat mDeltaTime = 0;
  GLfloat mLastFrame = glfwGetTime();

  GLFWwindow * mWindow = nullptr;
  GLuint mGraphicsPipelineShaderProgram = 0;
  GLuint mNormalsGraphicsPipelineShaderProgram = 0;

  Light mLights[9];
  int mLightsNumber = 9;
  glm::vec3 mRefLightPos = glm::vec3(-3.5f, 4.93f, -1.5f);
  glm::vec3 mLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::mat4 mLightProjectionViewMatrixCombined[9];
  float mDistBwLightRow = 4.0f;
  float mDistBwLightCol = 4.0f;

  Camera mCamera;
  std::map<std::string, Mesh3D> meshes;
};

struct Grid
{
  int mROW = 11;
  int mCOL = 16;
  float mTileSizeX = 1.0f;
  float mTileSizeY = 1.0f;

  GLuint mVertexArrayObject = 0; 
  GLuint mVertexBufferObjectH = 0;
  GLuint mVertexBufferObjectV = 0;

  std::vector<glm::vec3> mVertexDataH; // data for horizontal points
  std::vector<glm::vec3> mVertexDataV;
};
#endif
