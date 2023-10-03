#include <SFML/Graphics.hpp>
#include <fstream>
#include <strstream>
#include <cmath>
#include <vector>
#include <list>
#include "include/geometry.hpp"

const unsigned int SCREEN_WIDTH = 920;
const unsigned int SCREEN_HEIGHT = 640;

float theta = 0.0f;
mesh meshCube;
mat4x4 projMatrix;
vec3d camera, lookDir;
float yaw;

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
    // theta += 1.0f * elapsed.asSeconds();

    matRotZ = makeRotatedMatrixZ(theta);
    matRotX = makeRotatedMatrixX(theta*0.5f);

    mat4x4 matTrans;
    matTrans = makeTranslatedMatrix(0.0f, 0.0f, 2.0f);

    mat4x4 matWorld;
    matWorld = mulMatrices(matRotZ, matRotX);
    matWorld = mulMatrices(matWorld, matTrans);

    vec3d up = { 0,1,0 };
    vec3d forward = { 0,0,1 };
    mat4x4 matRotCamera = makeRotatedMatrixY(yaw);
    lookDir = mulMatrixByVector(matRotCamera, forward);
    forward = addVectors(camera, lookDir);

    mat4x4 matCamera = pointAt(camera, forward, up);
    // Make view matrix from camera
    mat4x4 matView = quickInverse(matCamera);

    std::vector<triangle> vecTrianglesToRaster;

    for (auto tri : meshCube.tris)
    {
        triangle triProjected, triTransformed, triViewed;

        triTransformed.p[0] = mulMatrixByVector(matWorld, tri.p[0]);
        triTransformed.p[1] = mulMatrixByVector(matWorld, tri.p[1]);
        triTransformed.p[2] = mulMatrixByVector(matWorld, tri.p[2]);

        // Use cross-product to get surface normal
        vec3d normal, line1, line2;
        line1 = subVectors(triTransformed.p[1], triTransformed.p[0]);
        line2 = subVectors(triTransformed.p[2], triTransformed.p[0]);
        normal = crossProduct(line1, line2);
        normal = normVector(normal);

        // Get Ray from triangle to camera
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

            // Convert world space -> view space
            triViewed.p[0] = mulMatrixByVector(matView, triTransformed.p[0]);
            triViewed.p[1] = mulMatrixByVector(matView, triTransformed.p[1]);
            triViewed.p[2] = mulMatrixByVector(matView, triTransformed.p[2]);
            triViewed.color = triTransformed.color;

            // Clip Viewed Triangle against near plane, this could form two additional triangles.
            triangle clipped[2];
            int nClippedTriangles = clipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

            for (int n = 0; n < nClippedTriangles; n++)
            {
                // Project triangles from 3D -> 2D
                triProjected.p[0] = mulMatrixByVector(projMatrix, clipped[n].p[0]);
                triProjected.p[1] = mulMatrixByVector(projMatrix, clipped[n].p[1]);
                triProjected.p[2] = mulMatrixByVector(projMatrix, clipped[n].p[2]);
                triProjected.color = clipped[n].color;

                // Scale into view
                triProjected.p[0] = divVector(triProjected.p[0], triProjected.p[0].w);
                triProjected.p[1] = divVector(triProjected.p[1], triProjected.p[1].w);
                triProjected.p[2] = divVector(triProjected.p[2], triProjected.p[2].w);

                // X/Y are inverted so put them back
                triProjected.p[0].x *= -1.0f;
                triProjected.p[1].x *= -1.0f;
                triProjected.p[2].x *= -1.0f;
                triProjected.p[0].y *= -1.0f;
                triProjected.p[1].y *= -1.0f;
                triProjected.p[2].y *= -1.0f;

                // Offset verts into visible normalised space
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
    }

    std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2){
        float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
        float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
        return z1 > z2;
    });

    // Loop through all transformed, viewed, projected, and sorted triangles
    for (auto &triToRaster : vecTrianglesToRaster)
    {
        // Clip triangles against all four screen edges, this could yield
        // a bunch of triangles, so create a queue that we traverse to 
        // ensure we only test new triangles generated against planes
        triangle clipped[2];
        std::list<triangle> listTriangles;

        // Add initial triangle
        listTriangles.push_back(triToRaster);
        int nNewTriangles = 1;

        for (int p = 0; p < 4; p++)
        {
            int nTrisToAdd = 0;
            while (nNewTriangles > 0)
            {
                // Take triangle from front of queue
                triangle test = listTriangles.front();
                listTriangles.pop_front();
                nNewTriangles--;

                // Clip it against a plane. We only need to test each 
                // subsequent plane, against subsequent new triangles
                // as all triangles after a plane clip are guaranteed
                // to lie on the inside of the plane.
                switch (p)
                {
                    case 0:
                        nTrisToAdd = clipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]);
                        break;
                    case 1:
                        nTrisToAdd = clipAgainstPlane({ 0.0f, (float)SCREEN_HEIGHT - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]);
                        break;
                    case 2:
                        nTrisToAdd = clipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]);
                        break;
                    case 3:
                        nTrisToAdd = clipAgainstPlane({ (float)SCREEN_WIDTH - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]);
                        break;
                }

                // Clipping may yield a variable number of triangles, so
                // add these new ones to the back of the queue for subsequent
                // clipping against next planes
                for (int w = 0; w < nTrisToAdd; w++)
                    listTriangles.push_back(clipped[w]);
            }
            nNewTriangles = listTriangles.size();
        }

        // Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
        for (auto &t : listTriangles)
        {
            // Rasterize triangle
            drawTriangleFilled(
                w,
                t.p[0].x, t.p[0].y,
                t.p[1].x, t.p[1].y,
                t.p[2].x, t.p[2].y,
                t.color
            );
            // drawTriangleLine(
            //     w,
            //     t.p[0].x, t.p[0].y,
            //     t.p[1].x, t.p[1].y,
            //     t.p[2].x, t.p[2].y,
            //     sf::Color::Black
            // );
        }
    }
}

void handleMovement(sf::Time elapsed)
{
    float speed = 2.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        camera.y += speed * elapsed.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        camera.y -= speed * elapsed.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        camera.x += speed * elapsed.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        camera.x -= speed * elapsed.asSeconds();

    vec3d forward = mulVector(lookDir, speed * elapsed.asSeconds());

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        camera = addVectors(camera, forward);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        camera = subVectors(camera, forward);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        yaw -= speed * elapsed.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        yaw += speed * elapsed.asSeconds();
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
        handleMovement(elapsed);

        window.clear();
        drawObj(window, elapsed);
        window.display();
    }
}