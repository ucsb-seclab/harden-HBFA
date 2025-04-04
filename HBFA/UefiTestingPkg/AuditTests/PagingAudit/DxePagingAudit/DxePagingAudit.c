/** @file -- DxePagingAuditApp.c
This DXE Driver writes page table and memory map information to SFS when triggered
by an event.

Copyright (c) 2017, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

// MS_CHANGE - Entire file created.

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Register/Msr.h>
#include <Library/PrintLib.h>
// #include <Library/ShellLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Guid/EventGroup.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/HeapGuardDebug.h>
#include <Register/Msr.h>
#include <Register/Cpuid.h>

#include <Library/DevicePathLib.h>
#include <Guid/DebugImageInfoTable.h>
#include <Guid/MemoryAttributesTable.h>
#include "DxePagingAuditCommon.h"

HEAP_GUARD_DEBUG_PROTOCOL *mHgDumpBitMap;
EFI_FILE  *mFs_Handle;
CHAR8     *mMemoryInfoDatabaseBuffer = NULL;
UINTN      mMemoryInfoDatabaseSize = 0;
UINTN      mMemoryInfoDatabaseAllocSize = 0;

/**
  This helper function writes a string entry to the memory info database buffer.
  If string would exceed current buffer allocation, it will realloc.

  NOTE: The buffer tracks its size. It does not work with NULL terminators.

  @param[in]  DatabaseString    A pointer to a CHAR8 string that should be
                                added to the database.

  @retval     EFI_SUCCESS           String was successfully added.
  @retval     EFI_OUT_OF_RESOURCES  Buffer could not be grown to accomodate string.
                                    String has not been added.

**/
STATIC
EFI_STATUS
AppendToMemoryInfoDatabase (
  IN CONST CHAR8    *DatabaseString
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
  UINTN         NewStringSize, NewDatabaseSize;
  CHAR8         *NewDatabaseBuffer;

  // If the incoming string is NULL or empty, get out of here.
  if (DatabaseString == NULL || DatabaseString[0] == '\0') {
    return EFI_SUCCESS;
  }

  // Determine the length of the incoming string.
  // NOTE: This size includes the NULL terminator.
  NewStringSize = AsciiStrnSizeS( DatabaseString, MEM_INFO_DATABASE_MAX_STRING_SIZE );
  NewStringSize = NewStringSize - sizeof(CHAR8);    // Remove NULL.

  // If we need more space, realloc now.
  // Subtract 1 because we only need a single NULL terminator.
  NewDatabaseSize = NewStringSize + mMemoryInfoDatabaseSize;
  if (NewDatabaseSize > mMemoryInfoDatabaseAllocSize) {
    NewDatabaseBuffer = ReallocatePool( mMemoryInfoDatabaseAllocSize,
                                        mMemoryInfoDatabaseAllocSize + MEM_INFO_DATABASE_REALLOC_CHUNK,
                                        mMemoryInfoDatabaseBuffer );
    // If we failed, don't change anything.
    if (NewDatabaseBuffer == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    }
    // Otherwise, updated the pointers and sizes.
    else {
      mMemoryInfoDatabaseBuffer = NewDatabaseBuffer;
      mMemoryInfoDatabaseAllocSize += MEM_INFO_DATABASE_REALLOC_CHUNK;
    }
  }

  // If we're still good, copy the new string to the end of
  // the buffer and update the size.
  if (!EFI_ERROR( Status )) {
    // Subtract 1 to remove the previous NULL terminator.
    CopyMem( &mMemoryInfoDatabaseBuffer[mMemoryInfoDatabaseSize], DatabaseString, NewStringSize );
    mMemoryInfoDatabaseSize = NewDatabaseSize;
  }

  return Status;
} // AppendToMemoryInfoDatabase()


/**
  Creates a new file and writes the contents of the caller's data buffer to the file.

  @param    Fs_Handle           Handle to an opened filesystem volume/partition.
  @param    FileName            Name of the file to create.
  @param    DataBufferSize      Size of data to buffer to be written in bytes.
  @param    Data                Data to be written.

  @retval   EFI_STATUS          File was created and data successfully written.
  @retval   Others              The operation failed.

**/
EFI_STATUS
DflDxeCreateAndWriteFileSFS (
    IN EFI_FILE*  Fs_Handle,
    IN CHAR16*    FileName,
    IN UINTN      DataBufferSize,
    IN VOID*      Data
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_FILE    *FileHandle = NULL;

    DEBUG((DEBUG_ERROR, "%a: Creating file: %s \n", __FUNCTION__, FileName));

    // Create the file with RW permissions.
    //
    Status = Fs_Handle->Open (
                            Fs_Handle,
                            &FileHandle,
                            FileName,
                            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                            0
                            );

    if (EFI_ERROR (Status)) {
        DEBUG((DEBUG_ERROR, "%a: Failed to create file %s: %r !\n", __FUNCTION__, FileName, Status));
        goto CleanUp;
    }

    // Write the contents of the caller's data buffer to the file.
    //
    Status = FileHandle->Write (
                                FileHandle,
                                &DataBufferSize,
                                Data
                                );

    if (EFI_ERROR (Status)) {
        DEBUG((DEBUG_ERROR, "%a: Failed to write to file %s: %r !\n", __FUNCTION__, FileName, Status));
        goto CleanUp;
    }

    FileHandle->Flush(Fs_Handle);

CleanUp:

    // Close the file if it was successfully opened.
    //
    if (FileHandle != NULL) {
        FileHandle->Close (FileHandle);
    }

    return Status;
}

