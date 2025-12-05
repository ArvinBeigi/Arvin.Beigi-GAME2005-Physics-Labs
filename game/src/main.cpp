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
#include <cmath>


// MODES
// 1 -> Labs 1–5  (Angry Bird, collisions, separation)
// 6 -> Lab 6     (Kinetic friction on a slope)
// 7 -> Lab 7     (Collision response, bouncing, restitution)
// F -> Key Practice 2 (Free Body Diagram)


const unsigned int TARGET_FPS = 60;

enum SimulationMode
{
    MODE_LABS = 0,
    MODE_LAB6 = 1,
    MODE_LAB7 = 2
};

SimulationMode currentMode = MODE_LABS;
bool showFBD = false; // Key Practice 2 toggle

// Simple sign helper
float Sign(float x)
{
    if (x > 0.0f) return 1.0f;
    if (x < 0.0f) return -1.0f;
    return 0.0f;
}

// Shared arrow drawing helper
void DrawArrow(Vector2 start, Vector2 end, Color c)
{
    DrawLineV(start, end, c);
    Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
    Vector2 left = { -dir.y, dir.x };
    Vector2 back = Vector2Add(end, Vector2Scale(dir, -10.0f));
    DrawLineV(end, back, c);
    DrawLineV(end, Vector2Add(back, Vector2Scale(left, 5.0f)), c);
    DrawLineV(end, Vector2Add(back, Vector2Scale(left, -5.0f)), c);
}


// LABS 1–5: Angry Bird + Physics Bodies + Collision Response

// Launch parameters
Vector2 launchPosition = { 400, 300 };
float launchAngle = 45.0f;
float launchSpeed = 200.0f;

Vector2 GetLaunchVelocity(float angleDeg, float speed)
{
    float rad = angleDeg * DEG2RAD;
    return { cosf(rad) * speed, -sinf(rad) * speed };
}

struct PhysicsBody {
    Vector2 position;
    Vector2 velocity;
    float drag;
    float mass;
    float radius;
    bool isFixed;
    Color color;

    PhysicsBody(Vector2 pos, Vector2 vel, float d = 0.0f, float m = 1.0f,
        float r = 8.0f, bool fixed = false)
        : position(pos), velocity(vel), drag(d), mass(m),
        radius(r), isFixed(fixed), color(GREEN) {
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
        if (body.isFixed) return;

        body.velocity = Vector2Add(body.velocity,
            Vector2Scale(gravity, deltaTime));
        body.velocity = Vector2Scale(body.velocity,
            (1.0f - body.drag * deltaTime));
        body.position = Vector2Add(body.position,
            Vector2Scale(body.velocity, deltaTime));
    }
};

PhysicsSimulation sim(1.0f / 60.0f, { 0, 200 });
std::vector<PhysicsBody> balls;

// Halfspace ground for Labs
struct Halfspace {
    Vector2 point;
    Vector2 normal;

    Halfspace(Vector2 p = { 0, 500 }, Vector2 n = { 0, -1 }) {
        point = p;
        normal = Vector2Normalize(n);
    }
};

Halfspace labsHalfspace({ 0, 500 }, { 0, -1 });

void LaunchBall()
{
    balls.emplace_back(launchPosition,
        GetLaunchVelocity(launchAngle, launchSpeed));
}

// Sphere–sphere overlap + separation
void CheckCollisions(std::vector<PhysicsBody>& bodies)
{
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            float distance = Vector2Distance(bodies[i].position,
                bodies[j].position);
            float radiusSum = bodies[i].radius + bodies[j].radius;

            if (distance < radiusSum) {
                bodies[i].color = RED;
                bodies[j].color = RED;

                float overlap = radiusSum - distance;
                Vector2 normal = Vector2Normalize(
                    Vector2Subtract(bodies[j].position, bodies[i].position));
                Vector2 correction = Vector2Scale(normal, overlap * 0.5f);

                if (!bodies[i].isFixed)
                    bodies[i].position =
                    Vector2Subtract(bodies[i].position, correction);
                if (!bodies[j].isFixed)
                    bodies[j].position =
                    Vector2Add(bodies[j].position, correction);
            }
            else {
                bodies[i].color = GREEN;
                bodies[j].color = GREEN;
            }
        }
    }
}

