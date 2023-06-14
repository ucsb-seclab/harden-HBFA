#ifdef TEST_WITH_LIBFUZZER
#include <stdint.h>
#include <stddef.h>
#endif

#include <Uefi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SmmMemLibStubLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

#include <../../../../../edk2/EmulatorPkg/Demo1_Variable/Demo1_Variable.h>
#include <../../../../../edk2/EmulatorPkg/Demo1_Access_Key/Demo1_Access_Key.h>

#include "../../../../../edk2/MdeModulePkg/Universal/Variable/RuntimeDxe/Variable.h"
#include <Guid/VariableFormat.h>

#define TOTAL_SIZE (512 * 1024)
#define DEBUGMODE 0

extern Demo1_Access_Key_PROTOCOL gDemo1_Access_Key_Protocol;
extern EFI_RNG_PROTOCOL *RngProtocol;
extern Demo1_Access_Key_PROTOCOL *AccessKeyProtocol;
extern EFI_RNG_PROTOCOL mRngRdRand;
extern DEMO1_ACCESS_KEY *masterKey;

extern VARIABLE_MODULE_GLOBAL *mineVariableModuleGlobal;

extern VARIABLE_STORE_HEADER *mineNvVariableCache;
extern VARIABLE_INFO_ENTRY *gVarInfo;

typedef struct _LINK LINK;

struct _LINK
{ // doubly linked list of keys
    DEMO1_ACCESS_KEY access_key;
    LINK *next;
    LINK *prev;
};

extern LINK *head;
extern LINK *last;
extern LINK *keychain;

EFI_STATUS
EFIAPI
mineVariableServiceGetVariable(
    IN CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN DEMO1_ACCESS_KEY *AccessKey,
    IN OUT UINTN *DataSize,
    OUT VOID *Data OPTIONAL);

EFI_STATUS
EFIAPI
mineVariableServiceSetVariable(
    IN CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes,
    IN DEMO1_ACCESS_KEY *AccessKey,
    IN UINTN DataSize,
    IN VOID *Data);

EFI_STATUS
mineInitNonVolatileVariableStore(
    VOID);

EFI_STATUS
mineInitEmuNonVolatileVariableStore(
    OUT EFI_PHYSICAL_ADDRESS *VariableStoreBase);

VOID FixBuffer(
    UINTN *TestBuffer,
    UINTN TestBufferSize)
{
}

UINTN
EFIAPI
GetMaxBufferSize(
    VOID)
{
    return TOTAL_SIZE;
}

VOID
    EFIAPI
    RunTestHarness(
        IN VOID *TestBuffer,
        IN UINTN TestBufferSize)
{
    FixBuffer(TestBuffer, TestBufferSize);

    EFI_STATUS Status;
    EFI_HANDLE *Handle = NULL;
    BOOLEAN retbool;

    masterKey = AllocatePool(sizeof(DEMO1_ACCESS_KEY));

    Status = gBS->InstallProtocolInterface(
        &Handle,
        &gDemo1AccessKeyProtocolGuid,
        EFI_NATIVE_INTERFACE,
        &gDemo1_Access_Key_Protocol);
    if (EFI_ERROR(Status))
    {
        ASSERT(FALSE);
    }

    Status = gBS->LocateProtocol(&gDemo1AccessKeyProtocolGuid, NULL, (VOID *)&AccessKeyProtocol);
    if (EFI_ERROR(Status))
    {
        ASSERT(FALSE);
    }

    Status = gBS->InstallMultipleProtocolInterfaces(
        &Handle,
        &gEfiRngProtocolGuid,
        &mRngRdRand,
        NULL);
    if (EFI_ERROR(Status))
    {
        ASSERT(FALSE);
    }

    Status = gBS->LocateProtocol(&gEfiRngProtocolGuid, NULL, (VOID **)&RngProtocol);
    if (EFI_ERROR(Status))
    {
        ASSERT(FALSE);
    }

    //
    // Allocate Variable Storage
    //
    mineVariableModuleGlobal = AllocateRuntimeZeroPool(sizeof(VARIABLE_MODULE_GLOBAL));
    if (mineVariableModuleGlobal == NULL)
    {
        ASSERT(FALSE);
    }

    //
    // Initialize Non-volatile Variable Store
    //
    Status = mineInitNonVolatileVariableStore();
    if (EFI_ERROR(Status) || mineVariableModuleGlobal == NULL || mineNvVariableCache == NULL)
    {
        FreePool(mineVariableModuleGlobal);
        ASSERT(FALSE);
    }

    // Status = Demo1GenerateAccessKey(&gDemo1_Access_Key_Protocol, NULL, TRUE, masterKey);
    Status = AccessKeyProtocol->Demo1GenerateAccessKey(AccessKeyProtocol, NULL, TRUE, masterKey);
    if (EFI_ERROR(Status))
    {
        ASSERT(FALSE);
    }

    UINTN testVal = 0xa;

    UINTN BufferVal = *(UINTN **)TestBuffer;
    Status = mineVariableServiceSetVariable(
        L"ExampleVar",
        &gExampleVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
        masterKey,
        BufferVal + 1, // to avoid DataSize==0 error
        &testVal);
    if (EFI_ERROR(Status) && Status != EFI_INVALID_PARAMETER)
    {
        ASSERT(FALSE);
    }
}
