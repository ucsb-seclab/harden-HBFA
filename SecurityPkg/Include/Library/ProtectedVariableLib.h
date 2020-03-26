/** @file
  Defines interfaces of protected variable services for non-volatile variable
  storage.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PROTECTED_VARIABLE_LIB_H_
#define _PROTECTED_VARIABLE_LIB_H_

#include <PiPei.h>
#include <PiDxe.h>

#include <Guid/VariableFormat.h>

#include <Protocol/VarCheck.h>
#include <Protocol/FirmwareVolumeBlock.h>

#include <Library/UefiLib.h>
#include <Library/EncryptionVariableLib.h>

#define DEFAULT_COUNTER_INDEX         0

typedef VARIABLE_ENCRYPTION_INFO      PROTECTED_VARIABLE_INFO;

/**

  This function writes data to the NV variable storage at given position.

  Note: Per current variable service architecture, only SMM is allowed to
        (directly) change NV variable storage.

  @param VariableInfo             Pointer to structure holding details of a variable.
  @param Offset                   Offset to the given variable to write from.
  @param Size                     Size of data to be written.
  @param Buffer                   Pointer to the buffer from which data is written.

  @retval EFI_INVALID_PARAMETER  Invalid parameters passed in.
  @retval EFI_UNSUPPORTED        Updating NV variable storage is not supported.
  @retval EFI_OUT_OF_RESOURCES   Not enough resource to complete the operation.
  @retval EFI_SUCCESS            Variable store successfully updated.

**/
typedef
EFI_STATUS
(EFIAPI *PROTECTED_VAR_LIB_UPDATE_VARIABLE_STORE) (
  IN  PROTECTED_VARIABLE_INFO     *VariableInfo,
  IN  UINTN                       Offset,
  IN  UINT32                      Size,
  IN  UINT8                       *Buffer
  );

/**

  Retrieve details about a variable and return them in VariableInfo->Header.

  If VariableInfo->Address is given, this function will calculate its offset
  relative to given variable storage via VariableStore; Otherwise, it will try
  other internal variable storages or cached copies. It's assumed that, for all
  copies of NV variable storage, all variables are stored in the same relative
  position. If VariableInfo->Address is found in the range of any storage copies,
  its offset relative to that storage should be the same in other copies.

  If VariableInfo->Offset is given (non-zero) but not VariableInfo->Address,
  this function will return the variable memory address inside VariableStore,
  if given, via VariableInfo->Address; Otherwise, the address of other storage
  copies will be returned, if any.

  For a new variable whose offset has not been determined, a value of -1 as
  VariableInfo->Offset should be passed to skip the offset calculation.

  @param VariableStore            Pointer to a variable storage. It's optional.
  @param VariableInfo             Pointer to variable information.

  @retval EFI_INVALID_PARAMETER  VariableInfo is NULL or both VariableInfo->Address
                                 and VariableInfo->Offset are NULL (0).
  @retval EFI_NOT_FOUND          If given Address or Offset is out of range of
                                 any given or internal storage copies.
  @retval EFI_SUCCESS            Variable details are retrieved successfully.

**/
typedef
EFI_STATUS
(EFIAPI *PROTECTED_VAR_LIB_GET_VAR_INFO) (
  IN      VARIABLE_STORE_HEADER     *VariableStore OPTIONAL,
  IN  OUT PROTECTED_VARIABLE_INFO   *VariableInfo
  );

/**

  Retrieve details of the variable next to given variable within VariableStore.

  If VarInfo->Address is NULL, the first one in VariableStore is returned.

  VariableStart and/or VariableEnd can be given optionally for the situation
  in which the valid storage space is smaller than the VariableStore->Size.
  This usually happens when PEI variable services make a compact variable
  cache to save memory, which cannot make use VariableStore->Size to determine
  the correct variable storage range.

  @param VariableStore            Pointer to a variable storage. It's optional.
  @param VariableStart            Start point of valid range in VariableStore.
  @param VariableEnd              End point of valid range in VariableStore.
  @param VariableInfo             Pointer to variable information.

  @retval EFI_INVALID_PARAMETER  VariableInfo or VariableStore is NULL.
  @retval EFI_NOT_FOUND          If the end of VariableStore is reached.
  @retval EFI_SUCCESS            The next variable is retrieved successfully.

**/
typedef
EFI_STATUS
(EFIAPI *PROTECTED_VAR_LIB_GET_NEXT_VAR_INFO) (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN      VARIABLE_HEADER           *VariableStart OPTIONAL,
  IN      VARIABLE_HEADER           *VariableEnd OPTIONAL,
  IN  OUT PROTECTED_VARIABLE_INFO   *VariableInfo
  );