// Sphere–halfspace response (ground)
void ResolveSphereHalfspace(PhysicsBody& sphere, const Halfspace& h)
{
    float distance = Vector2DotProduct(h.normal,
        Vector2Subtract(sphere.position,
            h.point));
    float overlap = sphere.radius - distance;

    if (overlap > 0.0f && !sphere.isFixed) {
        sphere.color = RED;
        sphere.position = Vector2Add(sphere.position,
            Vector2Scale(h.normal, overlap));

        // cancel velocity into plane
        if (Vector2DotProduct(h.normal, sphere.velocity) > 0.0f)
            sphere.velocity = Vector2Zero();
    }
}

void UpdateLabs(float dt)
{
    if (IsKeyPressed(KEY_SPACE))
        LaunchBall();

    sim.deltaTime = dt;
    sim.time += dt;

    for (auto& b : balls)
        sim.UpdateBody(b);

    CheckCollisions(balls);

    for (auto& b : balls)
        ResolveSphereHalfspace(b, labsHalfspace);
}

void DrawLabs()
{
    GuiSliderBar({ 10, 60,  200, 20 }, "Angle",
        TextFormat("%.1f", launchAngle),
        &launchAngle, 0, 90);
    GuiSliderBar({ 10, 90,  200, 20 }, "Speed",
        TextFormat("%.1f", launchSpeed),
        &launchSpeed, 0, 400);
    GuiSliderBar({ 10, 120, 200, 20 }, "Gravity Y",
        TextFormat("%.1f", sim.gravity.y),
        &sim.gravity.y, -500, 500);

    Vector2 v0 = GetLaunchVelocity(launchAngle, launchSpeed);
    Vector2 endPoint = Vector2Add(launchPosition,
        Vector2Scale(v0, 0.2f));

    DrawCircleV(launchPosition, 8, GREEN);
    DrawLineV(launchPosition, endPoint, RED);

    DrawText(TextFormat("Angle: %.1f deg", launchAngle),
        10, 150, 20, LIGHTGRAY);
    DrawText(TextFormat("Speed: %.1f", launchSpeed),
        10, 175, 20, LIGHTGRAY);
    DrawText(TextFormat("Velocity: (%.1f, %.1f)", v0.x, v0.y),
        10, 200, 20, LIGHTGRAY);
    DrawText("Press SPACE to launch balls", 10, 225, 20, LIGHTGRAY);

    for (auto& b : balls)
        DrawCircleV(b.position, b.radius, b.color);

    DrawLine(0, labsHalfspace.point.y,
        GetScreenWidth(), labsHalfspace.point.y, BLUE);
}

// KEY PRACTICE 2: Free Body Diagram 

float fbdMass = 8.0f;
float fbdPlaneAngle = 0.0f;
Vector2 fbdPosition = { 600, 300 };

