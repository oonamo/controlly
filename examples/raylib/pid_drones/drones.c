#include <controlly/arena.h>
#include <controlly/controllers/pid.h>
#include <controlly/core.h>
#include <controlly/matrix.h>
#include <controlly/statespace.h>
#include <controlly/tf.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// ========================================
// 1. CONFIGURATION & MODEL
// ========================================
#if defined(PLATFORM_WEB)
    #define SCREEN_WIDTH 800
    #define SCREEN_HEIGHT 600
#else
    #define SCREEN_WIDTH 1250
    #define SCREEN_HEIGHT 750
#endif

void RaylibSetup(void);
void ControlSetup(void);
void ControlLoop(void);
void MainLoop(void);
void DrawVisuals();

/// [DOC_START: drone_example]
typedef enum
{
    DRONE_CHASER,
    DRONE_WATCHER,
} DroneType;

typedef struct
{
    DroneType type;
    Vector2   pos;
    Vector2   vel;
    Vector2   target;
    float     kp, ki, kd;
    union
    {
        struct
        {
            ControlPIDController pid_x; // PID for X axis
            ControlPIDController pid_y; // PID for Y axis
        };
        struct
        {
            ControlStateSpace ss;       // State Space Model for Radial PID
            float             prev_pos; // Previous drone position
        };
    };
    float prev_dist;
} Drone;

#define TARGET_RADIUS 125.0f

static ControlHandle ctx = {0};
static uint8_t       scratch_mem[4096];
static uint8_t       persistent_mem[4096];

// If any call to the Controlly library fails, this function will be called before returning
void Control_ExitOnFailure(ControlResult result,
                           const char   *message,
                           const char   *verbose_data,
                           void         *user_data)
{
    (void)user_data;
    if (result != CONTROL_OK)
    {
        if (verbose_data != NULL)
        {
            TraceLog(LOG_FATAL, "Controlly Error [%s]: %s", verbose_data, message);
        }
        else
        {
            TraceLog(LOG_FATAL, "Controlly Error: %s", message);
        }
        exit(EXIT_FAILURE);
    }
}

void DroneInitWatcher(Drone *drone)
{
    drone->ss        = CONTROL_EMPTY_STATESPACE;
    drone->kp        = 64.0f;
    drone->kd        = 16.0f;
    Vector2 diff     = Vector2Subtract(drone->pos, drone->target);
    drone->prev_dist = Vector2Length(diff);

    Control_Matrix_AllocPersistent(&ctx, &drone->ss.A, 2, 2);
    Control_Matrix_AllocPersistent(&ctx, &drone->ss.B, 2, 1);
    Control_Matrix_AllocPersistent(&ctx, &drone->ss.C, 2, 2);
    Control_Matrix_AllocPersistent(&ctx, &drone->ss.D, 2, 1);

    drone->ss.A.data[0] = 0.0f;
    drone->ss.A.data[1] = 1.0f;
    drone->ss.A.data[2] = 0.0f;
    drone->ss.A.data[3] = 0.0f;

    drone->ss.B.data[0] = 0.0f;
    drone->ss.B.data[1] = 1.0f;

    drone->ss.C.data[0] = 1.0f;
    drone->ss.C.data[1] = 0.0f;
    drone->ss.C.data[2] = 0.0f;
    drone->ss.C.data[3] = 1.0f;

    drone->ss.D.data[0] = 0.0f;
    drone->ss.D.data[1] = 0.0f;

    float x_data[2] = {0.0f, 0.0f};
    float y_data[2] = {0.0f, 0.0f};
    float u_data[1] = {0.0f};

    Control_Poly_AllocPersistent(&ctx, &drone->ss.x, x_data, 2);
    Control_Poly_AllocPersistent(&ctx, &drone->ss.y, y_data, 2);
    Control_Poly_AllocPersistent(&ctx, &drone->ss.u, u_data, 1);
}

void DroneInitChaser(Drone *drone)
{
    // Prevent integral windup causing system to initially pause on target
    static ControlPIDConfig cfg = {
        .enable_anti_windup = true, .max_out = 1000.0f, .min_out = -1000.0f};

    Control_PID_Init(&drone->pid_x, drone->kp, drone->ki, drone->kd, &cfg);
    Control_PID_Init(&drone->pid_y, drone->kp, drone->ki, drone->kd, &cfg);
}

/**
 * Updates the chaser drone using 2 PID Controllers (one for each coordinate)
 * The output is treated as a velocity, resulting in instant feedback and minimal error
 */
void DroneUpdateChaser(Drone *drone, float dt)
{
    drone->vel.x = Control_PID_Update(&drone->pid_x, drone->target.x, drone->pos.x, dt);
    drone->vel.y = Control_PID_Update(&drone->pid_y, drone->target.y, drone->pos.y, dt);
}

/**
 * Updates the watcher drone using Feedback Linearization.
 * The controlly state-space model receives the error between the distance and target radius, and
 * outputs the next expected error.
 *
 * The linear control effort is in the drone->ss.u vector and is modeled as PD controller.
 * Then, it calculates the non-linear force (centrifugal acceleration) for the 2D physics
 */
