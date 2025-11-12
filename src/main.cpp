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
         tempCoordinate.x = refCoordinate.x - (float)(j * gGrid.mTileSize);
         tempCoordinate.y = refCoordinate.y;
         tempCoordinate.z = refCoordinate.z - (float)(i * gGrid.mTileSize);
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
         tempCoordinate.x = refCoordinate.x - (float)(j * gGrid.mTileSize);
         tempCoordinate.y = refCoordinate.y;
         tempCoordinate.z = refCoordinate.z - (float)(i * gGrid.mTileSize);
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


void LightInformation(App* app)
{
  // LightPosition
  GLint location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightPos");
  glUniform3f(location, app->mRefLightPos.x, app->mRefLightPos.y, app->mRefLightPos.z);

  // LightColor
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightColor");
  glUniform3f(location, app->mLightColor.x, app->mLightColor.y, app->mLightColor.z);


  // projection view matrix of light 
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightProjectionViewMatrix[0]");
  glUniformMatrix4fv(location, 9, GL_FALSE, glm::value_ptr(app->mLightProjectionViewMatrixCombined[0]));


  // attenuation constants
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightAttenLinear");
  glUniform1f(location, app->mLights[0].attenuationLinear);
  
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightAttenQuad");
  glUniform1f(location, app->mLights[0].attenuationQuad);

  // traget direction
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightTargetDirection");
  glUniform3f(location, app->mLights[0].mTargetDirection.x, app->mLights[0].mTargetDirection.y, app->mLights[0].mTargetDirection.z);


  // cone angle direction
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightInnerCutOffAngle");
  glUniform1f(location, app->mLights[0].mInnerCutOffAngle);

  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightOuterCutOffAngle");
  glUniform1f(location, app->mLights[0].mOuterCutOffAngle);
  
  // type strength [ambient, specular, diffuse]
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightAmbientStrength");
  glUniform1f(location, app->mLights[0].mAmbientStrength);

  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightDiffuseStrength");
  glUniform1f(location, app->mLights[0].mDiffuseStrength);

  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightSpecularStrength");
  glUniform1f(location, app->mLights[0].mSpecularStrength);
}


