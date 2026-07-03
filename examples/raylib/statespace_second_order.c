#include "ccontrol/arena.h"
#include "ccontrol/tf.h"
#include "raylib.h"
#include <ccontrol/statespace.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    float zeta;
    char *name;
} DampingProfile;

int main(void)
{
    // 1. Raylib Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight,
               "ccontrol + raylib : Mass-Spring-Damper");
    SetTargetFPS(60);

    // 2. CControl Initialization
    static const size_t MEM_SIZE = 256;

    void *scratch_mem = malloc(sizeof(uint8_t) * MEM_SIZE);
    void *persistent_mem = malloc(sizeof(uint8_t) * MEM_SIZE);

    ControlArena *s = ControlArena_Create(scratch_mem, MEM_SIZE);
    ControlArena *p = ControlArena_Create(persistent_mem, MEM_SIZE);

    ControlHandle ctx;
    ControlSystem_InitHandle(&ctx, p, s);

    float wn = 10.0f;

    size_t current_profile = 1;
    size_t NUM_PROFILES = 4;
    DampingProfile profiles[4] = {
        {0.0f, "Undamped (Marginal Stability)"},
        {0.5f, "Undamped (Overshoot)"},
        {1.0f, "Critically Damped"},
        {3.0f, "Over Damped (Slow)"},
    };

    float damping_ratio = profiles[current_profile].zeta;

    // 3. Define Transfer Function
    // Use the Second-Order Standard Form
    float n[] = {wn * wn};
    float d[] = {1.0f, 2 * damping_ratio * wn, wn * wn};

    control_vector_t num = PolyCoeffVector_Scratch(&ctx, n, 1);
    control_vector_t dem = PolyCoeffVector_Scratch(&ctx, d, 3);

    // H(s) = 50 / (s^2 + 2s + 50)
    TransferFunction tf = TransferFunctionFromCoeffs(&num, &dem);

    // 4. State Space Representation
    StateSpace sys = TransferFunctionToStateSpace(&ctx, &tf);

    // StateSpace owns its data, it is safe to delete
    // ControlArena_Clear(s);

    float x_data[] = {screenWidth / 2.0f, 0.0f}; // Start in middle of screen
    float u_data[] = {screenWidth / 2.0f};
    float y_data[] = {screenWidth / 2.0f};

    sys.x = (vector_t){.coeffs = x_data, .size = 2};
    sys.u = (vector_t){.coeffs = u_data, .size = 1};
    sys.y = (vector_t){.coeffs = y_data, .size = 1};

    // 5. Game Loop
    while (!WindowShouldClose())
    {
        // --- UPDATE PHASE ---
        if (IsKeyPressed(KEY_LEFT))
        {
            current_profile = (current_profile + 1) % NUM_PROFILES;
            damping_ratio = profiles[current_profile].zeta;
        }
        if (IsKeyPressed(KEY_RIGHT))
        {
            current_profile = (current_profile - 1) % NUM_PROFILES;
            damping_ratio = profiles[current_profile].zeta;
        }

        sys.A.data[3] = -2.0f * damping_ratio * wn;

        // A. Get the actual time elapsed
        float dt = GetFrameTime();

        // Set input data to be the mouse position
        sys.u.coeffs[0] = (float)GetMouseX();

        // C. Iterate over the system
        StateSpace_StepContinous(&ctx, &sys, dt);

        // --- DRAW PHASE ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        int target_x = (int)sys.u.coeffs[0];
        int actual_x = (int)sys.y.coeffs[0];

        DrawLine(target_x, 0, target_x, screenHeight, LIGHTGRAY);
        DrawText("Target (Mouse)", target_x + 5, 10, 20, GRAY);

        DrawRectangle(actual_x - 20, screenHeight / 2 - 20, 40, 40, MAROON);
        DrawText("System Output", actual_x - 50, screenHeight / 2 + 30, 20,
                 MAROON);

        DrawText("Use Left/Right Arrows to change the physical system", 10,
                 screenHeight - 80, 20, GRAY);

        DrawText(TextFormat("Mode: %s", profiles[current_profile].name), 10,
                 screenHeight - 50, 20, DARKBLUE);
        DrawText(TextFormat("Damping Ratio (Zeta): %.2f",
                            profiles[current_profile].zeta),
                 10, screenHeight - 25, 20, DARKGRAY);
        EndDrawing();
    }

    // 6. Cleanup
    CloseWindow();

    ControlSystem_DeInitHandle(&ctx);
    free(scratch_mem);
    free(persistent_mem);

    return 0;
}
