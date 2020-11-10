#include <stdio.h>
#include "parser.h"
#include "simcity.h"
typedef struct __attribute__((packed))
{
    uint16_t len;
    uint8_t  page_num;
    uint8_t  data[];
} mem_block_t;

void print_header(const snapshot_header_t *header)
{
    printf("\tMain header size: %lu\r\n", sizeof(*header));
    printf("a %02X\r\n", header->a);
    printf("f %02X\r\n", header->f);
    printf("bc %04X\r\n", header->bc);
    printf("hl %04X\r\n", header->hl);
    printf("version %04X\r\n", header->version);
    printf("sp %04X\r\n", header->sp);
    printf("interrupt_register %02X\r\n", header->interrupt_register);
    printf("refresh_register %02X\r\n", header->refresh_register);
    printf("flags.r %02X\r\n", header->flags1.r);
        printf("flags.border_color %02X\r\n", header->flags1.border_color);
        printf("flags.sam_rom %02X\r\n", header->flags1.sam_rom);
        printf("flags.compressed %02X\r\n", header->flags1.compressed);
    printf("de %04X\r\n", header->de);
    printf("bc_dash %04X\r\n", header->bc_dash);
    printf("de_dash %04X\r\n", header->de_dash);
    printf("hl_dash %04X\r\n", header->hl_dash);
    printf("a_dash %02X\r\n", header->a_dash);
    printf("f_dash %02X\r\n", header->f_dash);
    printf("IY %04X\r\n", header->IY);
    printf("IX %04X\r\n", header->IX);
    printf("interrupt_flip_flop %02X\r\n", header->interrupt_flip_flop);
    printf("IFF2 %02X\r\n", header->IFF2);
    printf("flags2 %02X\r\n", header->flags2);
}

void print_add_header(const additional_header_t *header)
{
    printf("\tAdditional header size: %lu\r\n", sizeof(*header));
    printf("add len %04X\r\n", header->additional_header_len);
    printf("pc %04X\r\n", header->pc);
    printf("hardware_mode %02X\r\n", header->hardware_mode);
}

void print_mem_block(const mem_block_t *block)
{
    for (size_t i = 0; i < block->len; i++)
    {
        printf("%02X ", block->data[i]);
    }
    printf("\r\n");
}
void snapshot_load(const uint8_t *buffer)
{
    const snapshot_header_t *header = (const snapshot_header_t *) buffer;
    print_header(header);

    uint16_t offset = sizeof(*header);
    // const additional_header_t *add_header = (additional_header_t *) &buffer[offset];
    // print_add_header(add_header);

    // const mem_block_t *block = (const mem_block_t *)
    //     &buffer[sizeof(header) + add_header->additional_header_len];
    // printf("page_len %04X page_num %02X\r\n", block->len, block->page_num);

    // offset += add_header->additional_header_len +2;
    printf("offset %d\r\n", offset);
    const mem_block_t *block = (const mem_block_t *) &buffer[offset];
    uint16_t i = 4;
    while (i--)
    {
        printf("\tblock id %02X size %04X\r\n", block->page_num, block->len);
        // print_mem_block(block);

        offset += block->len + sizeof(*block);
        block = (const mem_block_t *) &buffer[offset];
    }
}

int main()
{
    snapshot_load(Nether_Earth__1987__Argus_Press_Software__z80);
    return 0;
}