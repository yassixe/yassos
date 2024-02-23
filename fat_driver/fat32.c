#include "fat32.h"

char* image = "image.bin";




// typedef struct vbr{
//     uint8_t start_bytes[3];
//     uint8_t eom[8];
//     uint16_t bytes_per_sector;
//     uint8_t sectors_per_cluster;
//     uint16_t reserved_sector;
//     uint8_t number_of_fats;
//     uint16_t root_directory_entries;
//     uint16_t number_of_sectors;
//     uint8_t media_descriptior;
//     uint16_t number_of_sector_per_fat;
//     uint16_t number_of_sector_per_track;
//     uint16_t number_of_heads;
//     uint32_t number_of_headen_blocks;
//     uint32_t unkown;
//     uint8_t drive_number;
//     uint8_t flags;
//     uint8_t signature;
//     uint32_t volume_id;
//     uint8_t padded[11];
//     uint8_t sector[456];
//     uint16_t end;
// }__attribute__((packed)) vbr;



int main()
{
    vbr vbr_;
    printf("%d",sizeof(vbr_));
    FILE* fp = fopen(image,"rb+");
    printf("the info of the disk\n");
    fread(&vbr_,1,sizeof(vbr_),fp);
    printf("the start bytes : %d,%d,%d\n",vbr_.start_bytes[0],vbr_.start_bytes[0],vbr_.start_bytes[0]);
    printf("%s\n",(char*)vbr_.eom);
    printf("number of bytes by sector : %d\n",vbr_.bytes_per_sector);
    return 0;
}