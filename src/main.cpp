#include <SFML/Graphics.hpp>
#include <fstream>
#include <strstream>
#include <cmath>
#include <vector>
#include "include/geometry.hpp"

const unsigned int SCREEN_WIDTH = 920;
const unsigned int SCREEN_HEIGHT = 640;
float theta = 0.0f;

mesh meshCube;
mat4x4 projMatrix;
vec3d camera;

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