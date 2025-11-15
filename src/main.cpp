/*
  TO RUN:                 1.  g++ src/*.cpp glad/glad.c -o prog -I./glad/ -lGL -lglfw -ldl [from parent directory]
                          2.  ./prog


  TO NAVIGATE:            WASD           -> in XZ axis
                          Top Down arrow -> Y axis
                          Mouse


  TO UNDERSTAND THE CODE: Start from main function [at very bottom]               
*/


// Third Party Libraries
#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include "../glm/ext/matrix_transform.hpp"
#include "../glm/ext/matrix_clip_space.hpp"
#include "../glm/gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "../glm/gtx/string_cast.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Standard Libraries 
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>

// My libraries
#include "app.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "loadModel.hpp"
#include "shader.hpp"
#include "shadowMap.hpp"
#include "light.hpp"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBALS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
App gApp;
Grid gGrid;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBALS END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
  if(action != GLFW_REPEAT && action != GLFW_PRESS) return; 
  
  float cameraSpeed = gApp.mCameraSpeed * gApp.mDeltaTime;

  switch (key)
  {
    case GLFW_KEY_W:
      gApp.mCamera.moveForward(cameraSpeed);
      break;
  
    case GLFW_KEY_S:
      gApp.mCamera.moveBackward(cameraSpeed);
      break;

    case GLFW_KEY_D:
      gApp.mCamera.moveRight(cameraSpeed);
      break;

    case GLFW_KEY_A:
      gApp.mCamera.moveLeft(cameraSpeed);
      break;

    case GLFW_KEY_UP:
      gApp.mCamera.moveUp(cameraSpeed);
      break;

    case GLFW_KEY_DOWN:
      gApp.mCamera.moveDown(cameraSpeed);
      break;

    case GLFW_KEY_C:
      gApp.mIsPhong = !gApp.mIsPhong;
      break;
  }
}


// Handle mouse inputs
void cursorPosition_callback(GLFWwindow* window, double xPos, double yPos)
{
  gApp.mCamera.mouseLook(xPos, yPos);
}


// Getting things ready
void initialization(App* app) 
{ 
  if (!glfwInit()) return;

  app->mWindow = glfwCreateWindow(app->mScreenWidth, app->mScreenHeight, app->mTitle, NULL, NULL);

  if (!app->mWindow)
  {
      glfwTerminate();
      return;
  }
  glfwMakeContextCurrent(app->mWindow);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return;
  }

  glfwSetKeyCallback(app->mWindow, key_callback); 
  glfwSetInputMode(app->mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  
  glfwSetCursorPosCallback(app->mWindow, cursorPosition_callback);
}


void initializeGrid()
{
  glm::vec3 refCoordinate = glm::vec3(0.0f, 4.93f, 0.0f);

  for (int i = 0; i < gGrid.mROW; i++)
  {
    for (int j = 0; j < gGrid.mCOL; j++)
    {
       if (j == 0 || j == gGrid.mCOL - 1)
       {
         glm::vec3 tempCoordinate;
         tempCoordinate.x = refCoordinate.x - (float)(j * gGrid.mTileSizeX);
         tempCoordinate.y = refCoordinate.y;
         tempCoordinate.z = refCoordinate.z - (float)(i * gGrid.mTileSizeY);
         gGrid.mVertexDataH.push_back(tempCoordinate);
       }
    }
  }


  for (int j = 0; j < gGrid.mCOL; j++)
  {
    for (int i = 0; i < gGrid.mROW; i++)
    {
       if (i == 0 || i == gGrid.mROW - 1)
       {
         glm::vec3 tempCoordinate;
         tempCoordinate.x = refCoordinate.x - (float)(j * gGrid.mTileSizeX);
         tempCoordinate.y = refCoordinate.y;
         tempCoordinate.z = refCoordinate.z - (float)(i * gGrid.mTileSizeY);
         gGrid.mVertexDataV.push_back(tempCoordinate);
       }
    }
  }

  glGenVertexArrays(1, &gGrid.mVertexArrayObject);

  glGenBuffers(1, &gGrid.mVertexBufferObjectH);
  glGenBuffers(1, &gGrid.mVertexBufferObjectV);
  // we will fill the buffers in `DisplayGrid` function
  // as we are using 2 position vbo's for convinence
}


