#include "../glad/glad.h"

#include "shader.hpp"

#include <iostream>
#include <fstream>
#include <cstring>


GLuint Shader::mCreateGraphicsPipeline(std::string vertexSourcePath, std::string fragSourcePath)
{
  std::string vertexShaderSource = Shader::mLoadShaderAsString(vertexSourcePath); 
  std::string fragmentShaderSource = Shader::mLoadShaderAsString(fragSourcePath);

  return Shader::mCreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}


std::string Shader::mLoadShaderAsString(const std::string& filename)
{
  std::string result = "";
  std::string line = "";
  std::ifstream glslFile(filename.c_str());

  if (glslFile.is_open())
  {
    while (std::getline(glslFile, line))
    {
      result += line + '\n';
    }
    glslFile.close();
  }

  return result;
}

GLuint Shader::mCreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
  GLuint programObject = glCreateProgram();
  
  GLuint vertexShader = Shader::mCompileShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint fragmentShader = Shader::mCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  Shader::mCheckErrors(vertexShader);
  Shader::mCheckErrors(fragmentShader);

  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);

  glLinkProgram(programObject);
  return programObject;
}


GLuint Shader::mCompileShader(GLuint type, const std::string& source)
{
  GLuint shaderObject = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(shaderObject, 1, &src, nullptr);
  glCompileShader(shaderObject);

  return shaderObject;
}


void Shader::mCheckErrors(GLuint compiledShader)
{
  int passed;
  char buffer[4096];

  glGetShaderiv(compiledShader, GL_COMPILE_STATUS, &passed);

  if (!passed)
  {
    glGetShaderInfoLog(compiledShader, 4096, NULL, buffer);
    std::cout << "Shader compilation failed: \n" << buffer << std::endl;
  }
}
