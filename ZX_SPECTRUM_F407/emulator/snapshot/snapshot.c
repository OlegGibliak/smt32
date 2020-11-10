#include <stddef.h>
#include "snapshot.h"
#include "logger.h"
#include "mem.h"
#include "emulator.h"
#include "screen.h"

#define SNAPSHOT_FILE_VERSION_VALID         (0)
#define SNAPSHOT_LENGTH_MEANS_VERSION_2_1   (0x17)
#define SNAPSHOT_LENGTH_MEANS_VERSION_3_0   (0x36)
#define SNAPSHOT_FILE_VERSION_2_1           ("2.1")
#define SNAPSHOT_FILE_VERSION_3_0           ("3.0")
#define SNAPSHOT_LENGTH_TO_VERSION(_HEADER_LEN_)            \
    ((_HEADER_LEN_ == SNAPSHOT_LENGTH_MEANS_VERSION_2_1) ?  \
      SNAPSHOT_FILE_VERSION_2_1 :                           \
      SNAPSHOT_FILE_VERSION_3_0)

#define MEM_PAGE_1                          (0x8000)
#define MEM_PAGE_2                          (0xC000)
#define MEM_PAGE_5                          (0x4000)
#define MEM_PAGE_FULL_SIZE                  (0x4000)

#define IS_DATA_COMPRESSED(_DATA_SIZE_)     (_DATA_SIZE_ != 0xFFFF)

#define COMPRESSION_SEQUENCE_SIZE           (4)
#define COMPRESSION_SEQUENCE                (0xEDED)
#define COMPRESSION_END_MARKER              (0x00EDED00)

typedef enum
{
    PAGE_TYPE_0 = 0, /* 48K rom         rom (basic)     48K rom (Not suported)*/
    PAGE_TYPE_4 = 4, /* 8000-bfff       page 1          Normal 8000-bfff*/
    PAGE_TYPE_5 = 5, /* c000-ffff       page 2          Normal c000-ffff*/
    PAGE_TYPE_8 = 8  /* 4000-7fff       page 5          4000-7fff*/
} page_type_t;

typedef enum
{
    HARDWARE_MODE_0 = 0, /* 48k        */
    HARDWARE_MODE_1      /* 48k + If.1 */
} hardware_mode_t;

typedef struct __attribute__((packed))
{
    uint16_t len;
    uint8_t  page_type;
    uint8_t  data[];
} block_info_t;

static void snapshot_set_state(const snapshot_header_t *header,
                               const snapshot_add_header_t *add_header)
{
    Z80_STATE *cpu = emulator_cpu_state_get();
    uint8_t flags1 = (header->flags1.intr == 255) ? 1 : header->flags1.intr;

	cpu->registers.byte[Z80_A]  = header->a;
	cpu->registers.byte[Z80_F]  = header->f;
	cpu->registers.word[Z80_BC] = header->bc;
	cpu->registers.word[Z80_HL] = header->hl;
	cpu->registers.word[Z80_SP] = header->sp;
	cpu->i  = header->interrupt_register;
	cpu->r  = (header->refresh_register & 0x7F) | ((flags1 & 0x01) << 7);
	cpu->im = header->flags2 & 0x3;
	cpu->registers.word[Z80_DE] = header->de;
	cpu->alternates[Z80_BC] = header->bc_dash;
	cpu->alternates[Z80_DE] = header->de_dash;
	cpu->alternates[Z80_HL] = header->hl_dash;
	cpu->alternates[Z80_AF] = header->f_dash | (header->a_dash << 8);
	cpu->registers.word[Z80_IY] = header->IY;
	cpu->registers.word[Z80_IX] = header->IX;
	cpu->iff1 = header->interrupt_flip_flop;
	cpu->iff2 = header->IFF2;
    if (add_header != NULL)
    {
	    cpu->pc   = add_header->pc;
    }
    else{
        cpu->pc = header->version;
    }
    screen_draw_border((flags1 & 0x0E) >> 1);
}