// Load object
bool meshCreate(const char* path, Mesh3D* mesh)
{
  std::vector<float> vertexData;
  std::vector<float> uvData;
  std::vector<float> normalData;

  if(loadObj(path, vertexData, uvData, normalData) == false)
  {
    std::cout << "Problem occured in loading model" << std::endl;
    return false;
  }

  mesh->mVertexData = vertexData;
  mesh->mUvData = uvData;
  mesh->mNormalData = normalData;

  return true;
}


// Load texture
bool loadTexture(const char* path, Mesh3D* mesh)
{
  glGenTextures(1, &mesh->mTextureObject);
  glBindTexture(GL_TEXTURE_2D, mesh->mTextureObject);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  stbi_set_flip_vertically_on_load(true); // This line fixed a bug which was so annoying  

  int width, height, nChannels;
  unsigned char* data = stbi_load(path, &width, &height, &nChannels, 0);
  

  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load texture, for mesh: " << mesh->name << std::endl;
    return false;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);

  return true;
}


// Sets up mesh data transfer from CPU to GPU 
void meshCTGdataTransfer(Mesh3D* mesh) 
{
  glGenVertexArrays(1, &mesh->mVertexArrayObject);
  glBindVertexArray(mesh->mVertexArrayObject);

  // 1. start generating our position VBO
  glGenBuffers(1, &mesh->mPositionVertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->mPositionVertexBufferObject);
  glBufferData(GL_ARRAY_BUFFER,
              mesh->mVertexData.size() * sizeof(float),
              mesh->mVertexData.data(),
              GL_STATIC_DRAW);


  //    Linking the position attrib in VAO
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 
                        3,
                        GL_FLOAT,
                        false,
                        0, 
                        (void*)0);

  // 2. start generating our uv VBO
  glGenBuffers(1, &mesh->mUvVertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->mUvVertexBufferObject);
  glBufferData(GL_ARRAY_BUFFER,
              mesh->mUvData.size() * sizeof(float),
              mesh->mUvData.data(),
              GL_STATIC_DRAW);

  //    Linking the uv attrib in VAO
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        2,
                        GL_FLOAT,
                        false,
                        0,
                        (void*)0);

  // 3. start generating our normals VBO
  glGenBuffers(1, &mesh->mNormalVertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->mNormalVertexBufferObject);
  glBufferData(GL_ARRAY_BUFFER,
              mesh->mNormalData.size() * sizeof(float),
              mesh->mNormalData.data(),
              GL_STATIC_DRAW);

  //    Linking the normal attrib in VAO
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2,
                        3,
                        GL_FLOAT,
                        false,
                        0,
                        (void*)0);
  glBindVertexArray(0);
  glDisableVertexAttribArray(0); 
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}


