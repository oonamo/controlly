#include <controlly/arena.h>
#include <controlly/core.h>
#include <controlly/matrix.h>
#include <controlly/statespace.h>
#include <controlly/tf.h>
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct
{
    float zeta;
    char *name;
} DampingProfile;

#define NUM_PROFILES 4
static const DampingProfile profiles[NUM_PROFILES] = {
    {0.0f, "Undamped (Marginal Stability)"},
    {0.3f, "Undamped (Overshoot)"},
    {1.0f, "Critically Damped"},
    {3.0f, "Overdamped (Slow)"},
};

// ========================================
// 2. FORWARD DECLARATIONS
// ========================================
void  RaylibSetup(void);
void  ControlSetup(void);
void  ControlLoop(void);
void  MainLoop(void);
void  DrawVisuals(int target_x, int actual_x);
float NextProfile(ControlStateSpace *space);
float PrevProfile(ControlStateSpace *space);

// ========================================
// 3. Global State
// ========================================

size_t current_profile = 1;

// [DOC_START: statespace_example]
#define NATURAL_FREQUENCY 8.0f

float             damping_ratio;
ControlStateSpace sys;
ControlHandle     ctx;
void             *persistent_mem = NULL;
void             *scratch_mem    = NULL;

// If any call to the Controlly library fails, this function will be called before returning
void Control_ExitOnFailure(ControlResult result, const char *message, void *user_data)
{
    (void)user_data;
    if (result != CONTROL_OK)
    {
        TraceLog(LOG_FATAL, "Exiting, Reason: %s", message);
        exit(EXIT_FAILURE);
    }
}

void ControlSetup()
{
    // Initialize Memory Arenas
    static const size_t MEM_SIZE = 256;

    // Allocate raw backing buffers (can be statically allocated)
    scratch_mem    = malloc(sizeof(uint8_t) * MEM_SIZE);
    persistent_mem = malloc(sizeof(uint8_t) * MEM_SIZE);

    // Bind buffers to the arena trackers
    ControlArena *s = Control_Arena_Create(scratch_mem, MEM_SIZE);
    ControlArena *p = Control_Arena_Create(persistent_mem, MEM_SIZE);

    Control_System_Init(&ctx, p, s);
    ctx.on_error = Control_ExitOnFailure;

    float wn = NATURAL_FREQUENCY;

    // Get the selected behavior
    DampingProfile profile = profiles[current_profile];
    damping_ratio          = profile.zeta;

    // Define Transfer Function (Second Order Standard Form)
    float      n[] = {wn * wn};
    float      d[] = {1.0f, 2 * damping_ratio * wn, wn * wn};
    ControlVec num = {0};
    ControlVec dem = {0};

    Control_Poly_AllocScratch(&ctx, &num, n, 1);
    Control_Poly_AllocScratch(&ctx, &dem, d, 3);

    ControlTransferFunction tf = {0};

    Control_TF_FromPoly(&ctx, &tf, &num, &dem);

    // State Space Representation
    Control_StateSpace_FromTF(&ctx, &sys, &tf);

    // Initialize System State
    // Set initial visual position to y = 100
    // In the canonical form, the output is y = (wn^2) * x1
    // To get y = 100, we need to set the first state to be 100 / wn^2
    float x_data[2] = {100.0f / (NATURAL_FREQUENCY * NATURAL_FREQUENCY), 0.0f};
    float y_data[1] = {SCREEN_WIDTH / 2.0f}; // Initial reference
    float u_data[1] = {0};                   // Initial output

    // Allocate state, input, and output vectors in the Persistent arena
    Control_Poly_AllocPersistent(&ctx, &sys.x, x_data, 2);
    Control_Poly_AllocPersistent(&ctx, &sys.y, y_data, 2);
    Control_Poly_AllocPersistent(&ctx, &sys.u, u_data, 1);
}

