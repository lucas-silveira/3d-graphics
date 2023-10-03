#include <SFML/Graphics.hpp>
#include <cmath>
#include "../include/geometry.hpp"

mat4x4 makeIdentityMatrix()
{
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 makeRotatedMatrixX(float angleRad)
{
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = cosf(angleRad);
    matrix.m[1][2] = sinf(angleRad);
    matrix.m[2][1] = -sinf(angleRad);
    matrix.m[2][2] = cosf(angleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 makeRotatedMatrixY(float angleRad)
{
    mat4x4 matrix;
    matrix.m[0][0] = cosf(angleRad);
    matrix.m[0][2] = sinf(angleRad);
    matrix.m[2][0] = -sinf(angleRad);
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = cosf(angleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 makeRotatedMatrixZ(float angleRad)
{
    mat4x4 matrix;
    matrix.m[0][0] = cosf(angleRad);
    matrix.m[0][1] = sinf(angleRad);
    matrix.m[1][0] = -sinf(angleRad);
    matrix.m[1][1] = cosf(angleRad);
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 makeTranslatedMatrix(float x, float y, float z)
{
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    return matrix;
}

mat4x4 makeProjectionMatrix(float fovDegrees, float aspectRatio, float near, float far)
{
    float fovRad = 1.0f / tanf(fovDegrees * 0.5f / 180.0f * 3.14159f);
    mat4x4 matrix;
    matrix.m[0][0] = aspectRatio*fovRad;
    matrix.m[1][1] = fovRad;
    matrix.m[2][2] = far / (far-near);
    matrix.m[3][2] = (-far*near) / (far-near);
    matrix.m[2][3] = 1.0f;
    matrix.m[3][3] = 0.0f;
    return matrix;
}

mat4x4 mulMatrices(mat4x4 &m1, mat4x4 &m2)
{
    mat4x4 matrix;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            matrix.m[r][c] = m1.m[r][0] * m2.m[0][c]
                            + m1.m[r][1] * m2.m[1][c]
                            + m1.m[r][2] * m2.m[2][c]
                            + m1.m[r][3] * m2.m[3][c];
    return matrix;
}

vec3d mulMatrixByVector(mat4x4 &m, vec3d &i)
{
    vec3d v;
    v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
    v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
    v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
    v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
    return v;
}

vec3d addVectors(vec3d &v1, vec3d &v2)
{
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

vec3d subVectors(vec3d &v1, vec3d &v2)
{
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

vec3d mulVectors(vec3d &v1, float k)
{
    return {v1.x * k, v1.y * k, v1.z * k};
}

vec3d divVectors(vec3d &v1, float k)
{
    return {v1.x / k, v1.y / k, v1.z / k};
}

float dotProduct(vec3d &v1, vec3d &v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

float lenVector(vec3d &v)
{
    return sqrtf(dotProduct(v, v));
}

vec3d normVector(vec3d &v)
{
    float l = lenVector(v);
    return {v.x / l, v.y / l, v.z / l};
}

vec3d crossProduct(vec3d &v1, vec3d &v2)
{
    vec3d v;
    v.x = v1.y*v2.z - v1.z*v2.y;
    v.y = v1.z*v2.x - v1.x*v2.z;
    v.z = v1.x*v2.y - v1.y*v2.x;
    return v;
}
