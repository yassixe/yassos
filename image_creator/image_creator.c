#include "image_creator.h"

char *result_name = "image.img";

/* Table of CRCs of all 8-bit messages. */
uint32_t crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
    uint32_t c;
    int n, k;

    for (n = 0; n < 256; n++)
    {
        c = (uint32_t)n;
        for (k = 0; k < 8; k++)
        {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}
/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below). */

uint32_t update_crc(uint32_t crc, unsigned char *buf, int len)
{
    uint32_t c = crc;
    int n;

    if (!crc_table_computed)
        make_crc_table();
    for (n = 0; n < len; n++)
    {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
uint32_t crc(unsigned char *buf, int len)
{
    return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

void test()
{
    for (int i = 0; i < 10; i++)
    {
        printf("%d\n", rand() % 256);
    }
}

int main(void)
{
    FILE *image = fopen(result_name, "wb+");
    bool True = false;
    srand(time(NULL));
    if (!image)
    {
        printf("error creating image\n");
        return 1;
    }
    if ((True = write_mbr(image)) == false)
    {
        printf("error mbr writing\n");
        return 1;
    }

    if ((True = write_gpt_header(image)) == false)
    {
        printf("error gpt writing\n");
        return 1;
    }
    return 0;
}

bool write_vbr(FILE *image){
    vbr vbr_ = {
        .start_bytes = {0xeb,0x3c,0x90},
        .eom =  {"YASS  0S"},
        .bytes_per_sector = block_size_bytes,
        .sectors_per_cluster = 1,
        .reserved_sector = 0,//to check
        .number_of_fats = 2,
        .root_directory_entries= 0, //need to be corrected
        .number_of_sectors =  size_esp_bytes/block_size_bytes,
        .media_descriptior = 0xF0,//changeable
        .number_of_sector_per_fat = 0, //check
        .number_of_sector_per_track = 0, //check
        .number_of_heads = 0,
        .number_of_headen_blocks =0,//check
        .unkown = 0,
        .drive_number = 0,
        .flags = 0,
        .signature = 0,
        .volume_id = 0,
        

    }
}

bool write_mbr(FILE *image)
{
    partition_record pr_ = {
        .boot_indicator = 0,
        .starting_chs = {0x00, 0x02, 0x00},
        .os_type = 0xee,
        .ending_chs = {0xff, 0xff, 0xff}, // to verify
        .starting_lba = 0x1,
        .size_in_lba = (image_size_bytes / block_size_bytes) - 1};
    mbr mbr_ = {
        .boot_code = {0},
        .signature = 0,
        .unkown = 0,
        .partition_table = {
            pr_,
            (partition_record){0},
            (partition_record){0},
            (partition_record){0},
        },
        .end_signature = 0xAA55};
    size_t w_size = fwrite(&mbr_, 1, sizeof(mbr_), image);
    if (w_size != block_size_bytes)
        return 0;
    return true;
}

guid new_guid()
{
    guid result;
    uint8_t *r_microsoft = (uint8_t *)&result;
    for (uint8_t i = 0; i < 16; i++)
    {
        r_microsoft[i] = rand() % 256;
    }
    // set the variant and the version variant:
    //  1     1     0    Reserved, Microsoft Corporation backward
    //                   compatibility
    r_microsoft[8] |= (1 << 7);
    r_microsoft[8] |= (1 << 6);
    r_microsoft[8] &= ~(1 << 5);
    r_microsoft[7] &= 0b01001111;
    r_microsoft[7] |= (1 << 6);

    return result;
}

bool write_gpt_header(FILE *image)
{
    guid efi_guidtype = {
        .time_low = 0xC12A7328,
        .time_mid = 0xF81F,
        .time_high_version = 0x11D2,
        .clock_seq_reserved = 0xBA,
        .clock_seq_low = 0x4B,
        .node = {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};

    gpt_header gpth = {
        .signature = 0x5452415020494645,
        .revision = 0x00010000,
        .header_size = gpt_header_size_bytes,
        .header_crc32 = 0,
        .reserved = 0,
        .my_lba = 1,
        .alternate_lba = (image_size_bytes / block_size_bytes) - 1,
        .first_usable_lba = 1 + (entries_number * entry_size_bytes) / (block_size_bytes) + 1,
        .last_usable_lba = (image_size_bytes / block_size_bytes) - 1 - (entries_number * entry_size_bytes) / (block_size_bytes)-1,
        .disk_guid = new_guid(),
        .partition_entry_lba = 2,
        .number_of_partition_entries = entries_number,
        .size_of_partition_entries = entry_size_bytes,
        .partition_entry_crc32 = 0,
        .end = {0},
    };
    gpt_header gpth_alt = {
        .signature = 0x5452415020494645,
        .revision = 0x00010000,
        .header_size = gpt_header_size_bytes,
        .header_crc32 = 0,
        .reserved = 0,
        .my_lba = (image_size_bytes / block_size_bytes) - 1,
        .alternate_lba = 1,
        .first_usable_lba = 1 + (entries_number * entry_size_bytes) / (block_size_bytes) + 1,
        .last_usable_lba = (image_size_bytes / block_size_bytes) - 1 - (entries_number * entry_size_bytes) / (block_size_bytes)-1,
        .disk_guid = new_guid(),
        .partition_entry_lba = 2,
        .number_of_partition_entries = entries_number,
        .size_of_partition_entries = entry_size_bytes,
        .partition_entry_crc32 = 0,
        .end = {0},
    };
    gpt_partition_entry entries_table[128] = {
        {.type_guid = efi_guidtype,
         .unique_guid = new_guid(),
         .starting_lba = 2048,
         .ending_lba = 2048 + (size_esp_bytes / block_size_bytes) - 1,
         .attributes = 0,
         .name = "efi partition"},
        {.type_guid = efi_guidtype,
         .unique_guid = new_guid(),
         .starting_lba = 2048 + (size_esp_bytes / block_size_bytes),
         .ending_lba = 4096 + (size_esp_bytes / block_size_bytes) - 1,
         .attributes = 0,
         .name = "data partition"},
    };
    gpth.partition_entry_crc32 = crc((unsigned char *)entries_table, entries_number * entry_size_bytes);
    gpth_alt.partition_entry_crc32 = gpth.partition_entry_crc32;
    gpth.header_crc32 = crc((unsigned char *)&gpth, gpt_header_size_bytes);
    gpth_alt.header_crc32 = crc((unsigned char *)&gpth_alt, gpt_header_size_bytes);

    size_t w_size = fwrite(&gpth, 1, sizeof(gpth), image);
    if (w_size != block_size_bytes)
        return false;
    w_size = fwrite(entries_table, 1, sizeof(entries_table), image);
    if (w_size != 128 * 128)
        return false;

    uint64_t last_usable_lba = gpth.last_usable_lba;
    uint64_t seek_position = (last_usable_lba + 1) * block_size_bytes;
    fseek(image, (long)seek_position, 0);
    w_size = fwrite(entries_table, 1, sizeof(entries_table), image);
    w_size = fwrite(&gpth_alt, 1, sizeof(gpth_alt), image);
    ;

    return true;
}