/**
 * @brief      Writes a buffer to file.
 *
 * @param      FileName     The name of the file being written to.
 * @param      Buffer       The buffer to write to file.
 * @param[in]  BufferSize   Size of the buffer.
 * @param[in]  WriteCount   Number to append to the end of the file.
 */
STATIC
VOID
WriteBufferToFile (
  IN CONST CHAR16                   *FileName,
  IN       VOID                     *Buffer,
  IN       UINTN                     BufferSize
  )
{
  EFI_STATUS          Status;
  CHAR16              FileNameAndExt[MAX_STRING_SIZE];

  // Calculate final file name.
  ZeroMem( FileNameAndExt, sizeof(CHAR16) * MAX_STRING_SIZE );
  UnicodeSPrint( FileNameAndExt, MAX_STRING_SIZE, L"%s.dat", FileName );

  Status = DflDxeCreateAndWriteFileSFS(mFs_Handle, FileNameAndExt, BufferSize, Buffer);
  DEBUG((DEBUG_ERROR, "%a Writing file %s - %r\n", __FUNCTION__, FileNameAndExt, Status));
}

/**
 * @brief      Writes the MemoryAttributesTable to a file.
 */
VOID
EFIAPI
MemoryAttributesTableDump (
  VOID
  )
{
  EFI_STATUS                      Status;
  EFI_MEMORY_ATTRIBUTES_TABLE     *MatMap;
  EFI_MEMORY_DESCRIPTOR           *Map;
  UINT64                          EntrySize;
  UINT64                          EntryCount;
  CHAR8                           *WriteString;
  CHAR8                           *Buffer;
  UINT64                          Index;
  UINTN                           BufferSize;
  UINTN                           FormattedStringSize;
  // NOTE: Important to use fixed-size formatters for pointer movement.
  CHAR8                           MatFormatString[] = "MAT,0x%016lx,0x%016lx,0x%016lx,0x%016lx,0x%016lx\n";
  CHAR8                           TempString[MAX_STRING_SIZE];

  //
  // First, we need to locate the MAT table.
  //
  Status = EfiGetSystemConfigurationTable( &gEfiMemoryAttributesTableGuid, &MatMap );

  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a Failed to retrieve MAT %r", __FUNCTION__, Status));
    return;
  }

  // MAT should now be at the pointer.
  EntrySize    = MatMap->DescriptorSize;
  EntryCount   = MatMap->NumberOfEntries;
  Map          = (VOID*)((UINT8*)MatMap + sizeof( *MatMap ));


  //
  // Next, we need to allocate a buffer to hold all of the entries.
  // We'll be storing the data as fixed-length strings.
  //
  // Do a dummy format to determine the size of a string.
  // We're safe to use 0's, since the formatters are fixed-size.
  FormattedStringSize = AsciiSPrint( TempString, MAX_STRING_SIZE, MatFormatString, 0, 0, 0, 0, 0 );
  // Make sure to add space for the NULL terminator at the end.
  BufferSize = (UINTN)(EntryCount * FormattedStringSize) + sizeof(CHAR8);
  Buffer = AllocatePool(BufferSize);
  if (!Buffer) {
    DEBUG((DEBUG_ERROR, "%a Failed to allocate buffer for data dump!", __FUNCTION__));
    return;
  }


  //
  // Add all entries to the buffer.
  //
  WriteString = Buffer;
  for (Index = 0; Index < EntryCount; Index ++)
  {
    AsciiSPrint( WriteString,
                 FormattedStringSize+1,
                 MatFormatString,
                 Map->Type,
                 Map->PhysicalStart,
                 Map->VirtualStart,
                 Map->NumberOfPages,
                 Map->Attribute );

    WriteString += FormattedStringSize;
    Map = NEXT_MEMORY_DESCRIPTOR( Map, EntrySize );
  }


  //
  // Finally, write the strings to the dump file.
  //
  // NOTE: Don't need to save the NULL terminator.
  WriteBufferToFile( L"MAT", Buffer, BufferSize-1 );

  FreePool(Buffer);
}


/**
  Initializes the valid bits mask and valid address mask for MTRRs.

  This function initializes the valid bits mask and valid address mask for MTRRs.

  Copied from UefiCpuPkg MtrrLib

  @param[out]  MtrrValidBitsMask     The mask for the valid bit of the MTRR
  @param[out]  MtrrValidAddressMask  The valid address mask for the MTRR

**/
STATIC
VOID
InitializeMtrrMask (
  OUT UINT64 *MtrrValidBitsMask,
  OUT UINT64 *MtrrValidAddressMask
  )
{
  UINT32                          MaxExtendedFunction;
  CPUID_VIR_PHY_ADDRESS_SIZE_EAX  VirPhyAddressSize;


  AsmCpuid( CPUID_EXTENDED_FUNCTION, &MaxExtendedFunction, NULL, NULL, NULL );

  if (MaxExtendedFunction >= CPUID_VIR_PHY_ADDRESS_SIZE) {
    AsmCpuid( CPUID_VIR_PHY_ADDRESS_SIZE, &VirPhyAddressSize.Uint32, NULL, NULL, NULL );
  } else {
    VirPhyAddressSize.Bits.PhysicalAddressBits = 36;
  }

  *MtrrValidBitsMask = LShiftU64( 1, VirPhyAddressSize.Bits.PhysicalAddressBits ) - 1;
  *MtrrValidAddressMask = *MtrrValidBitsMask & 0xfffffffffffff000ULL;
}

