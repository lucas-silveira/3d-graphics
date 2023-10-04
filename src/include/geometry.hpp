#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <SFML/Graphics.hpp>
#include <vector>

struct vec3d
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
};

struct triangle
{
    vec3d p[3];
    sf::Color color;
};

struct mesh
{
    std::vector<triangle> tris;
    bool loadFromObjFile(std::string filename);
};

struct mat4x4
{
    float m[4][4] = { 0 };
};

extern mat4x4 makeIdentityMatrix();
extern mat4x4 makeRotatedMatrixX(float angleRad);
extern mat4x4 makeRotatedMatrixY(float angleRad);
extern mat4x4 makeRotatedMatrixZ(float angleRad);
extern mat4x4 makeTranslatedMatrix(float x, float y, float z);
extern mat4x4 makeProjectionMatrix(float fovDegrees, float aspectRatio, float near, float far);
extern mat4x4 mulMatrices(mat4x4 &m1, mat4x4 &m2);
extern mat4x4 pointAt(vec3d &pos, vec3d &forw, vec3d &up);
extern mat4x4 quickInverse(mat4x4 &m);
extern vec3d mulMatrixByVector(mat4x4 &m, vec3d &i);
extern vec3d addVectors(vec3d &v1, vec3d &v2);
extern vec3d subVectors(vec3d &v1, vec3d &v2);
extern vec3d mulVector(vec3d &v1, float k);
extern vec3d divVector(vec3d &v1, float k);
extern float dotProduct(vec3d &v1, vec3d &v2);
extern float lenVector(vec3d &v);
extern vec3d normVector(vec3d &v);
extern vec3d crossProduct(vec3d &v1, vec3d &v2);
extern vec3d intersectPlane(vec3d &planeP, vec3d &planeN, vec3d &lineStart, vec3d&lineEnd);
extern int clipAgainstPlane(vec3d planeP, vec3d planeN, triangle &inTri, triangle &outTri1, triangle &outTri2);

#endif