void DrawFBD()
{
    Vector2 Fgravity = { 0, fbdMass * sim.gravity.y };
    float rad = fbdPlaneAngle * DEG2RAD;

    Vector2 planeDir = Vector2Normalize(Vector2{ cosf(rad), sinf(rad) });
    Vector2 normal = Vector2Normalize(Vector2{ -sinf(rad), cosf(rad) });

    float perp = Vector2DotProduct(Fgravity, normal);
    Vector2 Fnormal = Vector2Scale(normal, -perp);

    float parallel = Vector2DotProduct(Fgravity, planeDir);
    Vector2 Ffriction = Vector2Scale(planeDir, -parallel); // infinite friction

    DrawCircleV(fbdPosition, 30.0f, RED);

    Vector2 FgEnd = Vector2Add(fbdPosition,
        Vector2Scale(Fgravity, 0.04f));
    Vector2 FnEnd = Vector2Add(fbdPosition,
        Vector2Scale(Fnormal, 0.04f));
    Vector2 FfEnd = Vector2Add(fbdPosition,
        Vector2Scale(Ffriction, 0.04f));

    DrawArrow(fbdPosition, FgEnd, PURPLE);
    DrawArrow(fbdPosition, FnEnd, GREEN);
    DrawArrow(fbdPosition, FfEnd, ORANGE);

    DrawText("Fgravity", int(FgEnd.x) + 5, int(FgEnd.y), 16, PURPLE);
    DrawText("Fnormal", int(FnEnd.x) + 5, int(FnEnd.y), 16, GREEN);
    DrawText("Ffriction", int(FfEnd.x) + 5, int(FfEnd.y), 16, ORANGE);

    Vector2 p1 = Vector2Add(fbdPosition,
        Vector2Scale(planeDir, -400));
    Vector2 p2 = Vector2Add(fbdPosition,
        Vector2Scale(planeDir, 400));
    DrawLineV(p1, p2, DARKGREEN);
}

void DrawFBDMode()
{
    GuiSliderBar({ 10, 60,  200, 20 }, "Mass (kg)",
        TextFormat("%.1f", fbdMass),
        &fbdMass, 0.1f, 20.0f);
    GuiSliderBar({ 10, 90,  200, 20 }, "Plane Angle",
        TextFormat("%.1f deg", fbdPlaneAngle),
        &fbdPlaneAngle, -60.0f, 60.0f);
    GuiSliderBar({ 10, 120, 200, 20 }, "Gravity Y",
        TextFormat("%.1f", sim.gravity.y),
        &sim.gravity.y, 10.0f, 400.0f);

    DrawText("Key Practice 2: Sphere on Halfspace (net force = 0, no motion)",
        10, 150, 18, LIGHTGRAY);

    DrawFBD();
}

// LAB 6: Kinetic friction, 4 spheres on a slope

struct SphereBody {
    Vector2 position;  // world position
    float   along;     // scalar along-plane coordinate (relative to mid)
    float   vAlong;    // velocity along plane
    float   mass;
    float   radius;
    float   friction;  // μk
    Color   baseColor;

    SphereBody(float a, float m, float r, float mu, Color c)
        : position{ 0,0 }, along(a), vAlong(0), mass(m),
        radius(r), friction(mu), baseColor(c) {
    }
};

std::vector<SphereBody> lab6Spheres;
float lab6GroundAngle = 0.0f;
float lab6GravityY = 200.0f;

void InitLab6Positions()
{
    float rad = lab6GroundAngle * DEG2RAD;
    Vector2 planeDir = Vector2Normalize(Vector2{ cosf(rad), sinf(rad) });
    Vector2 normal = Vector2Normalize(Vector2{ -sinf(rad), cosf(rad) });
    Vector2 mid = { GetScreenWidth() * 0.3f, 500.0f };

    for (auto& s : lab6Spheres) {
        Vector2 planePoint = Vector2Add(mid,
            Vector2Scale(planeDir, s.along));
        s.position = Vector2Add(planePoint,
            Vector2Scale(normal, -s.radius));
        s.vAlong = 0.0f;
    }
}

