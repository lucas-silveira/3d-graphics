#include <SFML/Graphics.hpp>
#include <fstream>
#include <strstream>
#include <cmath>
#include <vector>

const unsigned int SCREEN_WIDTH = 920;
const unsigned int SCREEN_HEIGHT = 640;
float theta = 0.0f;

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

mesh meshCube;
mat4x4 projMatrix;
vec3d camera;

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

void drawTriangleLine(
        sf::RenderWindow &w,
        int x1, int y1,
        int x2, int y2,
        int x3, int y3,
        sf::Color color = sf::Color::White
    )
{
    sf::VertexArray v1(sf::Lines, 2);
    sf::VertexArray v2(sf::Lines, 2);
    sf::VertexArray v3(sf::Lines, 2);

    v1[0].position = sf::Vector2f(x1, y1);
    v1[1].position = sf::Vector2f(x2, y2);

    v2[0].position = sf::Vector2f(x2, y2);
    v2[1].position = sf::Vector2f(x3, y3);

    v3[0].position = sf::Vector2f(x3, y3);
    v3[1].position = sf::Vector2f(x1, y1);

    v1[0].color = color;
    v1[1].color = color;
    v2[0].color = color;
    v2[1].color = color;
    v3[0].color = color;
    v3[1].color = color;

    w.draw(v1);
    w.draw(v2);
    w.draw(v3);
}

void drawTriangleFilled(
        sf::RenderWindow &w,
        int x1, int y1,
        int x2, int y2,
        int x3, int y3,
        sf::Color color
    )
{
    sf::ConvexShape triangle;
    triangle.setPointCount(6);

    triangle.setPoint(0, sf::Vector2f(x1, y1));
    triangle.setPoint(1, sf::Vector2f(x2, y2));

    triangle.setPoint(2, sf::Vector2f(x2, y2));
    triangle.setPoint(3, sf::Vector2f(x3, y3));

    triangle.setPoint(4, sf::Vector2f(x3, y3));
    triangle.setPoint(5, sf::Vector2f(x1, y1));

    triangle.setFillColor(color);

    w.draw(triangle);
}


void drawObj(sf::RenderWindow &w, sf::Time elapsed)
{
    // Set up rotation matrices
    mat4x4 matRotZ, matRotX;
    theta += 1.0f * elapsed.asSeconds();

    matRotZ = makeRotatedMatrixZ(theta);
    matRotX = makeRotatedMatrixX(theta*0.5f);

    mat4x4 matTrans;
    matTrans = makeTranslatedMatrix(0.0f, 0.0f, 2.0f);

    mat4x4 matWorld;
    matWorld = mulMatrices(matRotZ, matRotX);
    matWorld = mulMatrices(matWorld, matTrans);

    std::vector<triangle> vecTrianglesToRaster;

    for (auto tri : meshCube.tris)
    {
        triangle triProjected, triTransformed;

        triTransformed.p[0] = mulMatrixByVector(matWorld, tri.p[0]);
        triTransformed.p[1] = mulMatrixByVector(matWorld, tri.p[1]);
        triTransformed.p[2] = mulMatrixByVector(matWorld, tri.p[2]);

        // Use cross-product to get surface normal
        vec3d normal, line1, line2;
        line1 = subVectors(triTransformed.p[1], triTransformed.p[0]);
        line2 = subVectors(triTransformed.p[2], triTransformed.p[0]);
        normal = crossProduct(line1, line2);
        normal = normVector(normal);

        vec3d cameraRay = subVectors(triTransformed.p[0], camera);

        if (dotProduct(normal, cameraRay) < 0.0f)
        {
            // Shading
            vec3d lightDirection = {0.0f, 1.0f, -1.0f};
            lightDirection = normVector(lightDirection);
            float dp = std::max(0.1f, dotProduct(lightDirection, normal));

            triTransformed.color = {
                static_cast<sf::Uint8>(255*dp),
                static_cast<sf::Uint8>(255*dp),
                static_cast<sf::Uint8>(255*dp)
            };

            // Project triangles from 3D -> 2D
            triProjected.p[0] = mulMatrixByVector(projMatrix, triTransformed.p[0]);
            triProjected.p[1] = mulMatrixByVector(projMatrix, triTransformed.p[1]);
            triProjected.p[2] = mulMatrixByVector(projMatrix, triTransformed.p[2]);
            triProjected.color = triTransformed.color;

            triProjected.p[0] = divVectors(triProjected.p[0], triProjected.p[0].w);
            triProjected.p[1] = divVectors(triProjected.p[1], triProjected.p[1].w);
            triProjected.p[2] = divVectors(triProjected.p[2], triProjected.p[2].w);

            // Scale into view
            vec3d offsetView = { 1,1,0 };
            triProjected.p[0] = addVectors(triProjected.p[0], offsetView);
            triProjected.p[1] = addVectors(triProjected.p[1], offsetView);
            triProjected.p[2] = addVectors(triProjected.p[2], offsetView);
            triProjected.p[0].x *= 0.5f * (float)SCREEN_WIDTH;
            triProjected.p[0].y *= 0.5f * (float)SCREEN_HEIGHT;
            triProjected.p[1].x *= 0.5f * (float)SCREEN_WIDTH;
            triProjected.p[1].y *= 0.5f * (float)SCREEN_HEIGHT;
            triProjected.p[2].x *= 0.5f * (float)SCREEN_WIDTH;
            triProjected.p[2].y *= 0.5f * (float)SCREEN_HEIGHT;

            // Store triangle for sorting
            vecTrianglesToRaster.push_back(triProjected);
        }
    }

    std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2){
        float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
        float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
        return z1 > z2;
    });

    for (auto &triProjected : vecTrianglesToRaster)
    {
        // Rasterize triangle
        drawTriangleFilled(
            w,
            triProjected.p[0].x, triProjected.p[0].y,
            triProjected.p[1].x, triProjected.p[1].y,
            triProjected.p[2].x, triProjected.p[2].y,
            triProjected.color
        );

        // drawTriangleLine(
        //     w,
        //     triProjected.p[0].x, triProjected.p[0].y,
        //     triProjected.p[1].x, triProjected.p[1].y,
        //     triProjected.p[2].x, triProjected.p[2].y,
        //     sf::Color::Black
        // );
    }
}

void init()
{
    meshCube.loadFromObjFile("assets/monkey.obj");
    projMatrix = makeProjectionMatrix(
        90.0f,
        (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH,
        0.1f, 
        1000.0f
    );
}

int main()
{
    auto window = sf::RenderWindow{ { SCREEN_WIDTH, SCREEN_HEIGHT }, "3D Graphics" };
    window.setFramerateLimit(60);
    init();

    sf::Clock clock;
    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        sf::Time elapsed = clock.restart();
        window.clear();
        drawObj(window, elapsed);
        window.display();
    }
}