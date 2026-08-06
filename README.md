# Controlly

**A deterministic, memory safe, and allocation-free control systems library for C and C++**

<img width="852" height="474" alt="controlly_second_order" src="https://github.com/user-attachments/assets/f653b1d9-479b-43c9-9a54-c6def1d9cf09" />

Controlly is a standalone library designed for both hosted and embedded targets.

By utilizing a custom dual-arena memory allocator, Controlly completely eliminates memory fragmentation and guarantees deterministic execution times for your control loops, making it strictly safe for all environments.

## ✨ Key Features

* **Zero Dynamic Memory:** Arena backed memory management. Only provide the buffers and Controlly handles the rest.
* **Dual-Arena Architecture:** Separates persistent state (like StateSpace models and TransferFunctions) from temporary scratch memory (matrix multiplication buffers, temporary polynomial vectors).
* **C & C++ Native:** Compiles seamlessly in strict C99 or modern C++.
* **Two Integration Modes:** Use it as a modular static library via CMake, or drop a single `controlly.h` file directly into your source tree.
* **Rich Error Handling:** Context-aware error reporting with customizable stateful callbacks.

---

## 🚀 Getting Started

### Option 1: The Single-Header Drop-in (Easiest)

Download `dist/controlly.h` and drop into your project

In **exactly one** C or C++ file, define the implementation macro before including the header:

```c
// i.e controlly_impl.c
#define CONTROLLY_IMPLEMENTATION
#include "controlly.h"

```

In all other files, simply include the header normally:

```c
#include "controlly.h"

```

### Option 2: CMake Static Library

---

## 🧠 Memory Architecture: The Dual-Arena
Controlly solves memory management by requiring two distinct arenas:

1. **Persistent Arena (`ctx.persistent`):** Used for data that lives forever. E.g., The computed state-space model from a Transfer Function.

2. **Scratch Arena (`ctx.scratch`):** Used for temporary calculations. E.g., Intermediate mathematical operations. You can safely wipe the entire scratch arena at the end of every control loop cycle, guaranteeing you never run out of memory.

---

## ⚙️ Core Modules
### 1. Proportional-Integral-Derivative (PID) Controller
A lightweight and self-contained PID implementation. It does not require the arena allocator.

```c
#include <controlly/controllers/pid.h>
// or #include <controlly.h>

static ControlPIDController pid = {0};

void SetupPID()
{
    float kp = 250.0f;
    float ki = 10.0f;
    float kd = 100.0f;

    Control_PID_Init(&pid, kp, ki, kd, NULL);
}

void Loop(float dt)
{
    float current_position = get_current_position();
    float target_position = 250.0f;

    float correction = Control_PID_Update(&pid, target_position, current_position, dt);

    apply_correction(correction);
}
```

### 2. Transfer Functions and State-Space Models

```c
#include <controlly/tf.h>
#include <controlly/statespace.h>
// or #include <controlly.h>

ControlStateSpace sys = {0};

void SystemSetup(ControlHandle* ctx, float natural_freq, float damping_ratio)
{
    float wn = natural_freq;
    float zeta = damping_ratio;

    float n_coeffs[] = {wn * wn};
    float d_coeffs[] = {1.0f, 2 * zeta * wn, wn * wn};

    ControlVec num = {0};
    ControlVec den = {0};

    Control_Poly_AllocScratch(ctx, &num, n_coeffs, 1);
    Control_Poly_AllocScratch(ctx, &den, d_coeffs, 3);

    ControlTransferFunction tf = {0};
    Control_TF_FromPoly(ctx, &tf, &num, &den);

    Control_StateSpace_FromTF(ctx, &sys, &tf);

    float initial_x[2] = {0.0f, 0.0f};
    float initial_y[1] = {100.0f};
    float initial_u[1] = {0.0f};

    Control_Poly_AllocPersistent(ctx, &sys.x, initial_x, 2);
    Control_Poly_AllocPersistent(ctx, &sys.y, initial_y, 1);
    Control_Poly_AllocPersistent(ctx, &sys.u, initial_u, 1);

    Control_Arena_Clear(ctx->scratch);
}

float Loop(ControlHandle* ctx, float dt, float target_reference)
{
    sys.u.coeffs[0] = target_reference;

    Control_StateSpace_StepContinuous(ctx, &sys, dt);

    float output = sys.y.coeffs[0];

    return output;
}
```

### 3. Custom Error Handling
```c
#include <stdlib.h>

void Control_ExitOnFailure(ControlResult result,
                           const char   *message,
                           const char   *verbose_data,
                           void         *user_data)
{
    if (result != CONTROL_OK) {
        if (verbose_data != NULL) {
            printf("FATAL ERROR [%s]: %s\n", verbose_data, message);
        }
        else {
            printf("FATAL ERROR: %s\n", message);
        }
        exit(EXIT_FAILURE);
    }
}

// Bind it to your context after initialization
ctx.on_error = Control_ExitOnFailure;
```



---

## 🧪 Testing

Controlly is rigorously tested using the **Unity** testing framework. The test suite guarantees mathematical accuracy and validates strict C++ linkage (ODR, Placement `new`, and C++ compilation).

To run the tests:

```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

---
