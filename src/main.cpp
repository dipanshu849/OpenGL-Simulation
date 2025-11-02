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
#include "camera.hpp"
#include "loadModel.hpp"


struct App
{
  int mScreenWidth = 1080;
  int mScreenHeight = 720;
  
  const char* mTitle = "CL-3";

  GLFWwindow * mWindow = nullptr;
  GLuint mGraphicsPipelineShaderProgram = 0;

  Camera mCamera;
  GLfloat mCameraSpeed = 10.0f;

  GLfloat mDeltaTime = 0;
  GLfloat mLastFrame = glfwGetTime();

  int mIsPhong = 1;
};


template <typename T>
struct Mesh3D
{
  GLuint mVertexArrayObject = 0;

  GLuint mPositionVertexBufferObject = 0;
  GLuint mUvVertexBufferObject = 0;
  GLuint mNormalVertexBufferObject = 0;

  GLuint mTextureObject = 0;
  
  // we can use glfoat or glm::vec3 direct
  std::vector<T> mVertexData; 
  std::vector<T> mUvData; // T can cause problem here
  std::vector<T> mNormalData;

  glm::vec3 mOffset = glm::vec3(0.0f);
  GLfloat mRotate = 0.0f;
  glm::vec3 mScale = glm::vec3(0.0f);

  const char* name = "";
  const char* mModelPath = "";
  const char* mTexturePath = "";
};


struct Grid
{
  GLuint mVertexArrayObject = 0; 
  GLuint mVertexBufferObjectH = 0;
  GLuint mVertexBufferObjectV = 0;

  std::vector<glm::vec3> mVertexDataH;
  std::vector<glm::vec3> mVertexDataV;

  int mROW = 11;
  int mCOL = 16;
  float mTileSize = 1.0f;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBALS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
App gApp;
Grid gGrid;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBALS END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~~~~~~~~~~~~ ERROR HANDLING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void GLClearAllErrors()
{
  while(glGetError() != GL_NO_ERROR);
}


static bool GLCheckErrorStatus(const char* function, int line)
{
  while(GLenum error = glGetError())
  {
    std::cout << "OpenGL Error: " << error
              << "\tLine: " << line
              << "\tfunction: " << function << std::endl;
    return true;
  }
  return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);
// ~~~~~~~~~~~~~~~~~~~~~~~~~ ERROR HANDLING END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


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
  glm::vec3 refCoordinate = glm::vec3(0.0f, 0.0f, 0.0f);

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
template <typename T>
bool meshCreate(const char* path, Mesh3D<T> *mesh)
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
template <typename T>
bool loadTexture(const char* path, Mesh3D<T> *mesh)
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
  
  if(stbi_failure_reason())
  {
    std::cout << stbi_failure_reason() << std::endl;
  }
  
  if (data)
  {
     // std::cout << "Width of texture: " << width << std::endl;
     // std::cout << "Height of texture: " << height << std::endl;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load texture" << std::endl;
    return false;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);

  return true;
}


