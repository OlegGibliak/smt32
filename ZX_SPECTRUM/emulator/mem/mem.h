#ifndef MEM_H__
#define MEM_H__

#include <stdint.h>

void mem_init(void);

uint8_t mem_readbyte(uint16_t addr);

uint16_t mem_readword(uint16_t addr);

void mem_writebyte(uint16_t addr, uint8_t b);

void mem_writeword(uint16_t addr, uint16_t w);

uint8_t mem_input(uint8_t portLow, uint8_t portHigh);

void mem_output(uint8_t portLow, uint8_t portHigh, uint8_t x);

#endif /* MEM_H__ */