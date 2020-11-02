#ifndef EMULATOR_H__
#define EMULATOR_H__

#include <stdint.h>
#include "z80api.h"

void emulator_init(void);

void emulator_loop(void);

Z80_STATE*emulator_cpu_state_get(void);

#endif /* EMULATOR_H__ */
