/** @file
  Provides services to encrypt/decrypt variables.

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _ENCRYPTION_VARIABLE_LIB_H_
#define _ENCRYPTION_VARIABLE_LIB_H_

#include <IndustryStandard/Tpm20.h>

#include <Guid/VariableFormat.h>

#include <Library/AuthVariableLib.h>

#define ENC_TYPE_NULL         0
#define ENC_TYPE_AES          TPM_ALG_AES

typedef struct {
  AUTH_VARIABLE_INFO  Header;
  VARIABLE_HEADER     *Address;
  UINT32              Offset;
  VOID                *PlainData;
  UINT32              PlainDataSize;
  VOID                *CipherData;
  UINT32              CipherDataSize;
  UINT32              CipherHeaderSize;
  UINT32              CipherDataType;
  VOID                *Key;
  UINT32              KeySize;
  struct {
    BOOLEAN           Auth;
    BOOLEAN           DecryptInPlace;
  }                   Flags;
} VARIABLE_ENCRYPTION_INFO;

/**
  Encrypt variable data.

  @param[in, out]   VarInfo   Pointer to structure containing detailed information about a variable.

  @retval EFI_SUCCESS               Function successfully executed.
  @retval EFI_INVALID_PARAMETER     If ProtectedVarLibContextIn == NULL or ProtectedVarLibContextOut == NULL.
  @retval EFI_OUT_OF_RESOURCES      Fail to allocate enough resource.
  @retval EFI_UNSUPPORTED           Unsupported to process authenticated variable.

**/
EFI_STATUS
EFIAPI
EncryptVariable (
  IN OUT VARIABLE_ENCRYPTION_INFO   *VarInfo
  );

/**
  Process variable with EFI_VARIABLE_TIME_BASED_PROTECTEDENTICATED_WRITE_ACCESS set.

  @param[in] VariableName           Name of the variable.
  @param[in] VendorGuid             Variable vendor GUID.
  @param[in] Data                   Data pointer.
  @param[in] DataSize               Size of Data.
  @param[in] Attributes             Attribute value of the variable.

  @retval EFI_SUCCESS               The firmware has successfully stored the variable and its data as
                                    defined by the Attributes.
  @retval EFI_INVALID_PARAMETER     Invalid parameter.
  @retval EFI_WRITE_PROTECTED       Variable is write-protected.
  @retval EFI_OUT_OF_RESOURCES      There is not enough resource.
  @retval EFI_SECURITY_VIOLATION    The variable is with EFI_VARIABLE_TIME_BASED_PROTECTEDENTICATED_WRITE_ACESS
                                    set, but the AuthInfo does NOT pass the validation
                                    check carried out by the firmware.
  @retval EFI_UNSUPPORTED           Unsupported to process authenticated variable.

**/
EFI_STATUS
EFIAPI
DecryptVariable (
  IN OUT VARIABLE_ENCRYPTION_INFO   *VarInfo
  );

EFI_STATUS
EFIAPI
GetCipherDataInfo (
  IN  OUT VARIABLE_ENCRYPTION_INFO    *VarInfo
  );

EFI_STATUS
EFIAPI
SetCipherDataInfo (
  IN  OUT VARIABLE_ENCRYPTION_INFO    *VarInfo
  );

#endif  //_ENCRYPTION_VARIABLE_LIB_H_
