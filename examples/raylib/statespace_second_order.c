#include "ccontrol/arena.h"
#include "ccontrol/tf.h"
#include "raylib.h"
#include <ccontrol/statespace.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

typedef struct
{
    float zeta;
    char *name;
} DampingProfile;

#define SCREEN_WIDTH 850
#define SCREEN_HEIGHT 450
#define NATURAL_FREQUENCY 8.0f

#define NUM_PROFILES 4

static const DampingProfile profiles[NUM_PROFILES] = {
    {0.0f, "Undamped (Marginal Stability)"},
    {0.3f, "Undamped (Overshoot)"},
    {1.0f, "Critically Damped"},
    {3.0f, "Overdamped (Slow)"},
};

size_t current_profile = 1;
float damping_ratio;
ControlStateSpace sys;
ControlHandle ctx;

void UpdateDrawFrame(void);
float NextProfile(ControlStateSpace *space);

int main(void)
{
    // 1. Raylib Initialization
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;
    InitWindow(screenWidth, screenHeight,
               "ccontrol + raylib : Mass-Spring-Damper");
    SetTargetFPS(60);
    HideCursor();

    // 2. CControl Initialization
    static const size_t MEM_SIZE = 256;

    void *scratch_mem = malloc(sizeof(uint8_t) * MEM_SIZE);
    void *persistent_mem = malloc(sizeof(uint8_t) * MEM_SIZE);

    ControlArena *s = Control_Arena_Create(scratch_mem, MEM_SIZE);
    ControlArena *p = Control_Arena_Create(persistent_mem, MEM_SIZE);

    Control_System_Init(&ctx, p, s);

    float wn = NATURAL_FREQUENCY;

    DampingProfile profile = profiles[current_profile];
    damping_ratio = profile.zeta;

    // 3. Define Transfer Function
    // Use the Second-Order Standard Form
    float n[] = {wn * wn};
    float d[] = {1.0f, 2 * damping_ratio * wn, wn * wn};

    ControlVec num = Control_Poly_AllocScratch(&ctx, n, 1);
    ControlVec dem = Control_Poly_AllocScratch(&ctx, d, 3);

    ControlTransferFunction tf = Control_TF_FromPoly(&num, &dem);

    // 4. State Space Representation
    sys = Control_StateSpace_FromTF(&ctx, &tf);

    // StateSpace owns its data, it is safe to delete
    Control_Arena_Clear(s);

    float x_data[] = {100.0f / (wn * wn), 0.0f}; // Start in middle of screen
    float u_data[] = {screenWidth / 2.0f};
    float y_data[] = {100};

    sys.x = (ControlVec){.coeffs = x_data, .size = 2};
    sys.u = (ControlVec){.coeffs = u_data, .size = 1};
    sys.y = (ControlVec){.coeffs = y_data, .size = 1};

    // 5. Game Loop
#ifdef PLATFORM_WEB
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    // 6. Cleanup
    CloseWindow();

    Control_System_DeInit(&ctx);
    free(scratch_mem);
    free(persistent_mem);

    return 0;
}

void UpdateDrawFrame()
{
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP))
    {
        damping_ratio = NextProfile(&sys);
    }

    float dt = GetFrameTime();

#ifndef CCONTROL_EXAMPLE_SHOWCASE
    sys.u.coeffs[0] = (float)GetMouseX();
#endif

    // C. Iterate over the system
    Control_StateSpace_StepContinuous(&ctx, &sys, dt);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    int target_x = (int)sys.u.coeffs[0];
    int actual_x = (int)sys.y.coeffs[0];

    DrawLine(target_x, 0, target_x, SCREEN_HEIGHT, LIGHTGRAY);
    DrawText("Target (Mouse)", target_x + 5, 10, 20, GRAY);

    DrawRectangle(actual_x - 20, SCREEN_HEIGHT / 2 - 20, 40, 40, MAROON);
    DrawText("System Output", actual_x - 50, SCREEN_HEIGHT / 2 + 30, 20,
             MAROON);

    DrawText("Use Up/Right Arrows to change the physical system", 10,
             SCREEN_HEIGHT - 80, 20, GRAY);

    DrawText(TextFormat("Mode: %s", profiles[current_profile].name), 10,
             SCREEN_HEIGHT - 50, 20, DARKBLUE);
    DrawText(TextFormat("Damping Ratio (Zeta): %.2f",
                        profiles[current_profile].zeta),
             10, SCREEN_HEIGHT - 25, 20, DARKGRAY);
    EndDrawing();
}

float NextProfile(ControlStateSpace *sys)
{
    current_profile = (current_profile + 1) % NUM_PROFILES;
    DampingProfile p = profiles[current_profile];
    float damping_ratio = p.zeta;

    sys->x.coeffs[0] = 100.0f / (NATURAL_FREQUENCY * NATURAL_FREQUENCY);
    sys->x.coeffs[1] = 0.0f;

    sys->u.coeffs[0] = SCREEN_WIDTH / 2.0f;
    sys->y.coeffs[0] = 100.0f;

    // sys->A.data[3] = -(wn * wn);
    sys->A.data[3] = -2.0f * damping_ratio * NATURAL_FREQUENCY;

    // sys->C.data[1] = wn * wn;

    return damping_ratio;
}