STATIC
EFI_STATUS
TSEGDumpHandler (
  VOID
  )
{
  UINT64      SmrrBase;
  UINT64      SmrrMask;
  UINT64      Length; 
  UINT64      MtrrValidBitsMask; 
  UINT64      MtrrValidAddressMask;  
  CHAR8       TempString[MAX_STRING_SIZE];

  DEBUG(( DEBUG_INFO, "%a()\n", __FUNCTION__ ));

  MtrrValidBitsMask = 0;
  MtrrValidAddressMask = 0;

  InitializeMtrrMask( &MtrrValidBitsMask, &MtrrValidAddressMask );

  DEBUG(( DEBUG_VERBOSE, "%aMTRR valid bits 0x%016lx, address mask: 0x%016lx\n", __FUNCTION__, MtrrValidBitsMask , MtrrValidAddressMask ));

  // This is a 64-bit read, but the SMRR registers bits 63:32 are reserved.
  SmrrBase = AsmReadMsr64( MSR_IA32_SMRR_PHYSBASE );
  SmrrMask = AsmReadMsr64( MSR_IA32_SMRR_PHYSMASK );
  // Extend the mask to account for the reserved bits.
  SmrrMask |= 0xffffffff00000000ULL;

  DEBUG(( DEBUG_VERBOSE, "%aSMRR base 0x%016lx, mask: 0x%016lx\n", __FUNCTION__, SmrrBase , SmrrMask ));

  // Extend the top bits of the mask to account for the reserved

  Length = ((~(SmrrMask & MtrrValidAddressMask)) & MtrrValidBitsMask) + 1;

  DEBUG(( DEBUG_VERBOSE, "%aCalculated length: 0x%016lx\n", __FUNCTION__, Length ));

  // Writing this out in the format of a Memory Map entry (Type 16 will map to TSEG)
  AsciiSPrint( TempString, MAX_STRING_SIZE,
               "TSEG,0x%016lx,0x%016lx,0x%016lx,0x%016lx,0x%016lx\n",
               16,
               (SmrrBase & MtrrValidAddressMask),
               0,
               EFI_SIZE_TO_PAGES( Length ),
               0 );
  AppendToMemoryInfoDatabase( TempString );

  return EFI_SUCCESS;
}

/**
 * @brief      Writes the UEFI memory map to file.
 */
STATIC
VOID
MemoryMapDumpHandler (
  VOID
  )
{

  EFI_STATUS                  Status;
  UINTN                       EfiMemoryMapSize;
  UINTN                       EfiMapKey;
  UINTN                       EfiDescriptorSize;
  UINT32                      EfiDescriptorVersion;
  EFI_MEMORY_DESCRIPTOR       *EfiMemoryMap;
  EFI_MEMORY_DESCRIPTOR       *EfiMemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR       *EfiMemNext;
  CHAR8                       TempString[MAX_STRING_SIZE];

  DEBUG(( DEBUG_INFO, "%a()\n", __FUNCTION__ ));

  //
  // Get the EFI memory map.
  //
  EfiMemoryMapSize  = 0;
  EfiMemoryMap      = NULL;
  Status = gBS->GetMemoryMap( &EfiMemoryMapSize,
                              EfiMemoryMap,
                              &EfiMapKey,
                              &EfiDescriptorSize,
                              &EfiDescriptorVersion );
  //
  // Loop to allocate space for the memory map and then copy it in.
  //
  do {
    EfiMemoryMap = (EFI_MEMORY_DESCRIPTOR*)AllocateZeroPool( EfiMemoryMapSize );
    ASSERT( EfiMemoryMap != NULL );
    Status = gBS->GetMemoryMap( &EfiMemoryMapSize,
                                EfiMemoryMap,
                                &EfiMapKey,
                                &EfiDescriptorSize,
                                &EfiDescriptorVersion );
    if (EFI_ERROR( Status )) {
      FreePool( EfiMemoryMap );
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  EfiMemoryMapEnd = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)EfiMemoryMap + EfiMemoryMapSize);
  EfiMemNext = EfiMemoryMap;

  while (EfiMemNext < EfiMemoryMapEnd) {
    AsciiSPrint( TempString, MAX_STRING_SIZE,
                 "MemoryMap,0x%016lx,0x%016lx,0x%016lx,0x%016lx,0x%016lx\n",
                 EfiMemNext->Type,
                 EfiMemNext->PhysicalStart,
                 EfiMemNext->VirtualStart,
                 EfiMemNext->NumberOfPages,
                 EfiMemNext->Attribute );
    AppendToMemoryInfoDatabase( TempString );
    EfiMemNext = NEXT_MEMORY_DESCRIPTOR( EfiMemNext, EfiDescriptorSize );
  }

  if (EfiMemoryMap) {
    FreePool( EfiMemoryMap );
  }
}

/**
 * @brief      Writes the name, base, and limit of each image in the image table to a file.
 */
