# Controlly Examples

## Reading the Source Code
The [Raylib](http://www.raylib.com/) examples contain both rendering and UI code,
which may make it difficult to find the relevant control parts.

To understand the **Controlly** API, look for the following *code beacons*.

### 1. Setup: `ControlSetup()`
This function contains relevant steps for initializing the **Controlly** library.
To see how to start the library, look here.

Typically, the following will appear:
- Memory Arena Allocation : `Control_Arena_Create`
- System context initialization : `Control_System_Init`
- Definitions of Transfer Functions, State Space matrices, and PID gains.

### 2. Loop: `ControlLoop()`
This function contains the relevant information for the control loop.
In **Raylib**, the loop is executed once a frame (meaning speed will be dependent on the system hardware and platform).

To distinguish the different process of the loop, the function is divided into the following blocks
- `[TIME]` : Calculates the physical time between the last iteration
- `[INPUT]` : Updates the input to the control system
- `[SYSTEM CONFIG]` : Logic that dynamically changes system parameters mid-simulation
- `[ENVIRONMENT]` : Updates the environment of the system
- `[VISUALIZATION]` : Signifies a visualization step of the loop. This block is safe to ignore

#### Transfer Functions and PID
- `[CONTROLLER]` : Calls a **Controlly** Controller object to computed correction
- `[PLANT]` : Applies computed correction to a modeled system (typically a physics environment)

#### State Space
- `[STEP]` : Advances the state-space model by `dt`

### 3. Visuals
Functions like `RaylibSetup()` and `DrawVisuals()` are strictly for
rendering the graphics.

> [!TIP]
> If you care only about the **Controlly** and the relevant *control theory*,
> The graphic functions can be safely ignored

## Current Graphical Simulations

- `/pid_leader_follower` Demonstrates classical PID control, with an interactive slider, allowing for interactive tuning
of $k_p$, $k_i$, and $k_d$ parameters.
- `/statespace_second_order` Demonstrates using transfer functions to derive a state space system, for tracking of a dynamic step input

## Building
To build the examples:

```shell
# in the project root:
mkdir build
cd build
cmake .. -DCONTROLLY_BUILD_EXAMPLES
make
```

> [!NOTE]
> The examples are natively configured to be compiled in WebAssembly via Emscripten
