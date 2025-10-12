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

const unsigned int TARGET_FPS = 60; //frames/second
float dt = 1.0f / TARGET_FPS; //seconds/frame
float time = 0;
float x = 500;
float y = 500;
float frequency = 1;
float amplitude = 100;

// ---------------- LAB 1 VARIABLES ----------------
Vector2 launchPosition = { 400, 300 }; // Starting point
float launchAngle = 45.0f;             // Degrees
float launchSpeed = 200.0f;            // Pixels per second

Vector2 GetLaunchVelocity(float angleDeg, float speed)
{
    float rad = angleDeg * DEG2RAD;
    return { cosf(rad) * speed, -sinf(rad) * speed }; // -sin so positive angle aims upward
}

// ---------------- LAB 2 + LAB 3 STRUCTURES ----------------
struct PhysicsBody {
    Vector2 position;
    Vector2 velocity;
    float drag;
    float mass;
    float radius;
    Color color;

    PhysicsBody(Vector2 pos, Vector2 vel, float d = 0.0f, float m = 1.0f, float r = 8.0f)
        : position(pos), velocity(vel), drag(d), mass(m), radius(r), color(GREEN) {
    }
};

struct PhysicsSimulation {
    float deltaTime;
    float time;
    Vector2 gravity;

    PhysicsSimulation(float dt = 1.0f / 60.0f, Vector2 g = { 0, 200 })
        : deltaTime(dt), time(0), gravity(g) {
    }

    void UpdateBody(PhysicsBody& body) {
        // Apply gravity to velocity
        body.velocity = Vector2Add(body.velocity, Vector2Scale(gravity, deltaTime));

        // Apply drag if needed
        body.velocity = Vector2Scale(body.velocity, (1.0f - body.drag * deltaTime));

        // Update position
        body.position = Vector2Add(body.position, Vector2Scale(body.velocity, deltaTime));
    }
};

PhysicsSimulation sim(1.0f / 60.0f, { 0, 200 });
std::vector<PhysicsBody> balls;

void LaunchBall() {
    Vector2 velocity = GetLaunchVelocity(launchAngle, launchSpeed);
    balls.emplace_back(launchPosition, velocity);
}

// ---------------- COLLISION CHECK (LAB 3) ----------------
void CheckCollisions(std::vector<PhysicsBody>& bodies) {
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = i + 1; j < bodies.size(); j++) {
            float distance = Vector2Distance(bodies[i].position, bodies[j].position);
            float radiusSum = bodies[i].radius + bodies[j].radius;

            if (distance < radiusSum) {
                bodies[i].color = RED;
                bodies[j].color = RED;
            }
            else {
                bodies[i].color = GREEN;
                bodies[j].color = GREEN;
            }
        }
    }
}

// ---------------- UPDATE ----------------
void update()
{
    dt = 1.0f / TARGET_FPS;
    time += dt;

    // Existing oscillating motion (Lab 1 demo)
    x = x + (-sin(time * frequency)) * frequency * amplitude * dt;
    y = y + (cos(time * frequency)) * frequency * amplitude * dt;

    // Lab 2 simulation
    if (IsKeyPressed(KEY_SPACE)) {
        LaunchBall();
    }

    sim.time += sim.deltaTime;

    for (auto& ball : balls) {
        sim.UpdateBody(ball);
    }

    // Lab 3 collision check
    CheckCollisions(balls);
}

// ---------------- DRAW ----------------
void draw()
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("Arvin Beigi 101447957", 10, float(GetScreenHeight() - 30), 20, LIGHTGRAY);

    // Existing GUI + visuals (Lab 1)
    GuiSliderBar(Rectangle{ 10, 15, 1000, 20 }, "", TextFormat("%.2f", time), &time, 0, 240);
    DrawText(TextFormat("T: %6.2f", time), GetScreenWidth() - 140, 10, 30, LIGHTGRAY);

    DrawCircle(x, y, 70, RED);
    DrawCircle(500 + cos(time * frequency) * amplitude, 500 + sin(time * frequency) * amplitude, 70, GREEN);

    // --- Lab-specific controls and vector drawing ---
    GuiSliderBar({ 10, 60, 200, 20 }, "Angle", TextFormat("%.1f", launchAngle), &launchAngle, 0, 90);
    GuiSliderBar({ 10, 90, 200, 20 }, "Speed", TextFormat("%.1f", launchSpeed), &launchSpeed, 0, 400);
    GuiSliderBar({ 10, 120, 200, 20 }, "Gravity Y", TextFormat("%.1f", sim.gravity.y), &sim.gravity.y, -500, 500);

    Vector2 velocity = GetLaunchVelocity(launchAngle, launchSpeed);
    Vector2 endPoint = Vector2Add(launchPosition, Vector2Scale(velocity, 0.2f));

    DrawCircleV(launchPosition, 8, GREEN); // launch origin
    DrawLineV(launchPosition, endPoint, RED); // initial velocity vector

    DrawText(TextFormat("Angle: %.1f deg", launchAngle), 10, 150, 20, LIGHTGRAY);
    DrawText(TextFormat("Speed: %.1f", launchSpeed), 10, 175, 20, LIGHTGRAY);
    DrawText(TextFormat("Velocity: (%.1f, %.1f)", velocity.x, velocity.y), 10, 200, 20, LIGHTGRAY);
    DrawText("Press SPACE to launch balls", 10, 225, 20, LIGHTGRAY);

    // Draw all launched balls (Lab 2 + Lab 3)
    for (auto& ball : balls) {
        DrawCircleV(ball.position, ball.radius, ball.color);
    }

    EndDrawing();
}

// ---------------- MAIN ----------------
int main()
{
    InitWindow(InitialWidth, InitialHeight, "GAME2005 Arvin Beigi 101447957");
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose()) // Loops TARGET_FPS times per second
    {
        update();
        draw();
    }

    CloseWindow();
    return 0;
}