// Sets up mesh data transfer from CPU to GPU 
template <typename T>
void meshCTGdataTransfer(Mesh3D<T>* mesh) 
{
  glGenVertexArrays(1, &mesh->mVertexArrayObject);
  glBindVertexArray(mesh->mVertexArrayObject);

  // 1. start generating our position VBO
  glGenBuffers(1, &mesh->mPositionVertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->mPositionVertexBufferObject);
  glBufferData(GL_ARRAY_BUFFER,
              mesh->mVertexData.size() * sizeof(T),
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
              mesh->mUvData.size() * sizeof(T),
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
              mesh->mNormalData.size() * sizeof(T),
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


// ~~~~~~~~~~~~~~~~~~ Graphics Pipline Setup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GLuint CompileShader(GLuint type, const std::string& source) 
{
    GLuint shaderObject;

    if (type == GL_VERTEX_SHADER) {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    } else if (type == GL_FRAGMENT_SHADER) {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src = source.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);

    return shaderObject;
}

GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) 
{
    GLuint programObject = glCreateProgram();

    GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // error checking for fragment shader
    int success;
    char infoLog[512];
    glGetShaderiv(myFragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(myFragmentShader, 512, NULL, infoLog);
        std::cout << "Compilation of frag failed\n" << infoLog << std::endl;
    }
    //

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    glValidateProgram(programObject);
    return programObject;
}


// Converts GLSL files to strings
std::string loadShaderAsString(const std::string& filename) {
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if (myFile.is_open()) {
        while (std::getline(myFile, line)) {
            result += line + '\n';
        }
        myFile.close();
    }

    return result;
}


void createGraphicsPipeline(App* app) 
{
    std::string vertexShaderSource = loadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = loadShaderAsString("./shaders/frag.glsl");

    app->mGraphicsPipelineShaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
}
// ~~~~~~~~~~~~~~~~~~ Graphics Pipline Setup END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void Input(App* app)
{
  if(glfwGetKey(app->mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(app->mWindow, true);
}


void PreDraw(App* app) 
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glViewport(0, 0, app->mScreenWidth, app->mScreenHeight);
  glClearColor(1.f, 0.f, 0.f, 1.f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

  glUseProgram(app->mGraphicsPipelineShaderProgram);
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


template <typename T>
void MeshTransformation(App* app, Mesh3D<T>* mesh)
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


  // LightPosition
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightPos");
  glm::vec3 lightPos = glm::vec3(-3.5f, 6.5f, -1.5f);
  //glm::vec3 lightPos = glm::vec3(-3.5f, 1.0f, -1.5f);
  glUniform3f(location, lightPos.x, lightPos.y, lightPos.z);


  // LightColor
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_lightColor");
  glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
  glUniform3f(location, lightColor.x, lightColor.y, lightColor.z);

  // ViewPosition
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_viewPos");
  glm::vec3 viewPos = app->mCamera.getViewPos();
  glUniform3f(location, viewPos.x, viewPos.y, viewPos.z);

  // toggleShading
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_isPhong");
  glUniform1i(location, app->mIsPhong);
}


template <typename T>
void Draw(Mesh3D<T>* mesh) 
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mesh->mTextureObject);
  glBindVertexArray(mesh->mVertexArrayObject);
  glDrawArrays(GL_TRIANGLES, 0, mesh->mVertexData.size() / 3);
}


template <typename T>
void mainLoop(App* app, std::vector<Mesh3D<T>> meshes) 
{
  while (!glfwWindowShouldClose(app->mWindow))
  {
    float currentTime = glfwGetTime();
    app->mDeltaTime = currentTime - app->mLastFrame;
    app->mLastFrame = currentTime;
  
    Input(app);
    PreDraw(app);

    DisplayGrid(app);

    for (Mesh3D<T> mesh : meshes)
    {
      MeshTransformation(app, &mesh);
      Draw(&mesh);
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



void ObjectCreation(std::vector<Mesh3D<GLfloat>>& meshes)
{
  Mesh3D<GLfloat> bench;
  Mesh3D<GLfloat> podium;
  Mesh3D<GLfloat> table;
  Mesh3D<GLfloat> door;
  Mesh3D<GLfloat> light;
  Mesh3D<GLfloat> board;

  bench.name = "Bench";
  bench.mScale = glm::vec3(0.076f, 0.07f, 0.057f);
  bench.mOffset = glm::vec3(-0.6f, 0.1f, -2.6f);
  bench.mModelPath = "Models/BenchTextured.obj";
  bench.mTexturePath = "Models/textures/combinedBenchTexture.png";

  podium.name = "Podium";
  podium.mScale = glm::vec3(0.16f, 0.16f, 0.16f);
  podium.mOffset = glm::vec3(-2.2f, 0.0f, -1.05f);
  podium.mRotate = 180.0f;
  podium.mModelPath = "Models/podium.obj";
  podium.mTexturePath = "Models/textures/podium/podium_combined_texture_2.jpeg";

  table.name = "Table";
  table.mScale = glm::vec3(0.07f, 0.07f, 0.07f);
  table.mOffset = glm::vec3(-3.9f, 0.0f, -4.1f);
  table.mModelPath = "Models/table.obj";
  table.mTexturePath = "Models/textures/table/table_combined_texture_new_new.jpeg";

  door.name = "Door";
  door.mScale = glm::vec3(0.07f, 0.07f, 0.06f);
  door.mOffset = glm::vec3(-0.96f, 0.0f, -1.7f);
  door.mRotate = 300.0f;
  door.mModelPath = "Models/door_textured.obj";
  door.mTexturePath = "Models/textures/door/combined_texture.jpeg";

  light.name = "Light";
  light.mScale = glm::vec3(0.078f, 0.02f, 0.078f);
  light.mOffset = glm::vec3(-4.0f, 6.5f, -2.0f);
  light.mModelPath = "Models/light.obj";
  light.mTexturePath = "Models/textures/light/texture.png";

  board.name = "Board";
  board.mScale = glm::vec3(0.07f, 0.07f, 0.07f);
  board.mOffset = glm::vec3(-4.0f, 1.2f, 0.0f);
  board.mRotate = 180.0f;
  board.mModelPath = "Models/Board.obj";
  board.mTexturePath = "Models/textures/board/board_combined_texture_1.jpeg";

  meshes.push_back(bench); // It should at first else [benchplacement function] will not work :)
  meshes.push_back(podium);
  meshes.push_back(table);
  meshes.push_back(door);
  meshes.push_back(light);
  meshes.push_back(board);
}


void ObjectFilling(std::vector<Mesh3D<GLfloat>>& meshes)
{
  for (Mesh3D<GLfloat>& mesh : meshes) {
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


void BenchPlacement(std::vector<Mesh3D<GLfloat>>& meshes)
{
  Mesh3D<GLfloat> refBench = meshes[0];
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


void LightPlacement(std::vector<Mesh3D<GLfloat>>& meshes)
{
  Mesh3D<GLfloat> refLight = meshes[3]; // as the starting bench was erased

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

int main()
{
  initialization(&gApp);
  createGraphicsPipeline(&gApp);

  // ~~~~~~~~~~~~ HELPER 
  
  initializeGrid();

  // HELPER END ~~~~~~~~


  std::vector<Mesh3D<GLfloat>> meshes;
  ObjectCreation(meshes);
  ObjectFilling(meshes);

  BenchPlacement(meshes);
  LightPlacement(meshes);

  mainLoop(&gApp, meshes);
  cleanUp();

  return 0;
}
