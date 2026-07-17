#include <ccontrol/arena.h>
#include <ccontrol/statespace.h>
#include <ccontrol/tf.h>
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
#define SCREEN_WIDTH 850
#define SCREEN_HEIGHT 450
#define NATURAL_FREQUENCY 8.0f

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
// 2. Global State
// ========================================

size_t current_profile = 1;
float damping_ratio;

ControlStateSpace sys;
ControlHandle ctx;
void *persistent_mem = NULL;
void *scratch_mem = NULL;

float x_data[2];
float u_data[1];
float y_data[1];

// ========================================
// 3. FORWARD DECLARATIONS
// ========================================
void RaylibSetup(void);
void ControlSetup(void);
void ControlLoop(void);
void DrawVisuals(int target_x, int actual_x);
float NextProfile(ControlStateSpace *space);

void ControlSetup()
{
    // 1. Raylib Initialization
    RaylibSetup();

    // 2. CControl Initialization
    static const size_t MEM_SIZE = 256;

    scratch_mem = malloc(sizeof(uint8_t) * MEM_SIZE);
    persistent_mem = malloc(sizeof(uint8_t) * MEM_SIZE);

    ControlArena *s = Control_Arena_Create(scratch_mem, MEM_SIZE);
    ControlArena *p = Control_Arena_Create(persistent_mem, MEM_SIZE);

    Control_System_Init(&ctx, p, s);

    float wn = NATURAL_FREQUENCY;

    DampingProfile profile = profiles[current_profile];
    damping_ratio = profile.zeta;

    // 3. Define Transfer Function (Second Order Standard Form)
    float n[] = {wn * wn};
    float d[] = {1.0f, 2 * damping_ratio * wn, wn * wn};

    ControlVec num = Control_Poly_AllocScratch(&ctx, n, 1);
    ControlVec dem = Control_Poly_AllocScratch(&ctx, d, 3);

    ControlTransferFunction tf = Control_TF_FromPoly(&num, &dem);

    // 4. State Space Representation
    sys = Control_StateSpace_FromTF(&ctx, &tf);

    x_data[0] = 100.0f / (NATURAL_FREQUENCY * NATURAL_FREQUENCY);
    x_data[1] = 0.0f;
    u_data[0] = SCREEN_WIDTH / 2.0f;
    y_data[0] = 100.0f;

    sys.x = (ControlVec){.coeffs = x_data, .size = 2};
    sys.u = (ControlVec){.coeffs = u_data, .size = 1};
    sys.y = (ControlVec){.coeffs = y_data, .size = 1};
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

#ifndef CCONTROL_EXAMPLE_SHOWCASE
    // ----------------------------------------
    // [INPUT] Mouse acts as the reference target (Step Input)
    // ----------------------------------------
    sys.u.coeffs[0] = (float)GetMouseX();
#endif

    // ----------------------------------------
    // [SYSTEM CONFIG] Update Damping profile
    // ----------------------------------------
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE))
    {
        damping_ratio = NextProfile(&sys);
    }

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

int main(void)
{
    ControlSetup();

    // 5. Game Loop
#ifdef PLATFORM_WEB
    emscripten_set_main_loop(ControlLoop, 0, 1);
#else

    while (!WindowShouldClose())
    {
        ControlLoop();
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
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;
    InitWindow(screenWidth, screenHeight, "ccontrol + raylib : State Space System");
    SetTargetFPS(60);
    HideCursor();
}

void DrawVisuals(int target_x, int actual_x)
{

    BeginDrawing();
    {
        ClearBackground(RAYWHITE);

        DrawLine(target_x, 0, target_x, SCREEN_HEIGHT, LIGHTGRAY);
        DrawText("Target (Mouse)", target_x + 5, 10, 20, GRAY);

        DrawRectangle(actual_x - 20, SCREEN_HEIGHT / 2 - 20, 40, 40, BLUE);
        DrawText("System Output", actual_x - 50, SCREEN_HEIGHT / 2 + 30, 20, MAROON);

        DrawText(
            "Use Up/Right Arrows to change the physical system", 10, SCREEN_HEIGHT - 80, 20, GRAY);

        DrawText(TextFormat("Mode: %s", profiles[current_profile].name),
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

float NextProfile(ControlStateSpace *sys)
{
    current_profile = (current_profile + 1) % NUM_PROFILES;
    DampingProfile p = profiles[current_profile];
    float new_zeta = p.zeta;

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