void ControlLoop()
{
    // ----------------------------------------
    // [TIME] Get time between last iteration
    // ----------------------------------------
    float dt = GetFrameTime();
    if (dt > 0.1)
    {
        dt = 0.1f;
    }

#ifndef CONTROLLY_EXAMPLE_SHOWCASE
    // ----------------------------------------
    // [INPUT] Mouse acts as the reference target (Step Input)
    // ----------------------------------------
    sys.u.coeffs[0] = (float)GetMouseX();
#endif

    // ----------------------------------------
    // [STEP] Calculate the next step to the mathematical system
    // ----------------------------------------
    Control_StateSpace_StepContinuous(&ctx, &sys, dt);

    int target_x = sys.u.coeffs[0];
    int actual_x = sys.y.coeffs[0];

    // ----------------------------------------
    // [VISUALIZATION] Draw the simulated frame
    // ----------------------------------------
    DrawVisuals(target_x, actual_x);
}
// [DOC_END: statespace_example]

void MainLoop(void)
{
    // ----------------------------------------
    // [SYSTEM CONFIG] Update Damping profile
    // ----------------------------------------
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE) ||
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        damping_ratio = NextProfile(&sys);
    }
    else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN) ||
             IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        damping_ratio = PrevProfile(&sys);
    }
    ControlLoop();
}

int main(void)
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

    // 6. Cleanup
    CloseWindow();

    Control_System_DeInit(&ctx);
    free(scratch_mem);
    free(persistent_mem);

    return 0;
}

// ========================================
// 4. UI & Simulation Implementation
// ========================================
void RaylibSetup()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "controlly + raylib : State Space System");
    SetTargetFPS(60);
    SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
}

void DrawVisuals(int target_x, int actual_x)
{

    BeginDrawing();
    {
        ClearBackground(RAYWHITE);

        DrawLine(target_x, 0, target_x, SCREEN_HEIGHT, LIGHTGRAY);
        DrawText("Target (Mouse)", target_x + 5, 10, 20, GRAY);

        DrawRectangle(actual_x - 20, SCREEN_HEIGHT / 2 - 20, 40, 40, MAROON);
        DrawText("System Output", actual_x - 50, SCREEN_HEIGHT / 2 + 30, 20, MAROON);

        DrawText("UP/RIGHT/RIGHT Click: Next System", 10, SCREEN_HEIGHT - 120, 20, GRAY);

        DrawText("DOWN/LEFT/LEFT Click: Previous System", 10, SCREEN_HEIGHT - 80, 20, GRAY);

        DrawText(TextFormat("Mode: %s [%d/%d]",
                            profiles[current_profile].name,
                            current_profile + 1,
                            NUM_PROFILES),
                 10,
                 SCREEN_HEIGHT - 50,
                 20,
                 DARKBLUE);
        DrawText(TextFormat("Damping Ratio (Zeta): %.2f", profiles[current_profile].zeta),
                 10,
                 SCREEN_HEIGHT - 25,
                 20,
                 DARKGRAY);
    }
    EndDrawing();
}

float PrevProfile(ControlStateSpace *sys)
{
    current_profile         = current_profile != 0 ? current_profile - 1 : NUM_PROFILES - 1;
    DampingProfile p        = profiles[current_profile];
    float          new_zeta = p.zeta;

    // Reset state space vectors
    sys->x.coeffs[0] = 100.0f / (NATURAL_FREQUENCY * NATURAL_FREQUENCY);
    sys->x.coeffs[1] = 0.0f;
    sys->u.coeffs[0] = SCREEN_WIDTH / 2.0f;
    sys->y.coeffs[0] = 100.0f;

    // Update A matrix
    // A(1,1) is the only index that uses the NATURAL_FREQUENCY in its calculation
    sys->A.data[3] = -2.0f * new_zeta * NATURAL_FREQUENCY;

    return new_zeta;
}

float NextProfile(ControlStateSpace *sys)
{
    current_profile         = (current_profile + 1) % NUM_PROFILES;
    DampingProfile p        = profiles[current_profile];
    float          new_zeta = p.zeta;

    // Reset state space vectors
    sys->x.coeffs[0] = 100.0f / (NATURAL_FREQUENCY * NATURAL_FREQUENCY);
    sys->x.coeffs[1] = 0.0f;
    sys->u.coeffs[0] = SCREEN_WIDTH / 2.0f;
    sys->y.coeffs[0] = 100.0f;

    // Update A matrix
    // A(1,1) is the only index that uses the NATURAL_FREQUENCY in its calculation
    sys->A.data[3] = -2.0f * new_zeta * NATURAL_FREQUENCY;

    return new_zeta;
}
