/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"
#include <vector>

const unsigned int TARGET_FPS = 60;
float dt = 1.0f / TARGET_FPS;
float time = 0;
float x = 500;
float y = 500;
float frequency = 1;
float amplitude = 100;

// ---------------- LAB 1 VARIABLES ----------------
Vector2 launchPosition = { 400, 300 };
float launchAngle = 45.0f;
float launchSpeed = 200.0f;

Vector2 GetLaunchVelocity(float angleDeg, float speed)
{
    float rad = angleDeg * DEG2RAD;
    return { cosf(rad) * speed, -sinf(rad) * speed };
}

// ---------------- PHYSICS BODY STRUCT ----------------
struct PhysicsBody {
    Vector2 position;
    Vector2 velocity;
    float drag;
    float mass;
    float radius;
    bool isFixed;
    Color color;

    PhysicsBody(Vector2 pos, Vector2 vel, float d = 0.0f, float m = 1.0f, float r = 8.0f, bool fixed = false)
        : position(pos), velocity(vel), drag(d), mass(m), radius(r), isFixed(fixed), color(GREEN) {
    }
};

// ---------------- PHYSICS SIMULATION ----------------
struct PhysicsSimulation {
    float deltaTime;
    float time;
    Vector2 gravity;

    PhysicsSimulation(float dt = 1.0f / 60.0f, Vector2 g = { 0, 200 })
        : deltaTime(dt), time(0), gravity(g) {
    }

    void UpdateBody(PhysicsBody& body) {
        if (body.isFixed) return;

        body.velocity = Vector2Add(body.velocity, Vector2Scale(gravity, deltaTime));
        body.velocity = Vector2Scale(body.velocity, (1.0f - body.drag * deltaTime));
        body.position = Vector2Add(body.position, Vector2Scale(body.velocity, deltaTime));
    }
};

PhysicsSimulation sim(1.0f / 60.0f, { 0, 200 });
std::vector<PhysicsBody> balls;

void LaunchBall() {
    Vector2 velocity = GetLaunchVelocity(launchAngle, launchSpeed);
    balls.emplace_back(launchPosition, velocity);
}

// ---------------- LAB 3 COLLISION CHECK ----------------
void CheckCollisions(std::vector<PhysicsBody>& bodies) {
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = i + 1; j < bodies.size(); j++) {
            float distance = Vector2Distance(bodies[i].position, bodies[j].position);
            float radiusSum = bodies[i].radius + bodies[j].radius;

            if (distance < radiusSum) {
                bodies[i].color = RED;
                bodies[j].color = RED;

                // LAB 5 - COLLISION RESPONSE
                float overlap = radiusSum - distance;
                Vector2 normal = Vector2Normalize(Vector2Subtract(bodies[j].position, bodies[i].position));
                Vector2 correction = Vector2Scale(normal, overlap / 2.0f);

                if (!bodies[i].isFixed)
                    bodies[i].position = Vector2Subtract(bodies[i].position, correction);
                if (!bodies[j].isFixed)
                    bodies[j].position = Vector2Add(bodies[j].position, correction);
            }
            else {
                bodies[i].color = GREEN;
                bodies[j].color = GREEN;
            }
        }
    }
}

// ---------------- HALFSPACE STRUCT ----------------
struct Halfspace {
    Vector2 point;
    Vector2 normal;

    Halfspace(Vector2 p = { 0, 500 }, Vector2 n = { 0, -1 }) {
        point = p;
        normal = Vector2Normalize(n);
    }
};

Halfspace halfspace({ 0, 500 }, { 0, -1 });

bool CheckSphereHalfspace(const PhysicsBody& sphere, const Halfspace& h) {
    float distance = Vector2DotProduct(h.normal, Vector2Subtract(sphere.position, h.point));
    return (distance < sphere.radius);
}

// ---------------- LAB 5 - HALFSPACE RESPONSE ----------------
void ResolveSphereHalfspace(PhysicsBody& sphere, const Halfspace& h) {
    float distance = Vector2DotProduct(h.normal, Vector2Subtract(sphere.position, h.point));
    float overlap = sphere.radius - distance;

    if (overlap > 0 && !sphere.isFixed) {
        sphere.color = RED;
        sphere.position = Vector2Add(sphere.position, Vector2Scale(h.normal, overlap));

        // Stop downward velocity on contact
        if (Vector2DotProduct(h.normal, sphere.velocity) > 0)
            sphere.velocity = Vector2Zero();
    }
    else {
        sphere.color = GREEN;
    }
}

// ---------------- UPDATE ----------------
void update()
{
    dt = 1.0f / TARGET_FPS;
    time += dt;

    if (IsKeyPressed(KEY_SPACE)) {
        LaunchBall();
    }

    sim.time += sim.deltaTime;

    for (auto& ball : balls) {
        sim.UpdateBody(ball);
    }

    CheckCollisions(balls);

    for (auto& ball : balls) {
        ResolveSphereHalfspace(ball, halfspace);
    }
}

// ---------------- DRAW ----------------
void draw()
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("Arvin Beigi 101447957", 10, float(GetScreenHeight() - 30), 20, LIGHTGRAY);

    GuiSliderBar({ 10, 60, 200, 20 }, "Angle", TextFormat("%.1f", launchAngle), &launchAngle, 0, 90);
    GuiSliderBar({ 10, 90, 200, 20 }, "Speed", TextFormat("%.1f", launchSpeed), &launchSpeed, 0, 400);
    GuiSliderBar({ 10, 120, 200, 20 }, "Gravity Y", TextFormat("%.1f", sim.gravity.y), &sim.gravity.y, -500, 500);

    Vector2 velocity = GetLaunchVelocity(launchAngle, launchSpeed);
    Vector2 endPoint = Vector2Add(launchPosition, Vector2Scale(velocity, 0.2f));

    DrawCircleV(launchPosition, 8, GREEN);
    DrawLineV(launchPosition, endPoint, RED);

    DrawText(TextFormat("Angle: %.1f deg", launchAngle), 10, 150, 20, LIGHTGRAY);
    DrawText(TextFormat("Speed: %.1f", launchSpeed), 10, 175, 20, LIGHTGRAY);
    DrawText(TextFormat("Velocity: (%.1f, %.1f)", velocity.x, velocity.y), 10, 200, 20, LIGHTGRAY);
    DrawText("Press SPACE to launch balls", 10, 225, 20, LIGHTGRAY);

    for (auto& ball : balls) {
        DrawCircleV(ball.position, ball.radius, ball.color);
    }

    DrawLine(0, halfspace.point.y, GetScreenWidth(), halfspace.point.y, BLUE);

    EndDrawing();
}

// ---------------- MAIN ----------------
int main()
{
    InitWindow(InitialWidth, InitialHeight, "GAME2005 Arvin Beigi 101447957");
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose())
    {
        update();
        draw();
    }

    CloseWindow();
    return 0;
}