/**

  Initialize a memory copy of NV variable storage.

  To save memory consumption (especially in PEI phase), it's allowed to cache
  only valid variables. In such case, an index table recording offset of each
  valid variables could be employed. The index table makes sure the cached copy
  to be synchronized with the original copy in NV variable storage. To avoid
  TOCTOU issue, once the variables are cached in memory and verified, NV
  variable storage should not be used to read variable information. The cached
  copy should be used instead.

  If StoreCacheBase is not given, this function should return the required
  cache size and valid variable number, if VariableNumber is not NULL. Then
  the caller can prepare correct cache buffer and index table buffer before
  next calling.

  @param[in]      StoreCacheBase  Base address of NV variable storage cache.
  @param[in,out]  CacheSize       Size of buffer given by StoreCacheBase.
  @param[out]     CacheSize       Size of required cache buffer.
  @param[out]     IndexTable      Offset array, in order, of each valid variable.
  @param[out]     VariableNumber  Number of valid variables.
  @param[out]     AuthFlag        Auth-variable indicator.

  @retval EFI_INVALID_PARAMETER   CacheSize is NULL; Or,
                                  StoreCacheBase is 0 but *CacheSize it not.
  @retval EFI_VOLUME_CORRUPTED    If original NV variable storage is corrupted.
  @retval EFI_BUFFER_TOO_SMALL    If *CacheSize is smaller than required memory.
  @retval EFI_SUCCESS             The cached variable storage is initialized.

**/
typedef
EFI_STATUS
(EFIAPI *PROTECTED_VAR_LIB_INIT_VAR_STORE) (
     OUT  EFI_PHYSICAL_ADDRESS      StoreCacheBase OPTIONAL,
  IN OUT  UINT32                    *CacheSize,
     OUT  UINT32                    *IndexTable OPTIONAL,
     OUT  UINT32                    *VariableNumber OPTIONAL,
     OUT  BOOLEAN                   *AuthFlag OPTIONAL
  );

/**

  Initiate a variable retrieval in SMM environment from non-SMM environment.

  This is usually required in BS/RT environment when local cached copy is in
  encrypted form. Variable decryption can only be done in SMM environment.

  @param[in]      VariableName       Name of Variable to be found.
  @param[in]      VendorGuid         Variable vendor GUID.
  @param[out]     Attributes         Attribute value of the variable found.
  @param[in, out] DataSize           Size of Data found. If size is less than the
                                     data, this value contains the required size.
  @param[out]     Data               Data pointer.

  @retval EFI_SUCCESS                Found the specified variable.
  @retval EFI_INVALID_PARAMETER      Invalid parameter.
  @retval EFI_NOT_FOUND              The specified variable could not be found.

**/
typedef
EFI_STATUS
(EFIAPI *PROTECTED_VAR_LIB_FIND_VAR_SMM) (
  IN      CHAR16                     *VariableName,
  IN      EFI_GUID                   *VendorGuid,
      OUT UINT32                     *Attributes OPTIONAL,
  IN  OUT UINTN                      *DataSize,
      OUT VOID                       *Data OPTIONAL
  );

/**
  Check if a variable is user variable or not.

  @param[in] Variable   Pointer to variable header.

  @retval TRUE          User variable.
  @retval FALSE         System variable.

**/
typedef
BOOLEAN
(EFIAPI *PROTECTED_VAR_LIB_IS_USER_VAR) (
  IN VARIABLE_HEADER    *Variable
  );

typedef enum {
  FromPeiModule,
  FromBootServiceModule,
  FromRuntimeModule,
  FromSmmModule
} VARIABLE_SERVICE_USER;

#pragma pack(1)

#define PROTECTED_VARIABLE_CONTEXT_IN_STRUCT_VERSION  0x01

typedef struct _PROTECTED_VARIABLE_CONTEXT_IN {
  UINT32                                      StructVersion;
  UINT32                                      StructSize;
  UINT32                                      MaxVariableSize;

  VARIABLE_SERVICE_USER                       VariableServiceUser;

  PROTECTED_VAR_LIB_INIT_VAR_STORE            InitVariableStore;
  PROTECTED_VAR_LIB_FIND_VAR_SMM              FindVariableSmm;
  PROTECTED_VAR_LIB_GET_VAR_INFO              GetVariableInfo;
  PROTECTED_VAR_LIB_GET_NEXT_VAR_INFO         GetNextVariableInfo;
  PROTECTED_VAR_LIB_UPDATE_VARIABLE_STORE     UpdateVariableStore;
  PROTECTED_VAR_LIB_IS_USER_VAR               IsUserVariable;
} PROTECTED_VARIABLE_CONTEXT_IN;