VOID
EFIAPI
LoadedImageTableDump (
  VOID
  )
{
  EFI_STATUS                                   Status;
  EFI_DEBUG_IMAGE_INFO_TABLE_HEADER           *TableHeader;
  EFI_DEBUG_IMAGE_INFO                        *Table;
  EFI_LOADED_IMAGE_PROTOCOL                   *LoadedImageProtocolInstance;
  UINT64                                       ImageBase;
  UINT64                                       ImageSize;
  UINT64                                       Index;
  UINT32                                       TableSize;
  EFI_DEBUG_IMAGE_INFO_NORMAL                 *NormalImage;
  CHAR8                                       *PdbFileName;
  CHAR8                                       TempString[MAX_STRING_SIZE];

  DEBUG(( DEBUG_INFO, "%a()\n", __FUNCTION__ ));

  //
  // locate DebugImageInfoTable
  //
  Status = EfiGetSystemConfigurationTable( &gEfiDebugImageInfoTableGuid, (VOID**)&TableHeader );
  if (EFI_ERROR( Status )) {
    DEBUG(( DEBUG_ERROR, "Failed to retrieve loaded image table %r", Status ));
    return;
  }

  Table = TableHeader->EfiDebugImageInfoTable;
  TableSize = TableHeader->TableSize;

  DEBUG(( DEBUG_VERBOSE, "%a\n\nLength %lx Start x0x%016lx\n\n", __FUNCTION__, TableHeader->TableSize, Table ));

  for (Index = 0; Index < TableSize; Index ++) {
    if (Table[Index].NormalImage == NULL) {
      continue;
    }

    NormalImage = Table[Index].NormalImage;
    LoadedImageProtocolInstance = NormalImage->LoadedImageProtocolInstance;
    ImageSize = LoadedImageProtocolInstance->ImageSize;
    ImageBase = (UINT64) LoadedImageProtocolInstance->ImageBase;

    if (ImageSize == 0) {
      // No need to register empty slots in the table as images.
      continue;
    }
    PdbFileName = PeCoffLoaderGetPdbPointer( LoadedImageProtocolInstance->ImageBase );
    AsciiSPrint( TempString, MAX_STRING_SIZE,
                 "LoadedImage,0x%016lx,0x%016lx,%a\n",
                 ImageBase,
                 ImageSize,
                 PdbFileName );
    AppendToMemoryInfoDatabase( TempString );
  }
}
/**

  Opens the SFS volume and if successful, returns a FS handle to the opened volume.

  @param    mFs_Handle       Handle to the opened volume.

  @retval   EFI_SUCCESS     The FS volume was opened successfully.
  @retval   Others          The operation failed.

**/
EFI_STATUS
DflDxeOpenVolumeSFS (
  OUT EFI_FILE** Fs_Handle
  )
{

  EFI_DEVICE_PATH_PROTOCOL* DevicePath;
  BOOLEAN Found;
  EFI_HANDLE Handle;
  EFI_HANDLE* HandleBuffer;
  UINTN Index;
  UINTN NumHandles;
  EFI_DEVICE_PATH_PROTOCOL* OrigDevicePath;
  EFI_STRING PathNameStr;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* SfProtocol;
  EFI_STATUS Status;

  Status = EFI_SUCCESS;
  SfProtocol = NULL;
  NumHandles = 0;
  HandleBuffer = NULL;

  //
  // Locate all handles that are using the SFS protocol.
  //
  Status = gBS->LocateHandleBuffer(ByProtocol,
                                   &gEfiSimpleFileSystemProtocolGuid,
                                   NULL,
                                   &NumHandles,
                                   &HandleBuffer);

  if (EFI_ERROR(Status) != FALSE) {
    DEBUG((DEBUG_ERROR, "%a: failed to locate all handles using the Simple FS protocol (%r)\n", __FUNCTION__, Status));
    goto CleanUp;
  }

  //
  // Search the handles to find one that is on a GPT partition on a hard drive.
  //
  Found = FALSE;
  for (Index = 0; (Index < NumHandles) && (Found == FALSE); Index += 1) {
    DevicePath = DevicePathFromHandle(HandleBuffer[Index]);
    if (DevicePath == NULL) {
      continue;
    }

    //
    // Save the original device path because we change it as we're checking it
    // below. We'll need the unmodified version if we determine that it's good.
    //
    OrigDevicePath = DevicePath;

    //
    // Convert the device path to a string to print it.
    //
    PathNameStr = ConvertDevicePathToText(DevicePath,TRUE,TRUE);
    DEBUG((DEBUG_ERROR, "%a: device path %d -> %s\n", __FUNCTION__, Index, PathNameStr));

    //
    // Check if this is a block IO device path. If it is not, keep searching.
    // This changes our locat device path variable, so we'll have to restore
    // it afterwards.
    //
    Status = gBS->LocateDevicePath(&gEfiBlockIoProtocolGuid, 
                                   &DevicePath, 
                                   &Handle);

    if (EFI_ERROR(Status) != FALSE) {
      DEBUG((DEBUG_ERROR, "%a: not a block IO device path\n", __FUNCTION__));
      continue;
    }

    //
    // Restore the device path and check if this is a GPT partition. We only
    // want to write our log on GPT partitions.
    //
    DevicePath = OrigDevicePath;
    while (IsDevicePathEnd(DevicePath) == FALSE) {
      //
      // If the device path is not a hard drive, we don't want it.
      //
      if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH &&
          DevicePathSubType(DevicePath) == MEDIA_HARDDRIVE_DP) {

        //
        // Check if this is a gpt partition. If it is, we'll use it. Otherwise,
        // keep searching.
        //
        if (((HARDDRIVE_DEVICE_PATH*)DevicePath)->MBRType == MBR_TYPE_EFI_PARTITION_TABLE_HEADER && 
            ((HARDDRIVE_DEVICE_PATH*)DevicePath)->SignatureType == SIGNATURE_TYPE_GUID) {

          DevicePath = OrigDevicePath;
          Found = TRUE;
          break;
        }
      }

      //
      // Still searching. Advance to the next device path node.
      //
      DevicePath = NextDevicePathNode(DevicePath);
    }

    //
    // If we found a good device path, stop searching.
    //
    if (Found != FALSE) {
      DEBUG((DEBUG_ERROR, "%a: found GPT partition Index:%d\n", __FUNCTION__, Index));
      break;
    }
  }

  //
  // If a suitable handle was not found, return error.
  //
  if (Found == FALSE) {
    Status = EFI_NOT_FOUND;
    goto CleanUp;
  }

  Status = gBS->HandleProtocol(HandleBuffer[Index],
                               &gEfiSimpleFileSystemProtocolGuid,
                               (VOID**)&SfProtocol);

  if (EFI_ERROR(Status) != FALSE) {
    DEBUG((DEBUG_ERROR, "%a: Failed to locate Simple FS protocol using the handle to fs0: %r \n", __FUNCTION__, Status));
    goto CleanUp;
  }

  //
  // Open the volume/partition.
  //
  Status = SfProtocol->OpenVolume(SfProtocol, Fs_Handle);
  if (EFI_ERROR(Status) != FALSE) {
    DEBUG((DEBUG_ERROR, "%a: Failed to open Simple FS volume fs0: %r \n", __FUNCTION__, Status));
    goto CleanUp;
  }

