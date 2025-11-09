#ifndef LIGHT_HEADER
#define LIGHT_HEADER

#include "../glm/ext/matrix_transform.hpp"

#include <vector>

#include "shadowMap.hpp"
#include "mesh.hpp"
#include "shader.hpp"

// forward declaration
// otherwise it would fall in circular dependency with objects.hpp

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
    float mInnerCutOffCosine;
    float mOuterCutOffCosine;

    ShadowMap mShadowMap;
    Shader mShader;
   
    Light();
    Light(glm::vec3);
    glm::mat4 mGetProjectionMatrix();
    glm::mat4 mGetViewMatrix();
    void mGenShadowMap(std::vector<Mesh3D> meshes);
};
#endif