void UpdateLab6(float dt)
{
    float rad = lab6GroundAngle * DEG2RAD;
    Vector2 planeDir = Vector2Normalize(Vector2{ cosf(rad), sinf(rad) });
    Vector2 normal = Vector2Normalize(Vector2{ -sinf(rad), cosf(rad) });
    Vector2 mid = { GetScreenWidth() * 0.3f, 500.0f };

    const float minAlong = -200.0f;
    const float maxAlong = 200.0f;

    for (auto& s : lab6Spheres) {
        Vector2 Fg = { 0, s.mass * lab6GravityY };

        float perp = Vector2DotProduct(Fg, normal);
        Vector2 Fnormal = Vector2Scale(normal, -perp);

        float parallel = Vector2DotProduct(Fg, planeDir);
        float frictionMag = s.friction * fabsf(perp);

        float vAlong = s.vAlong;
        Vector2 Ffriction;

        if (fabsf(vAlong) > 1e-3f) {
            float signV = Sign(vAlong);
            Ffriction = Vector2Scale(planeDir, -signV * frictionMag);
        }
        else {
            float signPar = Sign(parallel);
            Ffriction = Vector2Scale(planeDir, -signPar * frictionMag);
        }

        float FgAlong = Vector2DotProduct(Fg, planeDir);
        float FfrictionAlong = Vector2DotProduct(Ffriction, planeDir);
        float netAlong = FgAlong + FfrictionAlong;
        float aAlong = netAlong / s.mass;

        if (fabsf(FgAlong) <= frictionMag && fabsf(vAlong) < 1.0f) {
            aAlong = 0.0f;
            vAlong = 0.0f;
        }
        else {
            vAlong += aAlong * dt;
        }

        float along = s.along + vAlong * dt;

        if (along < minAlong) {
            along = minAlong;
            vAlong = 0.0f;
        }
        else if (along > maxAlong) {
            along = maxAlong;
            vAlong = 0.0f;
        }

        s.along = along;
        s.vAlong = vAlong;

        Vector2 planePoint = Vector2Add(mid, Vector2Scale(planeDir, along));
        s.position = Vector2Add(planePoint, Vector2Scale(normal, -s.radius));
    }
}

void DrawLab6()
{
    GuiSliderBar({ 10, 60,  200, 20 }, "Plane Angle",
        TextFormat("%.1f deg", lab6GroundAngle),
        &lab6GroundAngle, -60.0f, 60.0f);
    GuiSliderBar({ 10, 90,  200, 20 }, "Gravity Y",
        TextFormat("%.1f", lab6GravityY),
        &lab6GravityY, 10.0f, 400.0f);

    DrawText("Lab 6: Kinetic Friction – 4 Spheres on a Slope",
        10, 130, 18, LIGHTGRAY);

    float rad = lab6GroundAngle * DEG2RAD;
    Vector2 planeDir = Vector2Normalize(Vector2{ cosf(rad), sinf(rad) });
    Vector2 normal = Vector2Normalize(Vector2{ -sinf(rad), cosf(rad) });
    Vector2 mid = { GetScreenWidth() * 0.3f, 500.0f };

    Vector2 p1 = Vector2Add(mid, Vector2Scale(planeDir, -400.0f));
    Vector2 p2 = Vector2Add(mid, Vector2Scale(planeDir, 400.0f));
    DrawLineV(p1, p2, DARKGREEN);

    for (auto& s : lab6Spheres) {
        DrawCircleV(s.position, s.radius, s.baseColor);

        Vector2 Fg = { 0, s.mass * lab6GravityY };
        float perp = Vector2DotProduct(Fg, normal);
        Vector2 Fnormal = Vector2Scale(normal, -perp);
        float parallel = Vector2DotProduct(Fg, planeDir);
        float frictionMag = s.friction * fabsf(perp);

        Vector2 Ffriction;
        if (fabsf(s.vAlong) > 1e-3f) {
            float signV = Sign(s.vAlong);
            Ffriction = Vector2Scale(planeDir, -signV * frictionMag);
        }
        else {
            float signPar = Sign(parallel);
            Ffriction = Vector2Scale(planeDir, -signPar * frictionMag);
        }

        Vector2 FgEnd = Vector2Add(s.position,
            Vector2Scale(Fg, 0.04f));
        Vector2 FnEnd = Vector2Add(s.position,
            Vector2Scale(Fnormal, 0.04f));
        Vector2 FfEnd = Vector2Add(s.position,
            Vector2Scale(Ffriction, 0.04f));

        DrawArrow(s.position, FgEnd, PURPLE);
        DrawArrow(s.position, FnEnd, GREEN);
        DrawArrow(s.position, FfEnd, ORANGE);

        Vector2 vWorld = Vector2Scale(planeDir, s.vAlong);
        Vector2 vEnd = Vector2Add(s.position,
            Vector2Scale(vWorld, 5.0f));
        DrawArrow(s.position, vEnd, RED);
    }

    DrawText("Purple: Gravity", 10, 160, 16, PURPLE);
    DrawText("Green: Normal", 10, 180, 16, GREEN);
    DrawText("Orange: Friction", 10, 200, 16, ORANGE);
    DrawText("Red: Velocity", 10, 220, 16, RED);
}