static void snapshot_decompression(const uint8_t *data, page_type_t page_type, uint16_t page_size, bool compressed)
{
    logger("Page type %d size %d compressed %d\r\n", page_type, page_size, compressed);
    uint16_t mem_addr;
    switch (page_type)
    {
        case PAGE_TYPE_4: mem_addr = MEM_PAGE_1; break;
        case PAGE_TYPE_5: mem_addr = MEM_PAGE_2; break;
        case PAGE_TYPE_8: mem_addr = MEM_PAGE_5; break;
        default: 
            logger("Unknown page type %d\r\n", page_type);
            return;
    }
    

    for (uint16_t i = 0; i < page_size; ++i)
    {
        if ((page_size - i) > COMPRESSION_SEQUENCE_SIZE)
        {
            /* Looking the end-marker. (Only for header version up to 1.4)*/
            uint8_t data32 = *(uint32_t *) &data[i];
            if (compressed && data32 == COMPRESSION_END_MARKER)
			{
                logger("COMPRESSION_END_MARKER found\r\n");
				break;
			}

            uint16_t data16 = *(uint16_t *) &data[i];
            if (compressed && data16 == COMPRESSION_SEQUENCE)
            {
                i += sizeof(data16); /* size of COMPRESSION_SEQUENCE. */

                uint8_t repeats = data[i++];
                uint8_t value   = data[i];
                while (repeats--)
                {
                    mem_writebyte(mem_addr++, value);
                }

                continue;
            }
        }

        mem_writebyte(mem_addr++, data[i]);
    }
}
extern uint8_t load;
bool snapshot_load(const uint8_t *buffer, uint16_t buffer_len)
{
    load = 0;
    uint8_t file_version = 1;
    const snapshot_header_t *header = (const snapshot_header_t *) buffer;
    uint16_t offset = sizeof(*header);

    if (header->version == SNAPSHOT_FILE_VERSION_VALID)
    {
        file_version = 2;
    }

    const snapshot_add_header_t *add_header = (const snapshot_add_header_t *) &buffer[offset];
    if (file_version != 1)
    {
        const snapshot_add_header_t *add_header = (const snapshot_add_header_t *) &buffer[offset];
        if (add_header->hardware_mode > HARDWARE_MODE_1)
        {
            logger("128k not supported\r\n");
            return false;
        }

        logger("Add length = %d\r\n", add_header->header_len);
		if(add_header->header_len == 23)
		{
			file_version = 2;
		}
		else if(add_header->header_len == 54)
		{
			file_version = 3;
		}
		else if(add_header->header_len == 55)
		{
			file_version = 4;
		}
        else
        {
            logger("Invalid add header len\r\n");
            return false;   
        }

        offset += add_header->header_len +2;
    }

    logger("Snapshot header version %d offset %d buffer_len %d\r\n", file_version, offset, buffer_len);

    const block_info_t *mem_block = (const block_info_t *) &buffer[offset];    
    if (file_version == 1)
    {
        add_header = NULL;
        // logger("Snapshot header version %d offset %d buffer_len %d\r\n", file_version, offset, buffer_len);
        // while (offset < buffer_len)
        {
            snapshot_decompression(&buffer[offset],
                                    PAGE_TYPE_8,
                                    buffer_len - offset,
                                    header->flags1.compressed ? true : false);

            // offset += mem_block->len + sizeof(*mem_block);
            // mem_block = (const block_info_t *) &buffer[offset];
        }
    }
    else
    {
        while (offset < buffer_len)
        {
            if (IS_DATA_COMPRESSED(mem_block->len))
            {
                snapshot_decompression(mem_block->data, mem_block->page_type,
                                       mem_block->len, true);
            }
            else
            {
                snapshot_decompression(mem_block->data, mem_block->page_type,
                                       MEM_PAGE_FULL_SIZE, false);
            }
            offset += mem_block->len + sizeof(*mem_block);
            mem_block = (const block_info_t *) &buffer[offset];
        }
        
    }

    snapshot_set_state(header, add_header);

    // const block_info_t *mem_block = (const block_info_t *) &buffer[offset];
    // while (mem_block->page_type == PAGE_TYPE_4 ||
    //        mem_block->page_type == PAGE_TYPE_5 ||
    //        mem_block->page_type == PAGE_TYPE_8)
    // {
    //         snapshot_decompression(mem_block);

    //         offset += mem_block->len + sizeof(*mem_block);
    //         mem_block = (const block_info_t *) &buffer[offset];
    // }

    return true;
}