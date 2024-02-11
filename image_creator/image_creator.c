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

    write_gpt_header(image);
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
        .size_in_lba = (image_size_bytes/block_size_bytes)-1};
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
    if(w_size != block_size_bytes) return 0;
    return true;
}

guid new_guid(){
    guid a;
    a.i[0]=1;
    a.i[1]=1;
    return a;
}






bool write_gpt_header(FILE* image){
    gpt_header gpth = {
        .signature =  0x5452415020494645,
        .revision = 0x00010000,
        .header_size = gpt_header_size_bytes,
        .header_crc32 = 0,
        .reserved = 0,
        .my_lba = 1,
        .alternate_lba = (image_size_bytes/block_size_bytes)-1,
        .first_usable_lba = 1 + (entries_number*entry_size_bytes)/(block_size_bytes) + 1,
        .last_usable_lba = (image_size_bytes/block_size_bytes)-1 -(entries_number*entry_size_bytes)/(block_size_bytes) -1,
        .disk_guid = new_guid(),
        .partition_entry_lba = 2,
        .number_of_partition_entries = entries_number,
        .size_of_partition_entries = entry_size_bytes,
        .partition_entry_crc32 = 0,
        .end = {0},
    };
    
    
    return true;
}
