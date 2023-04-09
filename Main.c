#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Guid/FileInfo.h>

#include "kernel/frame_buffer_config.h"

typedef struct{
	UINTN			memMapSize;
	EFI_MEMORY_DESCRIPTOR	*buffer;
	UINTN			mapKey;
	UINTN			descSize;
	UINT32			descVer;
} MemoryMap;

EFI_STATUS EFIAPI UefiMain(
		IN EFI_HANDLE ImageHandle,
		IN EFI_SYSTEM_TABLE *SystemTable
		)
{
	Print(L"Hello EDK II.\n");

	EFI_GUID sfsp_guid = {0x0964e5b22, 0x6459, 0x11d2, \
	       			{0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp;
	gBS->LocateProtocol(&sfsp_guid, NULL, (VOID**)&sfsp);

	// ルートディレクトリをオープン
	EFI_FILE_PROTOCOL *root;
	sfsp->OpenVolume(sfsp, &root);

     	// カーネルファイルを開く
	EFI_FILE_PROTOCOL *kernel_file;
	root->Open(root, &kernel_file, L"\\kernel.elf", EFI_FILE_MODE_READ, 0);

	// カーネルファイルのファイル情報を取得
	UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(UINT16) * 12;
	
	UINT8 file_info_buffer[file_info_size];
	kernel_file->GetInfo(
			kernel_file,
			&gEfiFileInfoGuid,
			&file_info_size,
			file_info_buffer
			);
	// ファイルサイズを取得
	EFI_FILE_INFO *kernel_file_info = (EFI_FILE_INFO*)file_info_buffer;
	UINT64 kernel_file_size = kernel_file_info->FileSize;

	// カーネルファイルをメモリに展開
	EFI_PHYSICAL_ADDRESS kernel_base_address = 0x100000;
	gBS->AllocatePages(
			AllocateAddress, EfiLoaderData,
			(kernel_file_size + 0xfff) / 0x1000, &kernel_base_address);
	kernel_file->Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_address);  
			
	// ピクセルを描く 
	EFI_GUID guid_gop = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	gBS->LocateProtocol(&guid_gop, NULL, (VOID**)&gop);
	
	FrameBufferConfig config = {
		(UINT8*)gop->Mode->FrameBufferBase,
		gop->Mode->Info->PixelsPerScanLine,
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution,
		0 };

	switch( gop->Mode->Info->PixelFormat )
	{
		case PixelRedGreenBlueReserved8BitPerColor:
		       config.pixel_format = kPixelRGBResvBitPerColor;
		       break;
		case PixelBlueGreenRedReserved8BitPerColor:
		       config.pixel_format = kPixelBGRResvBitPerColor;
		       break;
		default:
		       Print(L"error\n");
		       while( 1 );
		       break;
	}

	// ブートサービスを停止
	MemoryMap memmap;
	gBS->GetMemoryMap(
			&memmap.memMapSize,
			memmap.buffer,
			&memmap.mapKey,
			&memmap.descSize,
			&memmap.descVer);
	EFI_STATUS status;
	status = gBS->ExitBootServices(ImageHandle, memmap.mapKey);
	if ( status != EFI_SUCCESS ){
		Print(L"failed to exit boot services\n");
		while(1);
	}

	// カーネルのエントリポイントを実行
	UINT64 entry_point_addr = (UINT64)kernel_base_address + 24;
	typedef void EntryPointType(const FrameBufferConfig*);
	EntryPointType *entry_point = (EntryPointType*)entry_point_addr;
	entry_point(&config);

	while (1);
	return 0;
}
