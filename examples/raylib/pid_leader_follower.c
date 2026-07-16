/*
 * Cruise Control Example
 * TODO: Link to derivations
 */
#include <ccontrol/controllers/pid.h>
#include <ccontrol/tf.h>
#include <math.h>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define SCREEN_WIDTH 850
#define SCREEN_HEIGHT 450
#define FLOOR_HEIGHT SCREEN_HEIGHT * 0.6
#define VEHICLE_HEIGHT 50
#define VEHICLE_WIDTH 100

// ========================================
// 1. Physical Models
// ========================================
typedef struct
{
    Rectangle bounds;
    float acceleration;
    float velocity;
    float mass;
    float friction;
} Vehicle;

static Vehicle leader = {
    .bounds = {SCREEN_WIDTH * 0.5, FLOOR_HEIGHT - VEHICLE_HEIGHT, VEHICLE_WIDTH, VEHICLE_HEIGHT},
    .acceleration = 0.0f,
    .velocity = 100.0f,
    .mass = 1500.0f,
    .friction = 50.0f};

static Vehicle follower = {
    .bounds = {-100, FLOOR_HEIGHT - VEHICLE_HEIGHT, VEHICLE_WIDTH, VEHICLE_HEIGHT},
    .acceleration = 0.0f,
    .velocity = 0.0f,
    .mass = 1000.0f,
    .friction = 50.0f};

// ========================================
// 2. Simulation Details (Implemented below main)
// ========================================
Camera2D camera = {0};
static const char *collision_msg = "Vehicles Are Colliding!";
static const char *braking_msg = "BRAKING!";
float collision_msg_len = 0.0f;
float braking_msg_len = 0.0f;

void RaylibSetup();
void UpdateLeaderPosition(float dt);
void DrawVisuals(float cur_dist, float target, ControlPIDController *pid);
void CalculateZoom(void);
void DrawVehicle(Vehicle *car, Color color);
void DrawSkyline(void);
bool VehiclesAreColliding(void);

int main()
{
    // Simulation boilerplate, unneeded for Control Loop
    RaylibSetup();

    // Initialize PID Controller
    ControlPIDController pid = {0};
    float kp = 0;
    float ki = 0;
    float kd = 0;

    Control_PID_Init(&pid, kp, ki, kd, NULL);

    static const float TARGET_DISTANCE = 150.0f;

    while (!WindowShouldClose())
    {

        // ----------------------------------------
        // [TIME] Get time between last iteration
        // ----------------------------------------
        float dt = GetFrameTime();
        if (dt > 0.1f)
        {
            dt = 0.1f;
        }

        // ----------------------------------------
        // [ENVIROMENT] Updates how the lead car drives
        // ----------------------------------------
        UpdateLeaderPosition(dt);

        // ----------------------------------------
        // [INPUT] The physical state of our system, being the distance
        // ----------------------------------------
        float current_distance = leader.bounds.x - (follower.bounds.x + follower.bounds.width);

        // ----------------------------------------
        // [CONTROLLER] Computes the corrective force
        // ----------------------------------------
        float corrective_force = Control_PID_Update(&pid, current_distance, TARGET_DISTANCE, dt);

        // ----------------------------------------
        // [PLANT] Physics of our system
        // ----------------------------------------
        // 1. Calculate drag proportional to the velocity of the vehicle
        float drag = follower.friction * follower.velocity;

        // 2. Apply Newton's 2nd Law: a = F_total / m_total
        //    F_total = corrective_force - drag (Drag counteracts the direction of corrective force)
        follower.acceleration = (corrective_force - drag) / follower.mass;

        // 3. Apply Eueler integration
        follower.velocity += follower.acceleration * dt;
        follower.bounds.x += follower.velocity * dt;

        // ----------------------------------------
        // [VISUALIZATION] Draw the simulated frame
        // ----------------------------------------
        DrawVisuals(current_distance, TARGET_DISTANCE, &pid);
    }

    CloseWindow();
}

void RaylibSetup()
{
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;
    camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    camera.target.y = 0.0f;
    InitWindow(screenWidth, screenHeight, "ccontrol + raylib : Adapative Cruise Control");
    SetTargetFPS(60);

    collision_msg_len = MeasureText(collision_msg, 32);
    braking_msg_len = MeasureText(braking_msg, 32);
}

void UpdateLeaderPosition(float dt)
{
    float next_leader_velocity = 150.0f + 20.0f * fabs(sinf(GetTime() * 0.5f));
    if (IsKeyDown(KEY_SPACE))
    {
        next_leader_velocity = 0.0f;
    }
    leader.acceleration = -(float)(leader.velocity > next_leader_velocity);
    leader.velocity = next_leader_velocity;

    leader.bounds.x += leader.velocity * dt;
}

