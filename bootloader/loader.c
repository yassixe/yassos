#include "common.h"


EFI_STATUS get_memory(){
  EFI_STATUS status;
  //to get the correct size of the buffer we are going to get memory twice.

  UINTN buffer_size = 12;
  EFI_MEMORY_DESCRIPTOR buffer;
  UINTN desciptor_size, map_key, descriptor_version;
  if(uefi_call_wrapper(BS->GetMemoryMap,5,&buffer_size,&buffer,&map_key,&desciptor_size,descriptor_version) != EFI_BUFFER_TOO_SMALL){
    Print(L"something went wrong, it should return efi_small_buffer");
  }
  Print(L"size_buffer:%d\ndescriptor_size:%d\n",buffer_size,desciptor_size);
  UINT8 memory_buffer[buffer_size];
  uefi_call_wrapper(BS->GetMemoryMap,5,&buffer_size,memory_buffer,&map_key,&desciptor_size,descriptor_version);
  Print(L"size_buffer:%d\ndescriptor_size:%d\n",buffer_size,desciptor_size);
  return EFI_SUCCESS;
}


// typedef
// EFI_STATUS
// (EFIAPI \*EFI_GET_MEMORY_MAP) (
// IN OUT UINTN *MemoryMapSize,
// OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
// OUT UINTN *MapKey,
// OUT UINTN *DescriptorSize,
// OUT UINT32 *DescriptorVersion
// );

// typedef struct {
// UINT32 Type;
// EFI_PHYSICAL_ADDRESS PhysicalStart;
// EFI_VIRTUAL_ADDRESS VirtualStart;
// UINT64 NumberOfPages;
// UINT64 Attribute;
// } EFI_MEMORY_DESCRIPTOR;


EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  //uefi im alerady in the protected mode no need to write 16-bit assembly for real mode.
  EFI_STATUS status;
  
  //required to work with gnu-efi api
  InitializeLib(ImageHandle, SystemTable);

  //reset the text output device hardware
  status = uefi_call_wrapper(ST->ConOut->Reset,1,ST->ConOut);


  //Get memory map
  get_memory();
  while(1);
  return EFI_SUCCESS;
}