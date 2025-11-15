#ifndef LIGHT_HEADER
#define LIGHT_HEADER

#include "../glm/ext/matrix_transform.hpp"

#include <vector>
#include <map>

#include "shadowMap.hpp"
#include "mesh.hpp"
#include "shader.hpp"


class Light
{
  public:
    float attenuationLinear;
    float attenuationQuad;

    float mAmbientStrength;
    float mDiffuseStrength;
    float mSpecularStrength; 

    glm::vec3 mPosition;
    glm::vec3 mTargetDirection;
    glm::vec3 mUpDirection;

    float mInnerCutOffAngle;
    float mOuterCutOffAngle;
    float mInnerCutOffCosine;
    float mOuterCutOffCosine;

    ShadowMap mShadowMap;
    Shader mShader;
   
    Light();
    Light(glm::vec3);
    glm::mat4 mGetProjectionMatrix();
    glm::mat4 mGetViewMatrix();
    void mGenShadowMap(std::map<std::string, Mesh3D> meshes);
};
#endif
