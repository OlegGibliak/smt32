#include "emulator.h"
#include "stm32f1xx_hal.h"
#include "mem.h"
#include "ili9341.h"
#include "input.h"
#include "screen.h"
#include "logger.h"

#define CYCLES_PER_STEP     (69888)

static Z80_STATE zx_cpu;
static CONTEXT zx_context;
static int next_total;
static int total;
static uint32_t ticks;

void emulator_init(void)
{
    ili9341_init();
    mem_init();
    input_key_init();

    zx_context.readbyte  = mem_readbyte;
    zx_context.readword  = mem_readword;
    zx_context.writebyte = mem_writebyte;
    zx_context.writeword = mem_writeword;
    zx_context.input     = mem_input;
    zx_context.output    = mem_output;

    Z80Reset(&zx_cpu);
}

void emulator_loop(void)
{
    total += Z80Emulate(&zx_cpu, next_total - total, &zx_context);
    if (total >= next_total)
    {
        next_total += CYCLES_PER_STEP;
        Z80Interrupt(&zx_cpu, 0xff, &zx_context);
    }
    
    if ((HAL_GetTick() - ticks) >= 100)
    {
        screen_draw();
        ticks = HAL_GetTick();
    }
}

Z80_STATE* emulator_cpu_state_get(void)
{
    return &zx_cpu;
}