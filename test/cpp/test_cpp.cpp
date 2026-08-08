#include "controlly/tf.h"
#include <cassert>

#ifdef HEADERLIB_TEST
    #include "controlly.h"
#else
    #include <controlly/arena.h>
    #include <controlly/core.h>
    #include <controlly/matrix.h>
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>

class MockPlant
{
  private:
    float m_state;

  public:
    MockPlant(float initial_state) : m_state(initial_state)
    {
        std::cout << "Called constructor" << std::endl;
    }
    ~MockPlant()
    {
        std::cout << "Called destructor" << std::endl;
    }

    void update(float input)
    {
        m_state += input;
    }

    float get_state() const
    {
        return m_state;
    }
};

int main()
{
    ControlHandle                  ctx;
    alignas(std::max_align_t) auto scratch_buf   = new std::uint8_t[256];
    alignas(std::max_align_t) auto permanent_buf = new std::uint8_t[256];

    ControlResult last_err_code = CONTROL_OK;

    auto scratch_arena   = Control_Arena_Create(scratch_buf, 256);
    auto permanent_arena = Control_Arena_Create(permanent_buf, 256);

    Control_System_Init(&ctx, permanent_arena, scratch_arena);
    ctx.user_data = &last_err_code;

    auto err_handle =
        [](ControlResult code, const char *message, const char *verbose_data, void *user_data)
    {
        auto out_code = static_cast<ControlResult *>(user_data);
        if (out_code != nullptr)
        {
            *out_code = code;
        }
        std::cout << "message: " << message << std::endl;
    };
    ctx.on_error = err_handle;

    void *raw_mem = (void *)Control_Arena_Alloc(scratch_arena, 28);
    assert(raw_mem != nullptr && last_err_code == CONTROL_OK && "Arena allocation failed");

    auto space_left = Control_Arena_RemainingSpace(ctx.scratch);
    (void)Control_Arena_Alloc(ctx.scratch, space_left);

    ControlVec out;
    auto       status = Control_Vec_AllocScratch(&ctx, &out, 1);
    assert(status == CONTROL_ERROR_OUT_OF_MEMORY && last_err_code == CONTROL_ERROR_OUT_OF_MEMORY &&
           "Should have not been able to allocate memory");

    void *placement_mem = (void *)Control_Arena_Alloc(ctx.persistent, sizeof(MockPlant));
    assert(placement_mem != nullptr && "Arena allocation failed");

    auto model = new (placement_mem) MockPlant(10.0f);

    model->update(2.5f);
    assert(model->get_state() == 12.5f && "Model state was corrupted");

    float      n[3] = {1.0f, 2.0f, 3.0f};
    ControlVec num_vec;
    status = Control_Poly_AllocPersistent(&ctx, &num_vec, n, 3);

    assert(status == CONTROL_OK && "Allocation should have been ok");
    assert(std::equal(std::begin(n), std::end(n), num_vec.coeffs, num_vec.coeffs + 3) &&
           "Did not set vector properly");

    model->update(5.0f);
    assert(model->get_state() == 17.5f && "Model state was corrupted");

    model->~MockPlant();

    delete[] scratch_buf;
    delete[] permanent_buf;

    std::cout << "C++ arena compatability passed" << std::endl;
    return 0;
}