#pragma pack()

/**

  Initialization for protected variable services.

  If this initialization failed upon any error, the whole variable services
  should not be used.  A system reset might be needed to re-construct NV
  variable storage to be the default state.

  @param[in]  ContextIn   Pointer to variable service context needed by
                          protected variable.

  @retval EFI_SUCCESS               Protected variable services are ready.
  @retval EFI_INVALID_PARAMETER     If ContextIn == NULL or something missing or
                                    mismatching in the content in ContextIn.
  @retval EFI_COMPROMISED_DATA      If failed to check integrity of protected variables.
  @retval EFI_OUT_OF_RESOURCES      Fail to allocate enough resource.
  @retval EFI_UNSUPPORTED           Unsupported to process protected variable.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibInitialize (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn
  );

/**

  Retrieve plain data, if encrypted, of given variable.

  If variable encryption is employed, this function will initiate a SMM request
  to get the plain data. Due to security consideration, the decryption can only
  be done in SMM environment.

  @param[in]      Variable           Pointer to header of a Variable.
  @param[out]     Data               Pointer to plain data of the given variable.
  @param[in, out] DataSize           Size of data returned or data buffer needed.
  @param[in]      AuthFlag           Auth-variable indicator.

  @retval EFI_SUCCESS                Found the specified variable.
  @retval EFI_INVALID_PARAMETER      Invalid parameter.
  @retval EFI_NOT_FOUND              The specified variable could not be found.
  @retval EFI_BUFFER_TOO_SMALL       If *DataSize is smaller than needed.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibGetData (
  IN      VARIABLE_HEADER                   *Variable,
  IN  OUT VOID                              *Data,
  IN  OUT UINT32                            *DataSize,
  IN      BOOLEAN                           AuthFlag
  );

/**

  Prepare for variable update.

  This is needed only once during current boot to mitigate replay attack. Its
  major job is to advance RPMC (Replay Protected Monotonic Counter).

  @retval EFI_SUCCESS             Variable is ready to update hereafter.
  @retval EFI_UNSUPPORTED         Updating variable is not supported.
  @retval EFI_DEVICE_ERROR        Error in advancing RPMC.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibWriteInit (
  VOID
  );

/**

  Update a variable with protection provided by this library.

  If variable encryption is employed, the new variable data will be encrypted
  before being written to NV variable storage.

  A special variable, called "MetaDataHmacVar", will always be updated along
  with variable being updated to reflect the changes (HMAC value) of all
  protected valid variables. The only exceptions, currently, are variable
  "MetaDataHmacVar" itself and variable "VarErrorLog".

  The buffer passed by NewVariable must be double of maximum variable size,
  which allows to pass the "MetaDataHmacVar" back to caller along with encrypted
  new variable data, if any. This can make sure the new variable data and
  "MetaDataHmacVar" can be written at almost the same time to reduce the chance
  of compromising the integrity.

  If *NewVariableSize is zero, it means to delete variable passed by CurrVariable
  and/or CurrVariableInDel. "MetaDataHmacVar" will be updated as well in such
  case because of less variables in storage. NewVariable should be always passed
  in to convey new "MetaDataHmacVar" back.

  @param[in,out]  CurrVariable        Variable to be updated. It's NULL if
                                      adding a new variable.
  @param[in,out]  CurrVariableInDel   In-delete-transition copy of updating variable.
  @param[in]      NewVariable         Buffer of new variable data.
  @param[out]     NewVariable         Buffer of "MetaDataHmacVar" and new
                                      variable (encrypted).
  @param[in]      NewVariableSize     Size of NewVariable.
  @param[out]     NewVariableSize     Size of (encrypted) NewVariable and
                                      "MetaDataHmacVar".

  @retval EFI_SUCCESS             The variable is updated with protection successfully.
  @retval EFI_INVALID_PARAMETER   NewVariable is NULL.
  @retval EFI_NOT_FOUND           Information missing to finish the operation.
  @retval EFI_ABORTED             Failed to encrypt variable or calculate HMAC.
  @retval EFI_NOT_READY           The RPMC device is not yet initialized.
  @retval EFI_DEVICE_ERROR        The RPMC device has error in updating.
  @retval EFI_ACCESS_DENIED       The given variable is not allowed to update.
                                  Currently this only happens on updating
                                  "MetaDataHmacVar" from code outside of this
                                  library.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibUpdate (
  IN  OUT VARIABLE_HEADER             *CurrVariable OPTIONAL,
  IN  OUT VARIABLE_HEADER             *CurrVariableInDel OPTIONAL,
  IN  OUT VARIABLE_HEADER             *NewVariable,
  IN  OUT UINTN                       *NewVariableSize
  );

/**

  Finalize a variable updating after it's written to NV variable storage
  successfully.

  This usually includes works like increasing RPMC, synchronizing local cache,
  updating new position of "MetaDataHmacVar", deleting old copy of "MetaDataHmacVar"
  completely, etc.

  @param[in]      NewVariable       Buffer of new variables and MetaDataHmacVar.
  @param[in]      VariableSize      Size of buffer pointed by NewVariable.
  @param[in]      Offset            Offset to NV variable storage from where the new
                                    variable and MetaDataHmacVar have been written.

  @retval EFI_SUCCESS         No problem in winding up the variable write operation.
  @retval Others              Failed to updating state of old copy of updated
                              variable, or failed to increase RPMC, etc.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibWriteFinal (
  IN  VARIABLE_HEADER         *NewVariable,
  IN  UINTN                   VariableSize,
  IN  UINTN                   Offset
  );

/**

  Get a verified copy of NV variable storage.

  @param[out]     VariableFvHeader      Pointer to the header of whole NV firmware volume.
  @param[out]     VariableStoreHeader   Pointer to the header of variable storage.

  @retval EFI_SUCCESS             A copy of NV variable storage is returned
                                  successfully.
  @retval EFI_NOT_FOUND           The NV variable storage is not found or cached.
  @retval EFI_UNSUPPORTED         Not supported to get NV variable storage.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibGetStore (
  OUT EFI_FIRMWARE_VOLUME_HEADER            **VariableFvHeader,
  OUT VARIABLE_STORE_HEADER                 **VariableStoreHeader
  );

/**

  Perform garbage collection against the cached copy of NV variable storage.

  @param[in,out]  VariableStoreBuffer         Buffer used to do the reclaim.
  @param[out]     LastVariableOffset          New free space start point.
  @param[in]      CurrVariableOffset          Offset of existing variable.
  @param[in]      CurrVariableInDelOffset     Offset of old copy of existing variable.
  @param[in,out]  NewVariable                 Buffer of new variable data.
  @param[in]      NewVariableSize             Size of new variable data.
  @param[out]     HwErrVariableTotalSize      Total size of variables with HR attribute.
  @param[out]     CommonVariableTotalSize     Total size of common variables.
  @param[out]     CommonUserVariableTotalSize Total size of user variables.

  @retval EFI_SUCCESS               Reclaim is performed successfully.
  @retval EFI_INVALID_PARAMETER     Reclaim buffer is not given or big enough.
  @retval EFI_OUT_OF_RESOURCES      Not enough buffer to complete the reclaim.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibReclaim (
  IN  OUT VARIABLE_STORE_HEADER       *VariableStoreBuffer,
  OUT     UINTN                       *LastVariableOffset,
  IN      UINTN                       CurrVariableOffset,
  IN      UINTN                       CurrVariableInDelOffset,
  IN  OUT VARIABLE_HEADER             **NewVariable,
  IN      UINTN                       NewVariableSize,
  IN  OUT UINTN                       *HwErrVariableTotalSize,
  IN  OUT UINTN                       *CommonVariableTotalSize,
  IN  OUT UINTN                       *CommonUserVariableTotalSize
  );

/**

  An alternative version of ProtectedVariableLibGetData to get plain data, if
  encrypted, from given variable, for different use cases.

  @param[in,out]      VarInfo     Pointer to structure containing variable information.

  @retval EFI_SUCCESS               Found the specified variable.
  @retval EFI_INVALID_PARAMETER     VarInfo is NULL or both VarInfo->Address and
                                    VarInfo->Offset are invalid.
  @retval EFI_NOT_FOUND             The specified variable could not be found.

**/
EFI_STATUS
EFIAPI
ProtectedVariableLibGetDataInfo (
  IN  OUT PROTECTED_VARIABLE_INFO       *VarInfo
  );

#endif
