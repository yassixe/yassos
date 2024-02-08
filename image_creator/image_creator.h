#ifndef __creator_image__
#define __creator_image__
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

typedef struct partition_record{
    uint8_t boot_indicator;
    uint8_t starting_chs[3];
    uint8_t os_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t size_in_lba;
}__attribute__((packed)) partition_record;

typedef struct mbr{
    uint8_t             boot_code[440];
    uint32_t            signature;
    uint16_t            unkown;
    partition_record    partition_table[4];
    uint16_t            end_signature;
}__attribute__((packed)) mbr;



bool write_mbr(FILE* image);

#endif