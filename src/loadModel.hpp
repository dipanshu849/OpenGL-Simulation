#include "../glm/ext/vector_float2.hpp"
#include "../glm/ext/vector_float3.hpp"


bool loadObj(const char* path,
             std::vector<float> &outVertices,
             std::vector<float> &outUvs,
             std::vector<float> &outNormals,
             std::vector<float> &outTangents,
             std::vector<float> &outBitangents);
