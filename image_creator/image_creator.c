#include "image_creator.h"

char *result_name = "image.bin";

int main(void)
{
    FILE *image = fopen(result_name, "wb+");
    bool True = false;

    if (!image)
    {
        printf("error creating image\n");
        return 1;
    }
    if ((True = write_mbr(image)) == false)
        return 1;

    return 0;
}

bool write_mbr(FILE *image)
{
    partition_record pr_ = {
        .boot_indicator = 0,
        .starting_chs = {0x00, 0x02, 0x00},
        .os_type = 0xee,
        .ending_chs = {0xff, 0xff, 0xff},//to verify
        .starting_lba = 0x1,
        .size_in_lba = 0x1fffff};
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
    printf("the written size is : %ld\n", w_size);
    return true;
}

bool write_gpt_primary_header(FILE* image){
    gpt_header gpth_ = {
        .signature = 0x5452415020494645,
        .revision = 0x00010000,
        .header_size = HEADER_SIZE,
        .header_crc32 = 0x0, //to calculate
        .reserved = 0,
        .my_lba = 1,
        .alternate_lba = 0x1fffff,
        .first_usable_lba =2,
        .last_usable_lba = 0x1fffff -1,
        .disk_guid = {0,0},//find out how to calculate
        .partition_entry_lba = 
    }
}