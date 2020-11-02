#ifndef SNAPSHOT_H__
#define SNAPSHOT_H__

#include <stdint.h>
#include <stdbool.h>
// #include "z80emu.h"

typedef union __attribute__((packed))
{
	struct __attribute__((packed)) 
	{
		uint8_t r 			 :1;  	// Bit 0  : Bit 7 of the R-register
		uint8_t border_color :3;	// Bit 1-3: Border colour
		uint8_t sam_rom      :1;    // Bit 4  : 1=Basic SamRom switched in
		uint8_t compressed   :1;    // Bit 5  : 1=Block of data is compressed(only for version up to 1.4)
		uint8_t rfu			 :2;    // Bit 6-7: No meaning
	};
	uint8_t intr;
} flags1_t;

typedef struct __attribute__((packed))
{
	uint8_t  a;
	uint8_t  f;
	uint16_t bc;
	uint16_t hl;
    uint16_t version;
    uint16_t sp;
    uint8_t  interrupt_register;
    uint8_t  refresh_register;
    flags1_t flags1;
	uint16_t de;
	uint16_t bc_dash;
	uint16_t de_dash;
	uint16_t hl_dash;
	uint8_t  a_dash;
	uint8_t  f_dash;
	uint16_t IY;
	uint16_t IX;
	uint8_t  interrupt_flip_flop;
	uint8_t  IFF2;
    uint8_t  flags2;
} snapshot_header_t;

typedef struct __attribute__((packed))
{
	uint16_t header_len;            // Length of additional header block
    uint16_t pc;
    uint8_t  hardware_mode;
    uint8_t  pagingState;			// If in SamRam mode, bitwise state of 74ls259.
                        			// For example, bit 6=1 after an OUT 31,13 (=2*6+1)
                        			// If in 128 mode, contains last OUT to 7ffd
} snapshot_add_header_t;

bool snapshot_load(const uint8_t *buffer, uint16_t buffer_len);

#endif /* SNAPSHOT_H__ */