void PreDraw(App* app) 
{
  glEnable(GL_DEPTH_TEST);
  glCullFace(GL_BACK);

  glViewport(0, 0, app->mScreenWidth, app->mScreenHeight);
  glClearColor(1.f, 0.f, 0.f, 1.f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

  glUseProgram(app->mGraphicsPipelineShaderProgram);
}



void MeshTransformation(App* app, Mesh3D* mesh)
{
  // Local to world
  GLint location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_model");
  glm::mat4 model = glm::translate(glm::mat4(1.0f), mesh->mOffset);
  model = glm::rotate(model, glm::radians(mesh->mRotate), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::scale(model, mesh->mScale);
  glUniformMatrix4fv(location, 1, GL_FALSE, &model[0][0]);


  // World to camera
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_view");
  glm::mat4 cameraSpace = app->mCamera.getViewMatrix(); 
  glUniformMatrix4fv(location, 1, GL_FALSE, &cameraSpace[0][0]);    


  // Real screen view
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_projection");
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)app->mScreenWidth/app->mScreenHeight, 0.1f, 100.0f);
  glUniformMatrix4fv(location, 1, GL_FALSE, &projection[0][0]);


  // ViewPosition
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_viewPos");
  glm::vec3 viewPos = app->mCamera.getViewPos();
  glUniform3f(location, viewPos.x, viewPos.y, viewPos.z);


  // toggleShading
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_isPhong");
  glUniform1i(location, app->mIsPhong);


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


void mainLoop(App* app, std::vector<Mesh3D> meshes) 
{

  while (!glfwWindowShouldClose(app->mWindow))
  {
    // get fps
    float currentTime = glfwGetTime();
    app->mDeltaTime = currentTime - app->mLastFrame;
    app->mLastFrame = currentTime;
  

    Input(app);

    PreDraw(app);

    LightInformation(app);
    DisplayGrid(app);
    for (Mesh3D mesh : meshes)
    {
      MeshTransformation(app, &mesh);
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



void ObjectCreation(std::vector<Mesh3D>& meshes)
{
  Mesh3D bench;
  Mesh3D podium;
  Mesh3D table;
  Mesh3D door;
  Mesh3D light;
  Mesh3D board;
  Mesh3D tile;
  Mesh3D sideTile;
  Mesh3D remote1;
  Mesh3D winPanel1;
  Mesh3D winPanel2;
  Mesh3D winPanel3;
  Mesh3D winPanel4;
  Mesh3D doorFrame;

  bench.name = "Bench";
  bench.mScale = glm::vec3(0.076f, 0.07f, 0.057f);
  bench.mOffset = glm::vec3(-0.6f, 0.128f, -2.6f);
  bench.mModelPath = "Models/BenchTextured.obj";
  bench.mTexturePath = "Models/textures/combinedBenchTexture.png";

  podium.name = "Podium";
  podium.mScale = glm::vec3(0.16f, 0.16f, 0.16f);
  podium.mOffset = glm::vec3(-2.2f, 0.0f, -1.05f);
  podium.mRotate = 180.0f;
  podium.mModelPath = "Models/podium_shaded.obj";
  podium.mTexturePath = "Models/textures/podium/podium_combined_texture_2.jpeg";

  table.name = "Table";
  table.mScale = glm::vec3(0.07f, 0.07f, 0.07f);
  table.mOffset = glm::vec3(-5.6f, 0.0f, -3.6f);
  table.mModelPath = "Models/table_shaded.obj";
  table.mTexturePath = "Models/textures/table/table_combined_texture_new_new.jpeg";

  door.name = "Door";
  door.mScale = glm::vec3(0.07f, 0.07f, 0.06f);
  door.mOffset = glm::vec3(-0.922f, 0.0f, -1.708f);
  door.mRotate = 300.0f;
  door.mModelPath = "Models/door_shaded.obj";
  door.mTexturePath = "Models/textures/door/combined_texture.jpeg";

  doorFrame.name = "Door-Frame";
  doorFrame.mScale = glm::vec3(0.07f, 0.07f, 0.06f);
  doorFrame.mOffset = glm::vec3(0.05f, 0.0f, -1.93f);
  doorFrame.mRotate = 270.0f;
  doorFrame.mModelPath = "Models/door_frame.obj";
  doorFrame.mTexturePath = "Models/textures/door_frame/combined_texture_door_frame_2.jpeg";

  light.name = "Light";
  light.mScale = glm::vec3(0.078f, 0.02f, 0.078f);
  light.mOffset = glm::vec3(-4.0f, 4.93f, -2.0f);
  light.mModelPath = "Models/light.obj";
  light.mTexturePath = "Models/textures/light/texture.png";

  board.name = "Board";
  board.mScale = glm::vec3(0.07f, 0.07f, 0.07f);
  board.mOffset = glm::vec3(-4.0f, 1.2f, 0.0f);
  board.mRotate = 180.0f;
  board.mModelPath = "Models/board_shaded.obj";
  board.mTexturePath = "Models/textures/board/board_combined_texture_1.jpeg";

  tile.name = "Tile";
  tile.mScale = glm::vec3(0.078f, 0.02f, 0.078f);
  tile.mOffset = glm::vec3(0.0f, 0.0f, 0.0f);
  tile.mRotate = 180.0f;
  tile.mModelPath = "Models/tile.obj";
  tile.mTexturePath = "Models/textures/tile/tile_texture_combined_1.jpeg";

  sideTile.name = "Side-Tile";
  sideTile.mScale = glm::vec3(0.078f, 0.078f, 0.078f);
  sideTile.mOffset = glm::vec3(0.0f, 0.0f, 0.0f);
  sideTile.mRotate = 180.0f;
  sideTile.mModelPath = "Models/side_tile.obj";
  sideTile.mTexturePath = "Models/textures/tile/tile_texture_combined_1.jpeg";

  remote1.name = "Remote 1";
  remote1.mScale = glm::vec3(0.016f, 0.018f, 0.016f);
  remote1.mOffset = glm::vec3(-3.82f, 2.2f, 0.005f);
  remote1.mRotate = 180.0f;
  remote1.mModelPath = "Models/remote_1_shaded.obj";
  remote1.mTexturePath = "Models/textures/remote/remote_1_texture.jpeg";

  winPanel1.name = "Window Panel 1";
  winPanel1.mScale = glm::vec3(0.06f, 0.06f, 0.06f);
  winPanel1.mOffset = glm::vec3(0.15f, 4.2f, -8.2f);
  winPanel1.mRotate = 270.0f;
  winPanel1.mModelPath = "Models/window_panel_1_shaded.obj";
  winPanel1.mTexturePath = "Models/textures/window/combined_window_texture.jpeg";

  winPanel2.name = "Window Panel 2";
  winPanel2.mScale = glm::vec3(0.06f, 0.06f, 0.06f);
  winPanel2.mOffset = glm::vec3(-14.7f, 4.2f, -10.15f);
  winPanel2.mRotate = 0.0f;
  winPanel2.mModelPath = "Models/window_panel_2.obj";
  winPanel2.mTexturePath = "Models/textures/window/combined_texture_window.jpeg";

  winPanel3.name = "Window Panel 3";
  winPanel3.mScale = glm::vec3(0.06f, 0.06f, 0.06f);
  winPanel3.mOffset = glm::vec3(0.0f, 4.2f, 0.0f);
  winPanel3.mRotate = 90.0f;
  winPanel3.mModelPath = "Models/window_panel_3.obj";
  winPanel3.mTexturePath = "Models/textures/window/combined_texture_window.jpeg";

  winPanel4.name = "Window Panel 4";
  winPanel4.mScale = glm::vec3(0.069f, 0.06f, 0.06f);
  winPanel4.mOffset = glm::vec3(0.15f, 4.2f, -10.15f);
  winPanel4.mRotate = 270.0f;
  winPanel4.mModelPath = "Models/window_panel_4.obj";
  winPanel4.mTexturePath = "Models/textures/window/combined_texture_window.jpeg";

  meshes.push_back(bench); // It should at first else [benchplacement function] will not work :)
  meshes.push_back(sideTile);
  meshes.push_back(tile);
  meshes.push_back(light);
  meshes.push_back(podium);
  meshes.push_back(table);
  meshes.push_back(door);
  meshes.push_back(board);
  meshes.push_back(remote1);
  meshes.push_back(winPanel1);
  meshes.push_back(winPanel2);
  meshes.push_back(winPanel3);
  meshes.push_back(winPanel4);
  meshes.push_back(doorFrame);
}


void ObjectFilling(std::vector<Mesh3D>& meshes)
{
  for (Mesh3D& mesh : meshes) {
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


void BenchPlacement(std::vector<Mesh3D>& meshes)
{
  Mesh3D refBench = meshes[0];
  meshes.erase(meshes.begin());

  float distbwBenchRow = 1.5f;
  float distbwBenchCol = 3.2f;

  float refX = refBench.mOffset.x;
  float refY = refBench.mOffset.y;
  float refZ = refBench.mOffset.z;

  for (int i = 0; i < 25; i++)
  {
    // exceptions !! 
    if (i == 0 || i == 5) continue; // these benches are no there in class
    if (i / 5 == 2) // Column 2 and column 3 have less spacing then other
      refX = 0.07f;

    float newX = refX - (distbwBenchCol * (i / 5));
    float newY = refY;
    float newZ = refZ - (distbwBenchRow * (i % 5));
    
    refBench.mOffset = glm::vec3(newX, newY, newZ);
    meshes.push_back(refBench);
  }
}


void LightPlacement(std::vector<Mesh3D>& meshes)
{
  Mesh3D refLight = meshes[1]; // as the starting bench was erased

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
    meshes.push_back(refLight);
  }
}

void TilePlacement(std::vector<Mesh3D>& meshes)
{
  Mesh3D refTile = meshes[0]; 

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
    meshes.push_back(refTile);
  }
}

void SideTilePlacement(std::vector<Mesh3D>& meshes)
{
  Mesh3D refTile = meshes[0]; 
  meshes.erase(meshes.begin());

  float distbwTileRow = 1.0f;
  float distbwTileCol = 1.0f;

  float refX = refTile.mOffset.x;
  float refY = refTile.mOffset.y;
  float refZ = refTile.mOffset.z;
  float refR = refTile.mRotate;

  for (int i = 0; i < 10; i++)
  {
    if (i < 2)  continue; // door is there
    float newX = refX;
    float newY = refY;
    float newZ = refZ - (distbwTileRow * (i % 10));
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    meshes.push_back(refTile);
  }

  for (int i = 0; i < 15; i++)
  {
    float newX = refX - (distbwTileCol * (i % 15));
    float newY = refY;
    float newZ = refZ - 0.005f;
    float newR = refR +  90.0f;
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    refTile.mRotate = newR;
    meshes.push_back(refTile);
    
  }

  for (int i = 0; i < 15; i++)
  {
    float newX = (refX - (distbwTileCol * (i % 15))) - 1.005f;
    float newY = refY;
    float newZ = refZ - 10.0f;
    float newR = refR -  90.0f;
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    refTile.mRotate = newR;
    meshes.push_back(refTile);
    
  }

  for (int i = 0; i < 10; i++)
  {
    float newX = refX - 15.0f;
    float newY = refY;
    float newZ = (refZ - (distbwTileRow * (i % 10))) - 1.005f;
    float newR = refR -  180.0f;
    
    refTile.mOffset = glm::vec3(newX, newY, newZ);
    refTile.mRotate = newR;
    meshes.push_back(refTile);
  }
}

int main()
{
  initialization(&gApp);
  initializeGrid();

  Shader shader;
  gApp.mGraphicsPipelineShaderProgram =  shader.mCreateGraphicsPipeline("shaders/vert.glsl", "shaders/frag.glsl");

  std::vector<Mesh3D> meshes;
  ObjectCreation(meshes);
  ObjectFilling(meshes);

  BenchPlacement(meshes);
  SideTilePlacement(meshes);
  LightPlacement(meshes);
  TilePlacement(meshes);

  // Lights
  glm::vec3 tempLightPos;
  for (int i = 0; i < gApp.mLightsNumber; i++)
  {
    tempLightPos.x = gApp.mRefLightPos.x - (gApp.mDistBwLightCol * (i / 3));
    tempLightPos.y = gApp.mRefLightPos.y;
    tempLightPos.z = gApp.mRefLightPos.z - (gApp.mDistBwLightRow * (i % 3));

    gApp.mLights[i].mPosition = tempLightPos;
    gApp.mLights[i].mGenShadowMap(meshes);
    gApp.mLightProjectionViewMatrixCombined[i] = gApp.mLights[i].mGetProjectionMatrix() * gApp.mLights[i].mGetViewMatrix();
  }

  mainLoop(&gApp, meshes);
  cleanUp();

  return 0;
}
