#ifndef __creator_image__
#define __creator_image__
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#define DEFAULT_BLOCK_SIZE 512
typedef struct partition_record
{
    uint8_t boot_indicator;
    uint8_t starting_chs[3];
    uint8_t os_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t size_in_lba;
} __attribute__((packed)) partition_record;

typedef struct mbr
{
    uint8_t boot_code[440];
    uint32_t signature;
    uint16_t unkown;
    partition_record partition_table[4];
    uint16_t end_signature;
} __attribute__((packed)) mbr;

typedef struct gpt_header
{
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved;
    uint64_t my_lba;
    uint64_t alternate_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint64_t disk_guid[2];
    uint64_t partition_entry_lba;
    uint32_t number_of_partition_entries;
    uint32_t size_of_partition_entries;
    uint32_t partition_entry_crc32;
} __attribute__((packed)) gpt_header;

bool write_mbr(FILE *image);

#endif