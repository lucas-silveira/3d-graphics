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

mat4x4 makeIdentityMatrix();
mat4x4 makeRotatedMatrixX(float angleRad);
mat4x4 makeRotatedMatrixY(float angleRad);
mat4x4 makeRotatedMatrixZ(float angleRad);
mat4x4 makeTranslatedMatrix(float x, float y, float z);
mat4x4 makeProjectionMatrix(float fovDegrees, float aspectRatio, float near, float far);
mat4x4 mulMatrices(mat4x4 &m1, mat4x4 &m2);
mat4x4 pointAt(vec3d &pos, vec3d &forw, vec3d &up);
mat4x4 quickInverse(mat4x4 &m);
vec3d mulMatrixByVector(mat4x4 &m, vec3d &i);
vec3d addVectors(vec3d &v1, vec3d &v2);
vec3d subVectors(vec3d &v1, vec3d &v2);
vec3d mulVector(vec3d &v1, float k);
vec3d divVector(vec3d &v1, float k);
float dotProduct(vec3d &v1, vec3d &v2);
float lenVector(vec3d &v);
vec3d normVector(vec3d &v);
vec3d crossProduct(vec3d &v1, vec3d &v2);
vec3d intersectPlane(vec3d &planeP, vec3d &planeN, vec3d &lineStart, vec3d&lineEnd);
int clipAgainstPlane(vec3d planeP, vec3d planeN, triangle &inTri, triangle &outTri1, triangle &outTri2);
