#include <cstdio>
#include <vector>
#include <cstring>
#include <iostream>

#include "../glm/ext/vector_float2.hpp"
#include "../glm/ext/vector_float3.hpp"
#include "../glm/geometric.hpp"


bool loadObj(const char* path,
            std::vector<float> &outVertices,
            std::vector<float> &outUvs,
            std::vector<float> &outNormals,
            std::vector<float> &outTangents,
            std::vector<float> &outBitangents)
{
  FILE* fp = fopen(path, "r");
  if (fp == NULL)
  {
    printf("Can't even open the file\n");
    return false;
  }

  std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
  std::vector<glm::vec3> temp_vertices;
  std::vector<glm::vec2> temp_uvs;
  std::vector<glm::vec3> temp_normals;

  // Formatting data
  while (1)
  {
    char lineHeader[128];
    // Read first word of the line
    if (fscanf(fp, "%s", lineHeader) == EOF)
      break;

    if (strcmp(lineHeader, "v") == 0)
    {
      glm::vec3 vertex;
      fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
      temp_vertices.push_back(vertex);
    }
    else if (strcmp(lineHeader, "vn") == 0)
    {
      glm::vec3 normal;
      fscanf(fp, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
      temp_normals.push_back(normal);
    }
    else if (strcmp(lineHeader, "vt") == 0)
    {
      glm::vec2 uv;
      fscanf(fp, "%f %f\n", &uv.x, &uv.y);
      temp_uvs.push_back(uv);
    }
    else if (strcmp(lineHeader,"f") == 0)
    {
      unsigned int vertex[3], uv[3], normal[3]; 
      int matches = fscanf(fp, "%u/%u/%u %u/%u/%u %u/%u/%u\n",
                                &vertex[0], &uv[0], &normal[0],
                                &vertex[1], &uv[1], &normal[1],
                                &vertex[2], &uv[2], &normal[2]);

      if (matches != 9)
      {
        printf("Create a better praser\n");
        return false;
      }
      
      vertexIndices.push_back(vertex[0]);
      vertexIndices.push_back(vertex[1]);
      vertexIndices.push_back(vertex[2]);
      uvIndices.push_back(uv[0]);
      uvIndices.push_back(uv[1]);
      uvIndices.push_back(uv[2]);
      normalIndices.push_back(normal[0]);
      normalIndices.push_back(normal[1]);
      normalIndices.push_back(normal[2]);
    }
  }
  
  fclose(fp);

  // Storing vertices as float not as glm 
  for (unsigned int i = 0; i < vertexIndices.size(); i++)
  {
    unsigned int index = vertexIndices[i];
    glm::vec3 vertex = temp_vertices[index - 1];
    outVertices.push_back(vertex.x);
    outVertices.push_back(vertex.y);
    outVertices.push_back(vertex.z);
  }

  for (unsigned int i = 0; i < uvIndices.size(); i++)
  {
    unsigned int index = uvIndices[i];
    glm::vec2 uv = temp_uvs[index - 1];
    outUvs.push_back(uv.x);
    outUvs.push_back(uv.y);
  }

  for (unsigned int i = 0; i < normalIndices.size(); i++)
  {
    unsigned int index = normalIndices[i];
    glm::vec3 normal = temp_normals[index - 1];
   
    outNormals.push_back(normal.x);
    outNormals.push_back(normal.y);
    outNormals.push_back(normal.z);
  }


  // Generating tangent and bitangent
  std::vector<glm::vec3> temp_tangents(temp_vertices.size(), glm::vec3(0.0f));
  std::vector<glm::vec3> temp_bitangents(temp_vertices.size(), glm::vec3(0.0f));

  for (unsigned int i = 0; i < vertexIndices.size(); i+=3)
  {
    unsigned int i0 = vertexIndices[i];
    unsigned int i1 = vertexIndices[i + 1];
    unsigned int i2 = vertexIndices[i + 2];
    
    unsigned int i_uv0 = uvIndices[i]; 
    unsigned int i_uv1 = uvIndices[i + 1];
    unsigned int i_uv2 = uvIndices[i + 2];

    glm::vec3 p0 = temp_vertices[i0 - 1];
    glm::vec3 p1 = temp_vertices[i1 - 1];
    glm::vec3 p2 = temp_vertices[i2 - 1];

    glm::vec2 uv0 = temp_uvs[i_uv0 - 1];
    glm::vec2 uv1 = temp_uvs[i_uv1 - 1];
    glm::vec2 uv2 = temp_uvs[i_uv2 - 1];

    glm::vec3 E1 = p1 - p0;
    glm::vec3 E2 = p2 - p0;
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float f = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;

    if (fabs(f) < 1e-6f) // 1e-6 is a tiny "epsilon" value, close to zero
    {
        // This triangle has bad UVs, skip its contribution
        continue;
    }

    float determinant = 1.0f / f; 
    // float determinant = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    glm::vec3 faceTangent;
    faceTangent.x = determinant * (deltaUV2.y * E1.x - deltaUV1.y * E2.x);
    faceTangent.y = determinant * (deltaUV2.y * E1.y - deltaUV1.y * E2.y);
    faceTangent.z = determinant * (deltaUV2.y * E1.z - deltaUV1.y * E2.z);

    glm::vec3 faceBitangent;
    faceBitangent.x = determinant * (-deltaUV2.x * E1.x + deltaUV1.x * E2.x);
    faceBitangent.y = determinant * (-deltaUV2.x * E1.y + deltaUV1.x * E2.y);
    faceBitangent.z = determinant * (-deltaUV2.x * E1.z + deltaUV1.x * E2.z);
    
    temp_tangents[i0 - 1] += faceTangent;
    temp_tangents[i1 - 1] += faceTangent;
    temp_tangents[i2 - 1] += faceTangent;

    temp_bitangents[i0 - 1] += faceBitangent;
    temp_bitangents[i1 - 1] += faceBitangent;
    temp_bitangents[i2 - 1] += faceBitangent;
  }
 
  // converting to required format
  for (int i = 0; i < vertexIndices.size(); i++)
  {
    unsigned int vertexIndex = vertexIndices[i] - 1;

    glm::vec3 norm_tangents = glm::normalize(temp_tangents[vertexIndex]);
    outTangents.push_back(norm_tangents.x);
    outTangents.push_back(norm_tangents.y);
    outTangents.push_back(norm_tangents.z);

    glm::vec3 norm_bitangents = glm::normalize(temp_bitangents[vertexIndex]);
    outBitangents.push_back(norm_bitangents.x);
    outBitangents.push_back(norm_bitangents.y);
    outBitangents.push_back(norm_bitangents.z);
  }
    
  // std::cout << "In load model file" << std::endl;
  // std::cout << "Size of vertexIndex: " << vertexIndices.size() << std::endl;
  // std::cout << "Size of uvIndex: " << uvIndices.size() << std::endl;

  return true;
}
