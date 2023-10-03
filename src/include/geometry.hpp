#include <SFML/Graphics.hpp>
#include <fstream>
#include <strstream>
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

    bool loadFromObjFile(std::string filename)
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
vec3d mulVectors(vec3d &v1, float k);
vec3d divVectors(vec3d &v1, float k);
float dotProduct(vec3d &v1, vec3d &v2);
float lenVector(vec3d &v);
vec3d normVector(vec3d &v);
vec3d crossProduct(vec3d &v1, vec3d &v2);