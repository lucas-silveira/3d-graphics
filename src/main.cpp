#include <SFML/Graphics.hpp>
#include <vector>

const unsigned int SCREEN_WIDTH = 920;
const unsigned int SCREEN_HEIGHT = 640;
float theta = 0.0f;

struct vec3d
{
    float x, y, z;
};

struct triangle
{
    vec3d p[3];
};

struct mesh
{
    std::vector<triangle> tris;
};

struct mat4x4
{
    float m[4][4] = { 0 };
};

mesh meshCube;
mat4x4 matProj;

void multiplyMatrixVec(vec3d &i,vec3d &o, mat4x4 &m)
{
    o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
    o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
    o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
    float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

    if (w != 0.0f)
    {
        o.x /= w; o.y /= w; o.z /= w;
    }
}

void drawTriangle(
        sf::RenderWindow &w,
        int x1, int y1,
        int x2, int y2,
        int x3, int y3
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

    w.draw(v1);
    w.draw(v2);
    w.draw(v3);
}


void drawCube(sf::RenderWindow &w, sf::Time elapsed)
{
    mat4x4 matRotZ, matRotX;
    theta += 1.0f * elapsed.asSeconds();

    matRotZ.m[0][0] = std::cosf(theta);
    matRotZ.m[0][1] = std::sinf(theta);
    matRotZ.m[1][0] = -std::sinf(theta);
    matRotZ.m[1][1] = std::cosf(theta);
    matRotZ.m[2][2] = 1.0f;
    matRotZ.m[3][3] = 1.0f;

    matRotX.m[0][0] = 1.0f;;
    matRotX.m[1][1] = std::cosf(theta*0.5f);
    matRotX.m[1][2] = std::sinf(theta*0.5f);
    matRotX.m[2][1] = -std::sinf(theta*0.5f);
    matRotX.m[2][2] = std::cosf(theta*0.5f);
    matRotX.m[3][3] = 1.0f;

    for (auto tri : meshCube.tris)
    {
        triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

        multiplyMatrixVec(tri.p[0], triRotatedZ.p[0], matRotZ);
        multiplyMatrixVec(tri.p[1], triRotatedZ.p[1], matRotZ);
        multiplyMatrixVec(tri.p[2], triRotatedZ.p[2], matRotZ);

        multiplyMatrixVec(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
        multiplyMatrixVec(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
        multiplyMatrixVec(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

        triTranslated = triRotatedZX;
        triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
        triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
        triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

        multiplyMatrixVec(triTranslated.p[0], triProjected.p[0], matProj);
        multiplyMatrixVec(triTranslated.p[1], triProjected.p[1], matProj);
        multiplyMatrixVec(triTranslated.p[2], triProjected.p[2], matProj);

        // Scale into view
        triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
        triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
        triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

        triProjected.p[0].x *= 0.5f * (float)SCREEN_WIDTH;
        triProjected.p[0].y *= 0.5f * (float)SCREEN_HEIGHT;
        triProjected.p[1].x *= 0.5f * (float)SCREEN_WIDTH;
        triProjected.p[1].y *= 0.5f * (float)SCREEN_HEIGHT;
        triProjected.p[2].x *= 0.5f * (float)SCREEN_WIDTH;
        triProjected.p[2].y *= 0.5f * (float)SCREEN_HEIGHT;

        drawTriangle(
            w,
            triProjected.p[0].x, triProjected.p[0].y,
            triProjected.p[1].x, triProjected.p[1].y,
            triProjected.p[2].x, triProjected.p[2].y
        );
    }
}

void init()
{
    meshCube.tris = {
        // SOUTH
        {0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f},

        // EAST
        {1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f},

        // NORTH
        {1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f},

        // WEST
        {0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f},

        // TOP
        {0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 0.0f},

        // BOTTOM
        {1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f},
    };

    // Projection matrix
    float near = 0.1f, far = 1000.0f, fov = 90.0f;
    float aspectRatio = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH;
    float fovRad = 1.0f / std::tanf(fov*0.5f);

    matProj.m[0][0] = aspectRatio*fovRad;
    matProj.m[1][1] = fovRad;
    matProj.m[2][2] = far / (far-near);
    matProj.m[3][2] = (-far*near) / (far-near);
    matProj.m[2][3] = 1.0f;
    matProj.m[3][3] = 0.0f;
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
        drawCube(window, elapsed);
        window.display();
    }
}