// LAB 7: Collision response, bouncing, restitution

struct Lab7Sphere {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float mass;
    float restitution;
    Color color;
};

enum Lab7Scenario
{
    L7_BOUNCY = 0,
    L7_POOL = 1,
    L7_GALILEAN = 2
};

Lab7Scenario currentL7Scenario = L7_BOUNCY;

Lab7Sphere l7A;
Lab7Sphere l7B;

float l7_massA = 2.0f;
float l7_massB = 2.0f;
float l7_initVelA = 0.0f;
float l7_initVelB = 0.0f;
float l7_restitution = 0.9f;
float l7Gravity = 400.0f;
float l7FloorY = 500.0f;

void ResetLab7()
{
    l7A.mass = l7_massA;
    l7B.mass = l7_massB;
    l7A.restitution = l7_restitution;
    l7B.restitution = l7_restitution;

    switch (currentL7Scenario)
    {
    case L7_BOUNCY:
        l7A.radius = 25.0f;
        l7A.color = RED;
        l7A.position = { (float)GetScreenWidth() * 0.5f, 200.0f };
        l7A.velocity = { 0.0f, l7_initVelA }; // usually 0
        l7B.radius = 0.0f; // unused
        l7B.velocity = Vector2Zero();
        break;

    case L7_POOL:
        l7A.radius = 25.0f;
        l7B.radius = 25.0f;
        l7A.color = RED;
        l7B.color = BLUE;

        l7A.position = { 300.0f, 400.0f };
        l7B.position = { 430.0f, 400.0f };

        l7A.velocity = { l7_initVelA, 0.0f };  // A moves toward B
        l7B.velocity = { l7_initVelB, 0.0f };  // usually 0
        break;

    case L7_GALILEAN:
        l7A.radius = 28.0f; // bottom heavy ball
        l7B.radius = 16.0f; // top lighter ball
        l7A.color = RED;
        l7B.color = BLUE;

        {
            float x = (float)GetScreenWidth() * 0.7f;
            l7A.position = { x, 250.0f };
            l7B.position = { x, 250.0f - (l7A.radius + l7B.radius) };
        }

        l7A.velocity = { 0.0f, l7_initVelA };
        l7B.velocity = { 0.0f, l7_initVelB };
        break;
    }
}