void DrawVisuals(float cur_dist, float target, ControlPIDController *pid)
{
    CalculateZoom();
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        {
            DrawRectangle(camera.target.x - SCREEN_WIDTH,
                          FLOOR_HEIGHT,
                          SCREEN_WIDTH * 4,
                          SCREEN_HEIGHT * 0.5,
                          GRAY);

            DrawSkyline();

            DrawVehicle(&leader, PURPLE);
            DrawVehicle(&follower, BLUE);

            Vector2 start_line = {follower.bounds.x + follower.bounds.width, FLOOR_HEIGHT - 10.0f};
            Vector2 end_line = {leader.bounds.x, start_line.y};

            Color color = GREEN;
            if (start_line.x > end_line.x)
            {
                color = RED;
            }

            DrawLineEx(start_line, end_line, 3.0f, color);
        }
        EndMode2D();

        DrawText(TextFormat("Distance: %.1f px", cur_dist), 10, 10, 20, BLACK);
        DrawText(TextFormat("Target: %.1f px", target), 10, 40, 20, DARKGREEN);
        DrawText(TextFormat("Follower Vel: %.1f px/ms", follower.velocity), 10, 70, 20, BLUE);
        DrawText(TextFormat("Leader Vel: %.1f px/ms", leader.velocity), 10, 100, 20, RED);

        static const float padding = 10;
        static const float panelX = SCREEN_WIDTH - 250;
        static const Rectangle window_box = {
            .x = panelX - padding,
            .y = 10,
            .width = 250,
            .height = 130,
        };

        static const float slider_width = (window_box.width - 2.0f * padding) * 0.8f;
        static const float slider_x = panelX + (padding * 2.0f);

        GuiPanel(window_box, "PID Control Panel");

        // TODO: Redudant with versions above RAYGUI 4.0
        // Newer versions automatically check if value has been updated!
        float old_kp = pid->kp;
        float old_ki = pid->ki;
        float old_kd = pid->kd;

        GuiSliderBar((Rectangle){slider_x, 50, slider_width, 20},
                     "Kp",
                     TextFormat("%.1f", pid->kp),
                     &pid->kp,
                     0.0f,
                     1000.0f);

        GuiSliderBar((Rectangle){slider_x, 80, slider_width, 20},
                     "Ki",
                     TextFormat("%.1f", pid->ki),
                     &pid->ki,
                     0.0f,
                     100.0f);

        GuiSliderBar((Rectangle){slider_x, 110, slider_width, 20},
                     "Kd",
                     TextFormat("%.1f", pid->kd),
                     &pid->kd,
                     0.0f,
                     500.0f);

        if (VehiclesAreColliding())
        {
            float center = SCREEN_WIDTH / 2.0 - (collision_msg_len / 2.0);
            DrawText(collision_msg, center, FLOOR_HEIGHT + 80, 32, YELLOW);
        }
        if (IsKeyDown(KEY_SPACE))
        {
            float center = SCREEN_WIDTH / 2.0 - (braking_msg_len / 2.0);
            DrawText(braking_msg, center, 80, 32, RED);
        }

        if (pid->kp != old_kp || pid->ki != old_ki || pid->kd != old_kd)
        {
            Control_PID_Reset(pid);
        }
    }
    EndDrawing();
}

void CalculateZoom()
{
    float leader_center = leader.bounds.x + (leader.bounds.width / 2.0f);
    float follower_center = follower.bounds.x + (follower.bounds.width / 2.0f);

    // Center camera
    camera.target.x = (leader_center + follower_center) / 2.0f;

    camera.target.y = FLOOR_HEIGHT - 100.0f;

    float dist = fabs(leader_center - follower_center);

    float padding = 300.0f;
    float view_width = dist + padding;

    camera.zoom = SCREEN_WIDTH / view_width;

    if (camera.zoom > 1.0f)
    {
        camera.zoom = 1.0f;
    }
    else if (camera.zoom < 0.3f)
    {
        camera.zoom = 0.3f;
    }
}

void DrawVehicle(Vehicle *car, Color color)
{

    DrawRectangleRec(car->bounds, color);

    // Draw Wheels
    Vector2 rear_wheel = {car->bounds.x + 20, car->bounds.y + car->bounds.height};
    Vector2 front_wheel = {car->bounds.x + car->bounds.width - 20, rear_wheel.y};

    DrawCircleV(rear_wheel, 12.0f, BLACK);
    DrawCircleV(front_wheel, 12.0f, BLACK);

    // Draw Brake lights
    if (car->acceleration < 0 || car->velocity == 0.0f)
    {
        Rectangle brake_light = {
            .x = car->bounds.x,
            .y = car->bounds.y + car->bounds.height * 0.2,
            .width = 10.0f,
            .height = car->bounds.height * 0.4f,
        };

        DrawRectangleRec(brake_light, RED);
    }
}

void DrawSkyline()
{
    float view_width = GetRenderWidth();
    float start_x = camera.target.x - view_width;
    float end_x = camera.target.x + view_width;

    static const int grid_size = 150;
    int snapped_start = ((int)start_x / grid_size) * grid_size;

    for (int x = snapped_start; x < end_x; x += grid_size)
    {
        int pseudo_random = abs(x * 12345);
        float bldg_height = 50.0f + (pseudo_random % 200);
        float bldg_width = 40.0f + (pseudo_random % 60);

        Rectangle building = {.x = (float)x,
                              .y = FLOOR_HEIGHT - bldg_height,
                              .width = bldg_width,
                              .height = bldg_height};

        // Draw Buildings
        DrawRectangleRec(building, LIGHTGRAY);
        DrawRectangleLinesEx(building, 2.0f, GRAY);

        // Draw Lanes
        Rectangle lane = {
            .x = (float)x,
            .y = FLOOR_HEIGHT + 10,
            .width = grid_size * 0.5,
            .height = 10,
        };
        DrawRectangleRec(lane, WHITE);
    }
}

bool VehiclesAreColliding()
{
    return CheckCollisionRecs(leader.bounds, follower.bounds);
}
