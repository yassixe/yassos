#ifndef __creator_image__
#define __creator_image__
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#define DEFAULT_BLOCK_SIZE 512
#define HEADER_SIZE 92

enum {
    image_size_bytes = (1<<20)*35,
    block_size_bytes = (1 << 9),
    entry_size_bytes = (1 << 7),
    entries_number   = (1 << 7),
    start_esp_bytes  = (1<<20),
    size_esp_bytes   = (1<<20)*32,
    gpt_header_size_bytes = 92

};

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

typedef struct guid{
    uint8_t r_microsoft[16];
}__attribute__((packed)) guid;

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
    guid disk_guid;
    uint64_t partition_entry_lba;
    uint32_t number_of_partition_entries;
    uint32_t size_of_partition_entries;
    uint32_t partition_entry_crc32;
    uint8_t end[DEFAULT_BLOCK_SIZE-HEADER_SIZE];
} __attribute__((packed)) gpt_header;


typedef struct gpt_partition_entry{
    guid type_guid;
    guid unique_guid;
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    uint8_t name[72];
}__attribute__((packed)) gpt_partition_entry;

bool write_mbr(FILE *image);
bool write_gpt_header(FILE* image);
#endif