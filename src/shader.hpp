#ifndef SHADER_HEADER
#define SHADER_HEADER

#include "../glad/glad.h"

#include <string>


class Shader
{
  private:
    std::string mLoadShaderAsString(const std::string& filename);

    GLuint mCreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);

    GLuint mCompileShader(GLuint type, const std::string& source);

    // helper
    void mCheckErrors(GLuint compiledShader);

  public:
    GLuint mCreateGraphicsPipeline(std::string vertexSourcePath, std::string fragSourcePath);

};
#endif