CleanUp:
  if (HandleBuffer != NULL) {
    FreePool(HandleBuffer);
  }

  return Status;
}
/**
  This helper function walks the page tables to retrieve:
  - a count of each entry
  - a count of each directory entry
  - [optional] a flat list of each entry
  - [optional] a flat list of each directory entry

  @param[in, out]   Pte1GCount, Pte2MCount, Pte4KCount, PdeCount
      On input, the number of entries that can fit in the corresponding buffer (if provided).
      It is expected that this will be zero if the corresponding buffer is NULL.
      On output, the number of entries that were encountered in the page table.
  @param[out]       Pte1GEntries, Pte2MEntries, Pte4KEntries, PdeEntries
      A buffer which will be filled with the entries that are encountered in the tables.

  @retval     EFI_SUCCESS             All requested data has been returned.
  @retval     EFI_INVALID_PARAMETER   One or more of the count parameter pointers is NULL.
  @retval     EFI_INVALID_PARAMETER   Presence of buffer counts and pointers is incongruent.
  @retval     EFI_BUFFER_TOO_SMALL    One or more of the buffers was insufficient to hold
                                      all of the entries in the page tables. The counts
                                      have been updated with the total number of entries
                                      encountered.

**/
STATIC
EFI_STATUS
GetFlatPageTableData (
  IN OUT UINTN                    *Pte1GCount,
  IN OUT UINTN                    *Pte2MCount,
  IN OUT UINTN                    *Pte4KCount,
  IN OUT UINTN                    *PdeCount,
  IN OUT UINTN                    *GuardCount,
  OUT PAGE_TABLE_1G_ENTRY         *Pte1GEntries,
  OUT PAGE_TABLE_ENTRY            *Pte2MEntries,
  OUT PAGE_TABLE_4K_ENTRY         *Pte4KEntries,
  OUT UINT64                      *PdeEntries,
  OUT UINT64                      *GuardEntries
  )
{
  EFI_STATUS                      Status = EFI_SUCCESS;
  PAGE_MAP_AND_DIRECTORY_POINTER  *Work;
  PAGE_MAP_AND_DIRECTORY_POINTER  *Pml4;
  PAGE_TABLE_1G_ENTRY             *Pte1G;
  PAGE_TABLE_ENTRY                *Pte2M;
  PAGE_TABLE_4K_ENTRY             *Pte4K;
  UINTN                           Index1;
  UINTN                           Index2;
  UINTN                           Index3;
  UINTN                           Index4;
  UINTN                           MyGuardCount = 0;
  UINTN                           MyPdeCount = 0;
  UINTN                           My4KCount = 0;
  UINTN                           My2MCount = 0;
  UINTN                           My1GCount = 0;
  UINTN                           NumPage4KNotPresent = 0;
  UINTN                           NumPage2MNotPresent = 0;
  UINTN                           NumPage1GNotPresent = 0;
  UINT64                          Address;

  //
  // First, fail fast if some of the parameters don't look right.
  //
  // ALL count parameters should be provided.
  if (Pte1GCount == NULL || Pte2MCount == NULL || Pte4KCount == NULL || PdeCount == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  // If a count is greater than 0, the corresponding buffer pointer MUST be provided.
  // It will be assumed that all buffers have space for any corresponding count.
  if ((*Pte1GCount > 0 && Pte1GEntries == NULL) || (*Pte2MCount > 0 && Pte2MEntries == NULL) ||
      (*Pte4KCount > 0 && Pte4KEntries == NULL) || (*PdeCount > 0 && PdeEntries == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Alright, let's get to work.
  // 
  Pml4 = (PAGE_MAP_AND_DIRECTORY_POINTER *) AsmReadCr3 ();
  // Increase the count.
  // If we have room for more PDE Entries, add one.
  MyPdeCount++;
  if (MyPdeCount <= *PdeCount) {
    PdeEntries[MyPdeCount-1] = (UINT64)Pml4;
  }

  for (Index4 = 0x0; Index4 < 0x200; Index4 ++) {
    if (!Pml4[Index4].Bits.Present) {
      continue;
    }
    Pte1G = (PAGE_TABLE_1G_ENTRY *)(UINTN)(Pml4[Index4].Bits.PageTableBaseAddress << 12);
    // Increase the count.
    // If we have room for more PDE Entries, add one.
    MyPdeCount++;
    if (MyPdeCount <= *PdeCount) {
      PdeEntries[MyPdeCount-1] = (UINT64)Pte1G;
    }
    for (Index3 = 0x0;  Index3 < 0x200; Index3 ++ ) {
      if (!Pte1G[Index3].Bits.Present) {
        NumPage1GNotPresent++;
        continue;
      }
      //
      // MustBe1 is the bit that indicates whether the pointer is a directory
      // pointer or a page table entry.
      //
      if (!(Pte1G[Index3].Bits.MustBe1)) {
        //
        // We have to cast 1G and 2M directories to this to
        // get all of their address bits.
        //
        Work = (PAGE_MAP_AND_DIRECTORY_POINTER *) Pte1G;
        Pte2M = (PAGE_TABLE_ENTRY *)(UINTN)(Work[Index3].Bits.PageTableBaseAddress << 12);
        // Increase the count.
        // If we have room for more PDE Entries, add one.
        MyPdeCount ++;
        if (MyPdeCount <= *PdeCount) {
          PdeEntries[MyPdeCount-1] = (UINT64)Pte2M;
        }
        for (Index2 = 0x0; Index2 < 0x200; Index2 ++ ) {
          if (!Pte2M[Index2].Bits.Present) {
            NumPage2MNotPresent++;
            continue;
          }
          if (!(Pte2M[Index2].Bits.MustBe1)) {
            Work = (PAGE_MAP_AND_DIRECTORY_POINTER *) Pte2M;
            Pte4K = (PAGE_TABLE_4K_ENTRY *)(UINTN)(Work[Index2].Bits.PageTableBaseAddress << 12);
            // Increase the count.
            // If we have room for more PDE Entries, add one.
            MyPdeCount ++;
            if (MyPdeCount <= *PdeCount) {
              PdeEntries[MyPdeCount-1] = (UINT64)Pte4K;
            }
            for (Index1 = 0x0; Index1 < 0x200; Index1 ++ ) {
              if (!Pte4K[Index1].Bits.Present) {
                NumPage4KNotPresent++;
                Address = IndexToAddress(Index4, Index3, Index2, Index1);
                if ((mHgDumpBitMap != NULL) && (mHgDumpBitMap->IsGuardPage(Address))) {
                  MyGuardCount ++;
                  if (MyGuardCount <= *GuardCount) {
                    GuardEntries[MyGuardCount - 1] = Address;
                  }
                }
                continue;
              }
              // Increase the count.
              // If we have room for more Page Table entries, add one.
              My4KCount++;
              if (My4KCount <= *Pte4KCount) {
                Pte4KEntries[My4KCount-1] = Pte4K[Index1];
              }
            }
          }
          else {
            // Increase the count.
            // If we have room for more Page Table entries, add one.
            My2MCount++;
            if (My2MCount <= *Pte2MCount) {
              Pte2MEntries[My2MCount-1] = Pte2M[Index2];
            }
          }
        }
      }
      else {
        // Increase the count.
        // If we have room for more Page Table entries, add one.
        My1GCount++;
        if (My1GCount <= *Pte1GCount) {
          Pte1GEntries[My1GCount-1] = Pte1G[Index3];
        }
      }
    }
  }

  DEBUG(( DEBUG_ERROR, "Pages used for Page Tables   = %d\n", MyPdeCount ));
  DEBUG(( DEBUG_ERROR, "Number of   4K Pages active  = %d - NotPresent = %d\n", My4KCount, NumPage4KNotPresent ));
  DEBUG(( DEBUG_ERROR, "Number of   2M Pages active  = %d - NotPresent = %d\n", My2MCount, NumPage2MNotPresent ));
  DEBUG(( DEBUG_ERROR, "Number of   1G Pages active  = %d - NotPresent = %d\n", My1GCount, NumPage1GNotPresent ));
  DEBUG(( DEBUG_ERROR, "Number of   Guard Pages active  = %d\n",MyGuardCount));

  //
  // Deteremine whether any of the buffers were too small.
  // Only matters if a given buffer was provided.
  //
  if ((Pte1GEntries != NULL && *Pte1GCount < My1GCount) || (Pte2MEntries != NULL && *Pte2MCount < My2MCount) ||
      (Pte4KEntries != NULL && *Pte4KCount < My4KCount) || (PdeEntries != NULL && *PdeCount < MyPdeCount)) {
    Status = EFI_BUFFER_TOO_SMALL;
  }

  //
  // Update all the return pointers.
  //
  *Pte1GCount = My1GCount;
  *Pte2MCount = My2MCount;
  *Pte4KCount = My4KCount;
  *PdeCount = MyPdeCount;
  *GuardCount = MyGuardCount;

  return Status;
} // GetFlatPageTableData()


STATIC
BOOLEAN
LoadFlatPageTableData(
  OUT UINTN                       *Pte1GCount,
  OUT UINTN                       *Pte2MCount,
  OUT UINTN                       *Pte4KCount,
  OUT UINTN                       *PdeCount,
  OUT UINTN                       *GuardCount,
  OUT PAGE_TABLE_1G_ENTRY         **Pte1GEntries,
  OUT PAGE_TABLE_ENTRY            **Pte2MEntries,
  OUT PAGE_TABLE_4K_ENTRY         **Pte4KEntries,
  OUT UINT64                      **PdeEntries,
  OUT UINT64                      **GuardEntries
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
 
  // Run once to get counts.
  DEBUG(( DEBUG_ERROR, "%a - First call to determine required buffer sizes.\n", __FUNCTION__ ));
  *Pte1GCount = 0;
  *Pte2MCount = 0;
  *Pte4KCount = 0;
  *PdeCount = 0;
  *GuardCount = 0;
  Status = GetFlatPageTableData( Pte1GCount, Pte2MCount, Pte4KCount, PdeCount, GuardCount, NULL, NULL, NULL, NULL, NULL );

  (*Pte1GCount) += 15;
  (*Pte2MCount) += 15;
  (*Pte4KCount) += 15;
  (*PdeCount) += 15;

  // Allocate buffers if successful.
  if (!EFI_ERROR( Status )) {
    *Pte1GEntries = AllocateZeroPool( *Pte1GCount * sizeof( PAGE_TABLE_1G_ENTRY ) );
    *Pte2MEntries = AllocateZeroPool( *Pte2MCount * sizeof( PAGE_TABLE_ENTRY ) );
    *Pte4KEntries = AllocateZeroPool( *Pte4KCount * sizeof( PAGE_TABLE_4K_ENTRY ) );
    *PdeEntries = AllocateZeroPool( *PdeCount * sizeof( UINT64 ) );
    *GuardEntries = AllocateZeroPool( *GuardCount * sizeof( UINT64 ) );

    // Check for errors.
    if (*Pte1GEntries == NULL || *Pte2MEntries == NULL || *Pte4KEntries == NULL || *PdeEntries == NULL || *GuardEntries == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    }
  }

  // If still good, grab the data.
  if (!EFI_ERROR( Status )) {
    DEBUG(( DEBUG_INFO, "%a - Second call to grab the data.\n", __FUNCTION__ ));
    Status = GetFlatPageTableData( Pte1GCount, Pte2MCount, Pte4KCount, PdeCount, GuardCount,
                                   *Pte1GEntries, *Pte2MEntries, *Pte4KEntries, *PdeEntries, *GuardEntries );
    if (Status == EFI_BUFFER_TOO_SMALL)
    {
      FreePool( *Pte1GEntries );
      FreePool( *Pte2MEntries );
      FreePool( *Pte4KEntries );
      FreePool( *PdeEntries );
      FreePool( *GuardEntries );

      (*Pte1GCount) += 15;
      (*Pte2MCount) += 15;
      (*Pte4KCount) += 15;
      (*PdeCount) += 15;
      (*GuardCount) += 15;

      *Pte1GEntries = AllocateZeroPool( *Pte1GCount * sizeof( PAGE_TABLE_1G_ENTRY ) );
      *Pte2MEntries = AllocateZeroPool( *Pte2MCount * sizeof( PAGE_TABLE_ENTRY ) );
      *Pte4KEntries = AllocateZeroPool( *Pte4KCount * sizeof( PAGE_TABLE_4K_ENTRY ) );
      *PdeEntries = AllocateZeroPool( *PdeCount * sizeof( UINT64 ) );
      *GuardEntries = AllocateZeroPool( *GuardCount * sizeof( UINT64 ) );


      Status = GetFlatPageTableData( Pte1GCount, Pte2MCount, Pte4KCount, PdeCount, GuardCount,
                                     *Pte1GEntries, *Pte2MEntries, *Pte4KEntries, *PdeEntries, *GuardEntries );
    }
  }

  // If an error occurred, bail and free.
  if (EFI_ERROR( Status )) {
    if (*Pte1GEntries != NULL) {
      FreePool( *Pte1GEntries );
      *Pte1GEntries = NULL;
    }
    if (*Pte2MEntries != NULL) {
      FreePool( *Pte2MEntries );
      *Pte2MEntries = NULL;
    }
    if (*Pte4KEntries != NULL) {
      FreePool( *Pte4KEntries );
      *Pte4KEntries = NULL;
    }
    if (*PdeEntries != NULL) {
      FreePool( *PdeEntries );
      *PdeEntries = NULL;
    }
    if (*GuardEntries != NULL) {
      FreePool( *GuardEntries );
      *GuardEntries = NULL;
    }
    *Pte1GCount = 0;
    *Pte2MCount = 0;
    *Pte4KCount = 0;
    *PdeCount = 0;
    *GuardCount = 0;
  }

  return !EFI_ERROR( Status );
}

/**
  This helper function will flush the MemoryInfoDatabase to its corresponding
  file and free all resources currently associated with it.

  @param[in]  FileName    Name of the file to be flushed to.

  @retval     EFI_SUCCESS     Database has been flushed to file.

**/
STATIC
EFI_STATUS
FlushAndClearMemoryInfoDatabase (
  IN CONST CHAR16     *FileName
  )
{
  // If we have database contents, flush them to the file.
  if (mMemoryInfoDatabaseSize > 0) {
    WriteBufferToFile( FileName, mMemoryInfoDatabaseBuffer, mMemoryInfoDatabaseSize );
  }

  // If we have a database, free it, and reset all counters.
  if (mMemoryInfoDatabaseBuffer != NULL) {
    FreePool( mMemoryInfoDatabaseBuffer );
    mMemoryInfoDatabaseBuffer = NULL;
  }
  mMemoryInfoDatabaseAllocSize = 0;
  mMemoryInfoDatabaseSize = 0;

  return EFI_SUCCESS;
} // FlushAndClearMemoryInfoDatabase()



/**
   Event notification handler. Will dump paging information to disk.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
DumpPagingInfo (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
  static UINTN                           Pte1GCount = 0;
  static UINTN                           Pte2MCount = 0;
  static UINTN                           Pte4KCount = 0;
  static UINTN                           PdeCount = 0;
  static UINTN                           GuardCount = 0;
  static PAGE_TABLE_1G_ENTRY            *Pte1GEntries = NULL;
  static PAGE_TABLE_ENTRY               *Pte2MEntries = NULL;
  static PAGE_TABLE_4K_ENTRY            *Pte4KEntries = NULL;
  static UINT64                         *PdeEntries = NULL;
  static UINT64                         *GuardEntries = NULL;
  CHAR8                                  TempString[MAX_STRING_SIZE];

  Status = gBS->LocateProtocol(&gHeapGuardDebugProtocolGuid, NULL, (VOID **) &mHgDumpBitMap);
  if (EFI_ERROR(Status)){
    DEBUG((DEBUG_ERROR, "%a error finding hg bitmap protocol - %r\n", __FUNCTION__, Status));
    mHgDumpBitMap = NULL;
  }
  Status = DflDxeOpenVolumeSFS (&mFs_Handle);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a error opening sfs volume - %r\n", __FUNCTION__, Status));
    return;
  }

  if (LoadFlatPageTableData( &Pte1GCount, &Pte2MCount, &Pte4KCount, &PdeCount, &GuardCount,
                         &Pte1GEntries, &Pte2MEntries, &Pte4KEntries, &PdeEntries, &GuardEntries ))
  {
    DflDxeCreateAndWriteFileSFS(mFs_Handle, L"1G.dat", Pte1GCount * sizeof( PAGE_TABLE_1G_ENTRY ), Pte1GEntries);
    DflDxeCreateAndWriteFileSFS(mFs_Handle, L"2M.dat", Pte2MCount * sizeof( PAGE_TABLE_ENTRY ), Pte2MEntries);
    DflDxeCreateAndWriteFileSFS(mFs_Handle, L"4K.dat", Pte4KCount * sizeof( PAGE_TABLE_4K_ENTRY ), Pte4KEntries);
    DflDxeCreateAndWriteFileSFS(mFs_Handle, L"PDE.dat", PdeCount * sizeof( UINT64 ), PdeEntries);
  }

  for (UINT64 i = 0; i < GuardCount; i ++) {
    AsciiSPrint( TempString, MAX_STRING_SIZE,
      "GuardPage,0x%016lx\n",
      GuardEntries[i] );
    DEBUG((DEBUG_ERROR, "%a  %s\n", __FUNCTION__, TempString));
    AppendToMemoryInfoDatabase( TempString );
  }

  FlushAndClearMemoryInfoDatabase( L"GuardPage" );
  TSEGDumpHandler();
  MemoryMapDumpHandler();
  LoadedImageTableDump();
  MemoryAttributesTableDump();
  FlushAndClearMemoryInfoDatabase( L"MemoryInfoDatabase" );

  DEBUG((DEBUG_ERROR, "%a leave - %r\n", __FUNCTION__, Status));
}

/**
  SmmPagingAuditAppEntryPoint

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point executed successfully.
  @retval other           Some error occured when executing this entry point.

**/
EFI_STATUS
EFIAPI
PagingAuditDxeEntryPoint (
  IN     EFI_HANDLE         ImageHandle,
  IN     EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS    Status = EFI_SUCCESS;
  DEBUG((DEBUG_ERROR, "%a registered - %r\n", __FUNCTION__, Status));
  EFI_EVENT                         Event;

  Status = gBS->CreateEventEx (
                      EVT_NOTIFY_SIGNAL,
                      TPL_CALLBACK,
                      DumpPagingInfo,
                      NULL,
                      &gEfiEventExitBootServicesGuid,
                      &Event
                      );
  // // DumpPagingInfo();
  DEBUG((DEBUG_ERROR, "%a leave - %r\n", __FUNCTION__, Status));

  return EFI_SUCCESS;
} // SmmPagingAuditAppEntryPoint()
