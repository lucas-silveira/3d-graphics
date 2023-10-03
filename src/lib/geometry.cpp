#include <SFML/Graphics.hpp>
#include <cmath>
#include <fstream>
#include <strstream>
#include "../include/geometry.hpp"

bool mesh::loadFromObjFile(std::string filename)
{
    std::ifstream f(filename);
    if (!f.is_open()) return false;

    // Local cache vertices
    std::vector<vec3d> verts;

    while (!f.eof())
    {
        char line[128];
        f.getline(line, 128);

        std::strstream s;
        s << line;

        char junk;

        if (line[0] == 'v')
        {
            vec3d v;
            s >> junk >> v.x >> v.y >> v.z;
            verts.push_back(v);
        }

        if (line[0] == 'f')
        {
            int f[3];
            s >> junk >> f[0] >> f[1] >> f[2];
            tris.push_back({verts[f[0]-1], verts[f[1]-1], verts[f[2]-1]});
        }
    }

    return true;
}

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

mat4x4 pointAt(vec3d &pos, vec3d &forw, vec3d &up)
{
    // Forward direction
    vec3d newForward = subVectors(forw, pos);
    newForward = normVector(newForward);

    // Up direction
    vec3d a = mulVector(newForward, dotProduct(up, newForward));
    vec3d newUp = subVectors(up, a);
    newUp = normVector(newUp);

    // Right direction
    vec3d newRight = crossProduct(newUp, newForward);

    // Point at matrix
    mat4x4 matrix;
    matrix.m[0][0] = newRight.x;   matrix.m[0][1] = newRight.y;   matrix.m[0][2] = newRight.z;   matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = newUp.x;      matrix.m[1][1] = newUp.y;      matrix.m[1][2] = newUp.z;      matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = newForward.x; matrix.m[2][1] = newForward.y; matrix.m[2][2] = newForward.z; matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = pos.x;        matrix.m[3][1] = pos.y;        matrix.m[3][2] = pos.z;        matrix.m[3][3] = 1.0f;

    return matrix;
}

mat4x4 quickInverse(mat4x4 &m) // Only for rotation/translation matrices
{
    mat4x4 matrix;
    matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
    matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
    matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
    matrix.m[3][3] = 1.0f;
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

vec3d mulVector(vec3d &v1, float k)
{
    return {v1.x * k, v1.y * k, v1.z * k};
}

vec3d divVector(vec3d &v1, float k)
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

vec3d intersectPlane(vec3d &planeP, vec3d &planeN, vec3d &lineStart, vec3d&lineEnd)
{
    planeN = normVector(planeN);
    float planeD = -dotProduct(planeN, planeP);
    float ad = dotProduct(lineStart, planeN);
    float bd = dotProduct(lineEnd, planeN);
    float t = (-planeD - ad) / (bd - ad);
    vec3d lineStartToEnd = subVectors(lineEnd, lineStart);
    vec3d lineToIntersect = mulVector(lineStartToEnd, t);
    return addVectors(lineStart, lineToIntersect);
}

int clipAgainstPlane(vec3d planeP, vec3d planeN, triangle &inTri, triangle &outTri1, triangle &outTri2)
{
    planeN = normVector(planeN);

    // Return signed shortest distance from point to plane, plane normal must be normalised
    auto dist = [&](vec3d &p)
    {
        vec3d n = normVector(p);
        return (planeN.x*p.x + planeN.y*p.y + planeN.z*p.z - dotProduct(planeN, planeP));
    };

    // Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
    vec3d* insidePoints[3];
    vec3d* outsidePoints[3];
    int nInsidePointCount = 0, nOutsidePointCount = 0;

    // Get signed distance of each point in triangle to plane
    float d0 = dist(inTri.p[0]);
	float d1 = dist(inTri.p[1]);
	float d2 = dist(inTri.p[2]);

    if (d0 >= 0) insidePoints[nInsidePointCount++] = &inTri.p[0];
	else outsidePoints[nOutsidePointCount++] = &inTri.p[0];
    if (d1 >= 0) insidePoints[nInsidePointCount++] = &inTri.p[1];
	else outsidePoints[nOutsidePointCount++] = &inTri.p[1];
    if (d2 >= 0) insidePoints[nInsidePointCount++] = &inTri.p[2];
	else outsidePoints[nOutsidePointCount++] = &inTri.p[2];

    // Classify triangle points, and break the input triangle into 
    // smaller output triangles if required. There are four possible
    // outcomes...
    if (nInsidePointCount == 0)
    {
        // All points lie on the outside of plane, so clip whole triangle
        // It ceases to exist
        return 0; // No returned triangles are valid
    }

    if (nInsidePointCount == 3)
    {
        // All points lie on the inside of plane, so do nothing
        // and allow the triangle to simply pass through
        outTri1 = inTri;
        return 1; // Just the one returned original triangle is valid
    }

    if (nInsidePointCount == 1 && nOutsidePointCount == 2)
    {
        // Triangle should be clipped. As two points lie outside
        // the plane, the triangle simply becomes a smaller triangle

        // Copy appearance info to new triangle
        outTri1.color =  inTri.color;

        // The inside point is valid, so keep that...
        outTri1.p[0] = *insidePoints[0];

        // but the two new points are at the locations where the 
        // original sides of the triangle (lines) intersect with the plane
        outTri1.p[1] = intersectPlane(planeP, planeN, *insidePoints[0], *outsidePoints[0]);
        outTri1.p[2] = intersectPlane(planeP, planeN, *insidePoints[0], *outsidePoints[1]);

        return 1; // Return the newly formed single triangle
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1)
    {
        // Triangle should be clipped. As two points lie inside the plane,
        // the clipped triangle becomes a "quad". Fortunately, we can
        // represent a quad with two new triangles

        // Copy appearance info to new triangles
        outTri1.color =  inTri.color;
        outTri2.color =  inTri.color;

        // The first triangle consists of the two inside points and a new
        // point determined by the location where one side of the triangle
        // intersects with the plane
        outTri1.p[0] = *insidePoints[0];
        outTri1.p[1] = *insidePoints[1];
        outTri1.p[2] = intersectPlane(planeP, planeN, *insidePoints[0], *outsidePoints[0]);

        // The second triangle is composed of one of he inside points, a
        // new point determined by the intersection of the other side of the 
        // triangle and the plane, and the newly created point above
        outTri2.p[0] = *insidePoints[1];
        outTri2.p[1] = outTri1.p[2];
        outTri2.p[2] = intersectPlane(planeP, planeN, *insidePoints[1], *outsidePoints[0]);

        return 2; // Return two newly formed triangles which form a quad
    }

    return 0;
}