void Input(App* app)
{
  if(glfwGetKey(app->mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(app->mWindow, true);
}

void DisplayGrid(App* app)
{
  // Local to world
  GLint location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_model");
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
  glUniformMatrix4fv(location, 1, GL_FALSE, &model[0][0]);

  glBindVertexArray(gGrid.mVertexArrayObject);

  // 1 -> Drawing horizontal lines
  glBindBuffer(GL_ARRAY_BUFFER, gGrid.mVertexBufferObjectH);
  glBufferData(GL_ARRAY_BUFFER,
               gGrid.mVertexDataH.size() * sizeof(glm::vec3),
               gGrid.mVertexDataH.data(),
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        false,
                        0,
                        (void*)0);

  glDrawArrays(GL_LINES, 0, gGrid.mVertexDataH.size());

  // 2 -> Drawing vertical lines
  glBindBuffer(GL_ARRAY_BUFFER, gGrid.mVertexBufferObjectV);
  glBufferData(GL_ARRAY_BUFFER,
               gGrid.mVertexDataV.size() * sizeof(glm::vec3),
               gGrid.mVertexDataV.data(),
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        false,
                        0,
                        (void*)0);

  glDrawArrays(GL_LINES, 0, gGrid.mVertexDataV.size());

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableVertexAttribArray(0);
}


void LightInformation(App* app, GLuint graphicsPipeline)
{
  // LightPosition
  GLint location = glGetUniformLocation(graphicsPipeline, "u_lightPos");
  glUniform3f(location, app->mRefLightPos.x, app->mRefLightPos.y, app->mRefLightPos.z);

  // LightColor
  location = glGetUniformLocation(graphicsPipeline, "u_lightColor");
  glUniform3f(location, app->mLightColor.x, app->mLightColor.y, app->mLightColor.z);


  // projection view matrix of light 
  location = glGetUniformLocation(graphicsPipeline, "u_lightProjectionViewMatrix[0]");
  glUniformMatrix4fv(location, 9, GL_FALSE, glm::value_ptr(app->mLightProjectionViewMatrixCombined[0]));


  // attenuation constants
  location = glGetUniformLocation(graphicsPipeline, "u_lightAttenLinear");
  glUniform1f(location, app->mLights[0].attenuationLinear);
  
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightAttenQuad");
  glUniform1f(location, app->mLights[0].attenuationQuad);

  // traget direction
  location = glGetUniformLocation(graphicsPipeline, "u_lightTargetDirection");
  glUniform3f(location, app->mLights[0].mTargetDirection.x, app->mLights[0].mTargetDirection.y, app->mLights[0].mTargetDirection.z);


  // inner cone angle direction
  location = glGetUniformLocation(graphicsPipeline, "u_lightInnerCutOffAngle");
  glUniform1f(location, app->mLights[0].mInnerCutOffAngle);

  // outer cone angle direction
  location = glGetUniformLocation(graphicsPipeline, "u_lightOuterCutOffAngle");
  glUniform1f(location, app->mLights[0].mOuterCutOffAngle);
  
  // type strength [ambient, specular, diffuse]
  // Ambient
  location = glGetUniformLocation(graphicsPipeline, "u_lightAmbientStrength");
  glUniform1f(location, app->mLights[0].mAmbientStrength);

  // Diffuse
  location = glGetUniformLocation(graphicsPipeline, "u_lightDiffuseStrength");
  glUniform1f(location, app->mLights[0].mDiffuseStrength);

  // Specular
  location = glGetUniformLocation(graphicsPipeline, "u_lightSpecularStrength");
  glUniform1f(location, app->mLights[0].mSpecularStrength);
}


void PreDraw(App* app) 
{
  glEnable(GL_DEPTH_TEST);
  glCullFace(GL_BACK);

  glViewport(0, 0, app->mScreenWidth, app->mScreenHeight);
  glClearColor(1.f, 0.f, 0.f, 1.f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
}


void MeshTransformation(App* app, Mesh3D* mesh, GLuint graphicsPipeline)
{
  // Local to world
  GLint location = glGetUniformLocation(graphicsPipeline, "u_model");
  glm::mat4 model = glm::translate(glm::mat4(1.0f), mesh->mOffset);
  model = glm::rotate(model, glm::radians(mesh->mRotate), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::scale(model, mesh->mScale);
  glUniformMatrix4fv(location, 1, GL_FALSE, &model[0][0]);


  // World to camera
  location = glGetUniformLocation(graphicsPipeline, "u_view");
  glm::mat4 cameraSpace = app->mCamera.getViewMatrix(); 
  glUniformMatrix4fv(location, 1, GL_FALSE, &cameraSpace[0][0]);    


  // Real screen view
  location = glGetUniformLocation(graphicsPipeline, "u_projection");
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)app->mScreenWidth/app->mScreenHeight, 0.1f, 100.0f);
  glUniformMatrix4fv(location, 1, GL_FALSE, &projection[0][0]);


  // ViewPosition
  location = glGetUniformLocation(graphicsPipeline, "u_viewPos");
  glm::vec3 viewPos = app->mCamera.getViewPos();
  glUniform3f(location, viewPos.x, viewPos.y, viewPos.z);


  // toggleShading
  location = glGetUniformLocation(graphicsPipeline, "u_isPhong");
  glUniform1i(location, app->mIsPhong);

  // color
  location = glGetUniformLocation(graphicsPipeline, "u_color");
  glUniform3f(location, mesh->mColor.x, mesh->mColor.y, mesh->mColor.z);
}


void Draw(Mesh3D* mesh, App* app) 
{
  for (int i = 0; i < app->mLightsNumber; i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, app->mLights[i].mShadowMap.mTextureObject);    
  }

  glActiveTexture(GL_TEXTURE0 + app->mLightsNumber);
  glBindTexture(GL_TEXTURE_2D, mesh->mTextureObject);
  glBindVertexArray(mesh->mVertexArrayObject);
  glDrawArrays(GL_TRIANGLES, 0, mesh->mVertexData.size() / 3);
}


void mainLoop(App* app) 
{

  while (!glfwWindowShouldClose(app->mWindow))
  {
    // get fps
    float currentTime = glfwGetTime();
    app->mDeltaTime = currentTime - app->mLastFrame;
    app->mLastFrame = currentTime;
  
    Input(app);
    PreDraw(app);
    DisplayGrid(app);

    // 1. for simple meshes 
    GLuint currentGraphicsPipeline = app->mGraphicsPipelineShaderProgram;
    glUseProgram(currentGraphicsPipeline);
    LightInformation(app, currentGraphicsPipeline);

    for (auto& pair : gApp.meshes)
    {
      Mesh3D& mesh = pair.second;
      if (mesh.mGraphicsPipeline != 0) continue;
      MeshTransformation(app, &mesh, currentGraphicsPipeline);
      Draw(&mesh, app);
    }

    // 2. for normal meshes [walls and ceilings]
    currentGraphicsPipeline = app->mNormalsGraphicsPipelineShaderProgram;
    glUseProgram(currentGraphicsPipeline);
    LightInformation(app, currentGraphicsPipeline);

    for (auto& pair : gApp.meshes)
    {
      Mesh3D& mesh = pair.second;
      if (mesh.mGraphicsPipeline == 0) continue;
      MeshTransformation(app, &mesh, currentGraphicsPipeline);
      Draw(&mesh, app);
    }

    // Update the screen
    glfwPollEvents(); 
    glfwSwapBuffers(app->mWindow);
  }
}


void cleanUp() 
{
  glfwTerminate();
  return;
}

void ObjectCreation(const char* name,
                    glm::vec3 scale,
                    glm::vec3 offset,
                    GLfloat rotate,
                    const char* modelPath,
                    const char* texturePath = "",
                    GLuint graphicsPipeline = 0,
                    glm::vec3 color = glm::vec3(0.0))
{
  Mesh3D mesh;
  
  mesh.name = name; 
  mesh.mScale = scale;
  mesh.mOffset = offset;
  mesh.mRotate = rotate;
  mesh.mModelPath = modelPath;
  mesh.mTexturePath = texturePath;
  mesh.mGraphicsPipeline = graphicsPipeline;
  mesh.mColor = color;

  //meshes.push_back(mesh);
  gApp.meshes[name] = mesh;
}


void ObjectFilling()
{
  for (auto& pair : gApp.meshes) {
    Mesh3D& mesh = pair.second;
    if(!meshCreate(mesh.mModelPath, &mesh))       // Loading position, UV, normals for vertices
    {
      std::cout << "Failed to load model for " << mesh.name << std::endl;
    };

    if(strcmp(mesh.mTexturePath, "") != 0)
    {
      if (!loadTexture(mesh.mTexturePath, &mesh)) // Loading texture for object [if avaliable] 
      {
        std::cout << "Failed to load texture for " << mesh.name << std::endl;
      }
    } 
    else std::cout << "No texture allocated for " << mesh.name << std::endl;

    meshCTGdataTransfer(&mesh);
  }
}


void BenchPlacement()
{
  Mesh3D refBench = gApp.meshes.at("Bench");
  gApp.meshes.erase("Bench");

  float distbwBenchRow = 1.57f;
  float distbwBenchCol = 3.25f;

  float refX = refBench.mOffset.x;
  float refY = refBench.mOffset.y;
  float refZ = refBench.mOffset.z;

  for (int i = 0; i < 25; i++)
  {
    // exceptions !! 
    if (i == 0 || i == 5) continue; // these benches are no there in class
    if (i / 5 == 2) // Column 2 and column 3 have less spacing then other
      refX = 0.7f;

    float newX = refX - (distbwBenchCol * (i / 5));
    float newY = refY;
    float newZ = refZ - (distbwBenchRow * (i % 5));
    
    refBench.mOffset = glm::vec3(newX, newY, newZ);
    gApp.meshes["Bench " + std::to_string(i)] = refBench;
  }
}


void LightPlacement()
{
  Mesh3D refLight = gApp.meshes.at("Light"); 

  float distbwLightRow = 4.0f;
  float distbwLightCol = 4.0f;

  float refX = refLight.mOffset.x;
  float refY = refLight.mOffset.y;
  float refZ = refLight.mOffset.z;

  for (int i = 1; i < 9; i++)
  {
    float newX = refX - (distbwLightCol * (i / 3));
    float newY = refY;
    float newZ = refZ - (distbwLightRow * (i % 3));
    
    refLight.mOffset = glm::vec3(newX, newY, newZ);
    gApp.meshes["Light " + std::to_string(i)] = refLight;
  }
}

void TilePlacement()
{
  Mesh3D refTile = gApp.meshes.at("Tile"); 

  float distbwTileRow = 1.0f;
  float distbwTileCol = 1.0f;

  float refX = refTile.mOffset.x;
  float refY = refTile.mOffset.y;
  float refZ = refTile.mOffset.z;

  for (int i = 1; i < 150; i++)
  {
    float newX = refX - (distbwTileCol * (i / 10));
    float newY = refY;
    float newZ = refZ - (distbwTileRow * (i % 10));
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    gApp.meshes["Tile " + std::to_string(i)] = refTile;
  }
}

void SideTilePlacement()
{
  Mesh3D refTile = gApp.meshes.at("Tile Side"); 
  gApp.meshes.erase("Tile Side");

  float distbwTileRow = 1.0f;
  float distbwTileCol = 1.0f;

  float refX = refTile.mOffset.x;
  float refY = refTile.mOffset.y;
  float refZ = refTile.mOffset.z;
  float refR = refTile.mRotate;
  int count = 0;

  for (int i = 0; i < 10; i++)
  {
    if (i < 2)  continue; // door is there
    float newX = refX;
    float newY = refY;
    float newZ = refZ - (distbwTileRow * (i % 10));
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    gApp.meshes["Tile Side " + std::to_string(count++)] = refTile;
  }

  for (int i = 0; i < 15; i++)
  {
    float newX = refX - (distbwTileCol * (i % 15));
    float newY = refY;
    float newZ = refZ - 0.005f;
    float newR = refR +  90.0f;
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    refTile.mRotate = newR;
    gApp.meshes["Tile Side " + std::to_string(count++)] = refTile;
  }

  for (int i = 0; i < 15; i++)
  {
    float newX = (refX - (distbwTileCol * (i % 15))) - 1.005f;
    float newY = refY;
    float newZ = refZ - 10.0f;
    float newR = refR -  90.0f;
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    refTile.mRotate = newR;
    gApp.meshes["Tile Side " + std::to_string(count++)] = refTile;
  }

  for (int i = 0; i < 10; i++)
  {
    float newX = refX - 15.0f;
    float newY = refY;
    float newZ = (refZ - (distbwTileRow * (i % 10))) - 1.005f;
    float newR = refR -  180.0f;
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    refTile.mRotate = newR;
    gApp.meshes["Tile Side " + std::to_string(count++)] = refTile;
  }
}

void CeilingPlacement()
{
  Mesh3D refTile = gApp.meshes.at("Ceiling"); 

  float distbwTileRow = 1.01f;
  float distbwTileCol = 1.03f;

  float refX = refTile.mOffset.x;
  float refY = refTile.mOffset.y;
  float refZ = refTile.mOffset.z;

  for (int i = 1; i < 150; i++)
  {
    float newX = refX - (distbwTileCol * (i / 10));
    float newY = refY;
    float newZ = refZ - (distbwTileRow * (i % 10));
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    gApp.meshes["Ceiling " + std::to_string(i)] = refTile;
  }
}

void initializeObjects()
{
  ObjectCreation("Bench", 
                 glm::vec3(0.077f, 0.07f, 0.06f),
                 glm::vec3(-0.008f, 0.0f, -2.2f),
                 0.0f,
                 "Models/bench_1.obj",
                 "Models/textures/combinedBenchTexture.png");

  ObjectCreation("Board", 
                 glm::vec3(0.07f, 0.07f, 0.07f),
                 glm::vec3(-4.0f, 1.2f, 0.0f),
                 180.0f,
                 "Models/board_shaded.obj",
                 "Models/textures/board/board_combined_texture_1.jpeg");

  ObjectCreation("Ceiling", 
                 glm::vec3(0.083f, 0.02f, 0.080f),
                 glm::vec3(0.1f, 4.93f, 0.0f),
                 180.0f,
                 "Models/ceiling.obj",
                 "",
                 gApp.mNormalsGraphicsPipelineShaderProgram,
                 glm::vec3(171.0f, 171.0f, 196.0f));

  ObjectCreation("Clock", 
                 glm::vec3(0.31f, 0.31f, 0.25f),
                 glm::vec3(-15.0f, 4.4f, -5.2f),
                 90.0f,
                 "Models/clock.obj",
                 "Models/textures/clock/combined_texture_clock.jpeg");

  ObjectCreation("Door", 
                 glm::vec3(0.07f, 0.07f, 0.06f),
                 glm::vec3(-0.922f, 0.0f, -1.708f),
                 300.0f,
                 "Models/door_shaded.obj",
                 "Models/textures/door/combined_texture.jpeg");

  ObjectCreation("Door Frame", 
                 glm::vec3(0.068f, 0.07f, 0.05f),
                 glm::vec3(-0.05f, 0.0f, -1.87f),
                 270.0f,
                 "Models/door_frame.obj",
                 "Models/textures/door_frame/combined_texture_door_frame_2.jpeg");

  ObjectCreation("Light", 
                 glm::vec3(0.078f, 0.02f, 0.078f),
                 glm::vec3(-4.0f, 4.93f, -2.0f),
                 0.0f,
                 "Models/light.obj",
                 "Models/textures/light/texture.png");


  ObjectCreation("Podium", 
                 glm::vec3(0.16f, 0.16f, 0.16f),
                 glm::vec3(-2.2f, 0.0f, -1.05f),
                 180.0f,
                 "Models/podium_shaded.obj",
                 "Models/textures/podium/podium_combined_texture_2.jpeg");

  ObjectCreation("Projector Screen", 
                 glm::vec3(0.07f, 0.07f, 0.07f),
                 glm::vec3(-4.0f, 1.2f, 0.0f),
                 180.0f,
                 "Models/projector_screen_1.obj",
                 "Models/textures/projector_screen/combined_projector_screen_texture.jpeg");

  ObjectCreation("Remote", 
                 glm::vec3(0.016f, 0.018f, 0.016f),
                 glm::vec3(-3.86f, 2.27f, 0.005f),
                 180.0f,
                 "Models/remote_1_shaded.obj",
                 "Models/textures/remote/remote_1_texture.jpeg");

  ObjectCreation("Switch 1", 
                 glm::vec3(0.085f, 0.075f, 0.085f),
                 glm::vec3(0.0f, 0.7f, -3.6f),
                 270.0f,
                 "Models/switch_1.obj",
                 "Models/textures/switch/combined_projector_screen_texture.jpeg");

  ObjectCreation("Switch 2", 
                 glm::vec3(0.6f, 0.6f, 0.7f),
                 glm::vec3(-3.4f, 0.7f, 0.0f),
                 180.0f,
                 "Models/switch_2.obj",
                 "Models/textures/switch/combined_projector_screen_texture.jpeg");

  ObjectCreation("Switch 3", 
                 glm::vec3(0.6f, 0.6f, 0.7f),
                 glm::vec3(-12.5f, 0.7f, 0.0f),
                 180.0f,
                 "Models/switch_2.obj",
                 "Models/textures/switch/combined_projector_screen_texture.jpeg");

  ObjectCreation("Switch 4", 
                 glm::vec3(0.4f, 0.36f, 0.4f),
                 glm::vec3(0.0f, 2.0f, -3.2f),
                 270.0f,
                 "Models/switch_3.obj",
                 "Models/textures/wire_cover/white_bluish.png");


  ObjectCreation("Table", 
                 glm::vec3(0.07f, 0.07f, 0.07f),
                 glm::vec3(-5.6f, 0.0f, -3.6f),
                 0.0f,
                 "Models/table_shaded.obj",
                 "Models/textures/table/table_combined_texture_new_new.jpeg");

  ObjectCreation("Tile", 
                 glm::vec3(0.078f, 0.02f, 0.078f),
                 glm::vec3(0.0f, 0.0f, 0.0f),
                 180.0f,
                 "Models/tile.obj",
                 "Models/textures/tile/tile_texture_combined_1.jpeg");

  ObjectCreation("Tile Side", 
                 glm::vec3(0.078f, 0.078f, 0.078f),
                 glm::vec3(0.0f, 0.0f, 0.0f),
                 180.0f,
                 "Models/side_tile.obj",
                 "Models/textures/tile/tile_texture_combined_1.jpeg");

  ObjectCreation("Wall Back", 
                 glm::vec3(0.07f, 0.07f, 0.07f),
                 glm::vec3(-15.0f, 0.0f, -10.18f),
                 0.0f,
                 "Models/wall_back.obj",
                 "",
                 gApp.mNormalsGraphicsPipelineShaderProgram,
                 glm::vec3(230.0f, 226.0f, 209.0f));

  ObjectCreation("Wall Front", 
                 glm::vec3(0.07f, 0.07f, 0.07f),
                 glm::vec3(-4.0f, 1.2f, -0.01f),
                 180.0f,
                 "Models/wall_front.obj",
                 "",
                 gApp.mNormalsGraphicsPipelineShaderProgram,
                 glm::vec3(230.0f, 226.0f, 209.0f));

  ObjectCreation("Wall Left", 
                 glm::vec3(0.07f, 0.07f, 0.07f),
                 glm::vec3(0.0f, 0.0f, 0.01f),
                 90.0f,
                 "Models/wall_left.obj",
                 "",
                 gApp.mNormalsGraphicsPipelineShaderProgram,
                 glm::vec3(230.0f, 226.0f, 209.0f));

  ObjectCreation("Wall Right", 
                 glm::vec3(0.07f, 0.07f, 0.07f),
                 glm::vec3(-15.337f, 0.0f, 0.01f),
                 90.0f,
                 "Models/wall_right.obj",
                 "",
                 gApp.mNormalsGraphicsPipelineShaderProgram,
                 glm::vec3(230.0f, 226.0f, 209.0f));


  ObjectCreation("Window Panel 1", 
                 glm::vec3(0.066f, 0.06f, 0.06f),
                 glm::vec3(0.15f, 4.2f, -8.2f),
                 270.0f,
                 "Models/window_panel_1_shaded.obj",
                 "Models/textures/window/combined_window_texture.jpeg");

  ObjectCreation("Window Panel 2", 
                 glm::vec3(0.0618f, 0.06f, 0.06f),
                 glm::vec3(-15.05f, 4.2f, -10.11f),
                 0.0f,
                 "Models/window_panel_2.obj",
                 "Models/textures/window/combined_window_texture.jpeg");

  ObjectCreation("Window Panel 3", 
                 glm::vec3(0.0605f, 0.06f, 0.06f),
                 glm::vec3(0.0f, 4.2f, 0.06f),
                 90.0f,
                 "Models/window_panel_3.obj",
                 "Models/textures/window/combined_window_texture.jpeg");

  ObjectCreation("Window Panel 4", 
                 glm::vec3(0.069f, 0.06f, 0.06f),
                 glm::vec3(0.15f, 4.2f, -10.15f),
                 270.0f,
                 "Models/window_panel_4.obj",
                 "Models/textures/window/combined_window_texture.jpeg");

  ObjectCreation("Wire Cover 1", 
                 glm::vec3(0.6f, 66.0f, 0.6f),
                 glm::vec3(-3.4f, -11.65f, 0.0f),
                 180.0f,
                 "Models/wire_cover.obj",
                 "Models/textures/wire_cover/white_bluish.png");

  ObjectCreation("Wire Cover 2", 
                 glm::vec3(148.0f, 0.7f, 0.5f),
                 glm::vec3(45.3f, 0.675f, 0.0f),
                 180.0f,
                 "Models/wire_cover_2.obj",
                 "Models/textures/wire_cover/white_bluish.png");
}

int main()
{
  initialization(&gApp);
  initializeGrid();
  
  // Pipline
  Shader shader;
  gApp.mGraphicsPipelineShaderProgram =  shader.mCreateGraphicsPipeline("shaders/vert.glsl", "shaders/frag.glsl");
  gApp.mNormalsGraphicsPipelineShaderProgram = shader.mCreateGraphicsPipeline("shaders/normals/vert.glsl", "shaders/normals/frag.glsl");

  // Objects
  initializeObjects();
  ObjectFilling();
  BenchPlacement();
  SideTilePlacement();
  LightPlacement();
  TilePlacement();
  CeilingPlacement(); 

  // Lights
  glm::vec3 tempLightPos;
  for (int i = 0; i < gApp.mLightsNumber; i++)
  {
    tempLightPos.x = gApp.mRefLightPos.x - (gApp.mDistBwLightCol * (i / 3));
    tempLightPos.y = gApp.mRefLightPos.y;
    tempLightPos.z = gApp.mRefLightPos.z - (gApp.mDistBwLightRow * (i % 3));

    gApp.mLights[i].mPosition = tempLightPos;
    gApp.mLights[i].mGenShadowMap(gApp.meshes);
    gApp.mLightProjectionViewMatrixCombined[i] = gApp.mLights[i].mGetProjectionMatrix() * gApp.mLights[i].mGetViewMatrix();
  }

  mainLoop(&gApp);
  cleanUp();

  return 0;
}