void UpdateLab7(float dt)
{
    Vector2 g = { 0.0f, l7Gravity };

    switch (currentL7Scenario)
    {
    case L7_BOUNCY:
        // gravity
        l7A.velocity = Vector2Add(l7A.velocity, Vector2Scale(g, dt));
        l7A.position = Vector2Add(l7A.position,
            Vector2Scale(l7A.velocity, dt));

        // collide with floor
        if (l7A.position.y + l7A.radius > l7FloorY)
        {
            l7A.position.y = l7FloorY - l7A.radius;
            if (l7A.velocity.y > 0.0f)
                l7A.velocity.y = -l7A.velocity.y * l7A.restitution;
        }
        break;

    case L7_POOL:
    {
        // no gravity; sliding on table
        l7A.position = Vector2Add(l7A.position,
            Vector2Scale(l7A.velocity, dt));
        l7B.position = Vector2Add(l7B.position,
            Vector2Scale(l7B.velocity, dt));

        Vector2 diff = Vector2Subtract(l7B.position, l7A.position);
        float dist = Vector2Length(diff);
        float rsum = l7A.radius + l7B.radius;

        if (dist > 0.0f && dist < rsum)
        {
            Vector2 normal = Vector2Scale(diff, 1.0f / dist);

            float va = Vector2DotProduct(l7A.velocity, normal);
            float vb = Vector2DotProduct(l7B.velocity, normal);

            float ma = l7A.mass;
            float mb = l7B.mass;
            float e = l7_restitution;

            float vaNew = ((ma - e * mb) * va + (1 + e) * mb * vb) / (ma + mb);
            float vbNew = ((mb - e * ma) * vb + (1 + e) * ma * va) / (ma + mb);

            l7A.velocity = Vector2Add(l7A.velocity,
                Vector2Scale(normal, vaNew - va));
            l7B.velocity = Vector2Add(l7B.velocity,
                Vector2Scale(normal, vbNew - vb));

            float overlap = rsum - dist;
            Vector2 correction = Vector2Scale(normal, overlap * 0.5f);
            l7A.position = Vector2Subtract(l7A.position, correction);
            l7B.position = Vector2Add(l7B.position, correction);
        }
    }
    break;

    case L7_GALILEAN:
    {
        // both fall with gravity
        l7A.velocity = Vector2Add(l7A.velocity, Vector2Scale(g, dt));
        l7B.velocity = Vector2Add(l7B.velocity, Vector2Scale(g, dt));
        l7A.position = Vector2Add(l7A.position,
            Vector2Scale(l7A.velocity, dt));
        l7B.position = Vector2Add(l7B.position,
            Vector2Scale(l7B.velocity, dt));

        // bottom ball hits floor
        if (l7A.position.y + l7A.radius > l7FloorY)
        {
            l7A.position.y = l7FloorY - l7A.radius;
            if (l7A.velocity.y > 0.0f)
                l7A.velocity.y = -l7A.velocity.y * l7A.restitution;
        }

        // sphere–sphere collision 
        Vector2 diff = Vector2Subtract(l7B.position, l7A.position);
        float dist = Vector2Length(diff);
        float rsum = l7A.radius + l7B.radius;
        if (dist > 0.0f && dist < rsum)
        {
            Vector2 normal = Vector2Scale(diff, 1.0f / dist);

            float va = Vector2DotProduct(l7A.velocity, normal);
            float vb = Vector2DotProduct(l7B.velocity, normal);

            float ma = l7A.mass;
            float mb = l7B.mass;
            float e = l7_restitution;

            float vaNew = ((ma - e * mb) * va + (1 + e) * mb * vb) / (ma + mb);
            float vbNew = ((mb - e * ma) * vb + (1 + e) * ma * va) / (ma + mb);

            l7A.velocity = Vector2Add(l7A.velocity,
                Vector2Scale(normal, vaNew - va));
            l7B.velocity = Vector2Add(l7B.velocity,
                Vector2Scale(normal, vbNew - vb));

            float overlap = rsum - dist;
            Vector2 correction = Vector2Scale(normal, overlap * 0.5f);
            l7A.position = Vector2Subtract(l7A.position, correction);
            l7B.position = Vector2Add(l7B.position, correction);
        }
    }
    break;
    }
}