void DroneUpdateWatcher(Drone *drone, float dt)
{
    ControlStateSpace *sys = &drone->ss;

    Vector2 diff = Vector2Subtract(drone->pos, drone->target);
    float   dist = Vector2Length(diff);

    float nx                = 1.0f;
    float ny                = 0.0f;
    float centrifugal_force = 0.0f;
    float u_tan_x           = 0.0f;
    float u_tan_y           = 0.0f;

    if (dist >= 0.001f)
    {
        nx = diff.x / dist;
        ny = diff.y / dist;
    }

    float r_dot = (drone->vel.x * nx) + (drone->vel.y * ny);

    if (dist >= 0.001f)
    {
        float tan_vel_x = drone->vel.x - (r_dot * nx);
        float tan_vel_y = drone->vel.y - (r_dot * ny);

        float tan_vel_sq  = (tan_vel_x * tan_vel_x) + (tan_vel_y * tan_vel_y);
        centrifugal_force = tan_vel_sq / dist;

        float tangental_damping = 4.0f;
        u_tan_x                 = -tan_vel_x * tangental_damping;
        u_tan_y                 = -tan_vel_y * tangental_damping;
    }

    float dist_err = dist - TARGET_RADIUS;

    sys->x.coeffs[0] = dist_err;
    sys->x.coeffs[1] = r_dot;

    sys->u.coeffs[0] = -(drone->kp * sys->x.coeffs[0] + drone->kd * sys->x.coeffs[1]);

    Control_StateSpace_StepContinuous(&ctx, sys, dt);

    float v_r = sys->u.coeffs[0];
    float u_r = v_r - centrifugal_force;

    drone->vel.x += ((u_r * nx) + u_tan_x) * dt;
    drone->vel.y += ((u_r * ny) + u_tan_y) * dt;
}

void DroneInit(Drone *drone, Vector2 start_pos)
{
    drone->pos    = start_pos;
    drone->target = (Vector2){0.0f, 0.0f};
    drone->vel    = (Vector2){0.0f, 0.0f};
    drone->kp     = 5.0f;
    drone->ki     = 10.0f;
    drone->kd     = 0.0f;

    if (drone->type == DRONE_WATCHER)
    {
        DroneInitWatcher(drone);
    }
    else if (drone->type == DRONE_CHASER)
    {
        DroneInitChaser(drone);
    }
}

void DroneUpdate(Drone *drone, float dt)
{
    if (drone->type == DRONE_CHASER)
    {
        DroneUpdateChaser(drone, dt);
    }
    else if (drone->type == DRONE_WATCHER)
    {
        DroneUpdateWatcher(drone, dt);
    }

    drone->pos.x += drone->vel.x * dt;
    drone->pos.y += drone->vel.y * dt;
}

static Drone chaser  = {.type = DRONE_CHASER};
static Drone watcher = {.type = DRONE_WATCHER};

void ControlSetup()
{
    ControlArena *persistent_arena = Control_Arena_Create(persistent_mem, 4096);
    ControlArena *scratch_arena    = Control_Arena_Create(scratch_mem, 4096);

    Control_System_Init(&ctx, persistent_arena, scratch_arena);
    ctx.on_error = Control_ExitOnFailure;

    DroneInit(&chaser, (Vector2){0.0f, 0.0f});
    DroneInit(&watcher, (Vector2){0.0f, 0.0f});
}

void ControlLoop()
{
    float dt = GetFrameTime();
    if (dt > 0.1 || dt <= 0.00001f)
    {
        dt = 0.00001f;
    }

    Vector2 target = GetMousePosition();

    chaser.target  = target;
    watcher.target = target;

    DroneUpdate(&chaser, dt);
    DroneUpdate(&watcher, dt);

    DrawVisuals();
}

/// [DOC_END: drone_example]

#define DRONE_RADIUS 20.0f

void DrawVisuals()
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        DrawCircleV(chaser.pos, DRONE_RADIUS, BLUE);
        DrawCircleV(watcher.pos, DRONE_RADIUS, RED);

        float actual_error = Vector2Distance(watcher.target, watcher.pos) - TARGET_RADIUS;

        const char *watcher_text       = TextFormat("ERROR: %.2f", actual_error);
        int         watcher_text_width = MeasureText(watcher_text, (int)DRONE_RADIUS);

        int watcher_start_pos = watcher.pos.x - watcher_text_width / 2.0;
        DrawText(watcher_text, watcher_start_pos, watcher.pos.y - 40, 20, RED);

        const char *chaser_text       = TextFormat("ERROR: %.2f", actual_error);
        int         chaser_text_width = MeasureText(chaser_text, (int)DRONE_RADIUS);

        int chaser_start_pos = chaser.pos.x - chaser_text_width / 2.0;
        DrawText(chaser_text, chaser_start_pos, chaser.pos.y - 40, 20, BLUE);

        DrawCircleLinesV(GetMousePosition(), TARGET_RADIUS, RED);
    }
    EndDrawing();
}

void RaylibSetup()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "controlly + raylib : Drone Chaser + Watcher");
    SetTargetFPS(60);
    SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
}

void MainLoop()
{
    if (IsKeyPressed(KEY_R))
    {
        watcher.pos = (Vector2){0.0f, 0.0f};
        chaser.pos  = (Vector2){0.0f, 0.0f};
    }
    ControlLoop();
}

int main()
{
    RaylibSetup();
    ControlSetup();
#ifdef PLATFORM_WEB
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    while (!WindowShouldClose())
    {
        MainLoop();
    }
#endif

    CloseWindow();
    return 0;
}