void DrawLab7()
{
    DrawText("Lab 7: Collision Response, Bouncing, Restitution",
        10, 130, 18, LIGHTGRAY);
    DrawText("Z: Bouncy ball   X: Pool collision   C: Galilean cannon",
        10, 150, 16, LIGHTGRAY);

    // Scenario selection with keys (Z/X/C)
    if (IsKeyPressed(KEY_Z)) { currentL7Scenario = L7_BOUNCY; ResetLab7(); }
    if (IsKeyPressed(KEY_X)) { currentL7Scenario = L7_POOL;   ResetLab7(); }
    if (IsKeyPressed(KEY_C)) { currentL7Scenario = L7_GALILEAN; ResetLab7(); }

    // Sliders for masses and initial velocities
    GuiSliderBar({ 10, 60,  220, 20 }, "Mass A",
        TextFormat("%.1f", l7_massA),
        &l7_massA, 0.5f, 10.0f);
    GuiSliderBar({ 10, 90,  220, 20 }, "Mass B",
        TextFormat("%.1f", l7_massB),
        &l7_massB, 0.5f, 10.0f);
    GuiSliderBar({ 10, 120, 220, 20 }, "Init Vel A",
        TextFormat("%.1f", l7_initVelA),
        &l7_initVelA, -600.0f, 600.0f);
    GuiSliderBar({ 10, 150, 220, 20 }, "Init Vel B",
        TextFormat("%.1f", l7_initVelB),
        &l7_initVelB, -600.0f, 600.0f);
    GuiSliderBar({ 10, 180, 220, 20 }, "Restitution e",
        TextFormat("%.2f", l7_restitution),
        &l7_restitution, 0.0f, 1.0f);

    if (GuiButton({ 10, 210, 140, 30 }, "Reset Scenario")) {
        ResetLab7();
    }

    // Floor line
    DrawLine(0, (int)l7FloorY, GetScreenWidth(), (int)l7FloorY, DARKGREEN);

    // Draw spheres and velocities / normals
    auto drawSphereDebug = [](const Lab7Sphere& s)
        {
            if (s.radius <= 0.0f) return;
            DrawCircleV(s.position, s.radius, s.color);

            // velocity arrow
            Vector2 vEnd = Vector2Add(s.position,
                Vector2Scale(s.velocity, 0.1f));
            DrawArrow(s.position, vEnd, RED);
        };

    drawSphereDebug(l7A);
    drawSphereDebug(l7B);
}

// MASTER UPDATE / DRAW

void UpdateGame()
{
    float dt = 1.0f / TARGET_FPS;

    if (IsKeyPressed(KEY_ONE))
        currentMode = MODE_LABS;
    if (IsKeyPressed(KEY_SIX))
        currentMode = MODE_LAB6;
    if (IsKeyPressed(KEY_SEVEN))
        currentMode = MODE_LAB7;

    if (IsKeyPressed(KEY_F))
        showFBD = !showFBD;

    if (showFBD)
        return;

    switch (currentMode) {
    case MODE_LABS: UpdateLabs(dt);  break;
    case MODE_LAB6: UpdateLab6(dt);  break;
    case MODE_LAB7: UpdateLab7(dt);  break;
    }
}

void DrawGame()
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("GAME2005 – Arvin Beigi 101447957",
        10, 10, 20, LIGHTGRAY);
    DrawText("1: Labs 1–5   6: Lab 6   7: Lab 7   F: Free Body Diagram",
        10, 30, 18, LIGHTGRAY);

    if (showFBD) {
        DrawFBDMode();
        EndDrawing();
        return;
    }

    switch (currentMode) {
    case MODE_LABS: DrawLabs();  break;
    case MODE_LAB6: DrawLab6();  break;
    case MODE_LAB7: DrawLab7();  break;
    }

    EndDrawing();
}

// MAIN

int main()
{
    InitWindow(InitialWidth, InitialHeight,
        "GAME2005 – Physics Labs, Lab 6 & Lab 7");
    SetTargetFPS(TARGET_FPS);

    // Lab 6 spheres: evenly spaced along slope
    lab6Spheres.push_back(SphereBody(-120.0f, 2.0f, 20.0f, 0.1f, RED));
    lab6Spheres.push_back(SphereBody(-40.0f, 2.0f, 20.0f, 0.8f, GREEN));
    lab6Spheres.push_back(SphereBody(40.0f, 8.0f, 25.0f, 0.1f, BLUE));
    lab6Spheres.push_back(SphereBody(120.0f, 8.0f, 25.0f, 0.8f, YELLOW));
    InitLab6Positions();

    // Lab 7 initial setup
    ResetLab7();

    while (!WindowShouldClose()) {
        UpdateGame();
        DrawGame();
    }

    CloseWindow();
    return 0;
}
