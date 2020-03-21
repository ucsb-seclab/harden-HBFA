/** @file
  PCI Segment Library that layers on top of the PCI Library which only
   supports segment 0 PCI configuration access.

  Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>

#include <IndustryStandard/Pci22.h>

#include <Library/PciSegmentStubLib.h>

typedef struct {
  // No type, the type can be found IS_PCI_P2P
  BOOLEAN    Discoverable; // If a device under a bridge, and the bridge does not have subordinate, it is not discoverable.
  UINTN      BufferSize; // >= 0x40, <= 0x1000
  // real register
  UINT8      *Buffer;
  // 0 means no touch, 1 means to set to 1.
  UINT8      *OrBuffer; // Always set to 1
  // 0 means clear to 0, 1 means no touch
  UINT8      *AndBuffer; // Always set to 0
} PCI_DEVICE_BUFFER;

UINTN                        mPciDevicesCount;
REGISTER_PCI_DEVICE_STRUCT   *mPciDevices;
PCI_DEVICE_BUFFER            *mPciDevicesBuffer;

PCI_DEVICE_BUFFER *
GetPciDeviceBuffer (
  IN UINT64                    Address
  );

VOID
PciSegmentLibWriteHook (
  IN UINT64                    Address,
  IN UINT32                    Value,
  IN UINT8                     Size
  );

/**
  Register a PCI device so PCI configuration registers may be accessed after
  SetVirtualAddressMap().

  If any reserved bits in Address are set, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @retval RETURN_SUCCESS           The PCI device was registered for runtime access.
  @retval RETURN_UNSUPPORTED       An attempt was made to call this function
                                   after ExitBootServices().
  @retval RETURN_UNSUPPORTED       The resources required to access the PCI device
                                   at runtime could not be mapped.
  @retval RETURN_OUT_OF_RESOURCES  There are not enough resources available to
                                   complete the registration.

**/
RETURN_STATUS
EFIAPI
PciSegmentRegisterForRuntimeAccess (
  IN UINTN  Address
  )
{
  return RETURN_SUCCESS;
}

/**
  Reads an 8-bit PCI configuration register.

  Reads and returns the 8-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.

  @return The 8-bit PCI configuration register specified by Address.

**/
UINT8
EFIAPI
PciSegmentRead8 (
  IN UINT64                    Address
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  PCI_DEVICE_BUFFER               *PciDeviceBuffer;

  LibAddress.Data = Address;
  PciDeviceBuffer = GetPciDeviceBuffer (Address);
  if (PciDeviceBuffer == NULL) {
    return 0xFF;
  }
  if (!PciDeviceBuffer->Discoverable) {
    return 0xFF;
  }

  if (PciDeviceBuffer->BufferSize <= LibAddress.Bits.Register) {
    return 0xFF;
  }

  return (*(UINT8 *)((UINTN)PciDeviceBuffer->Buffer + LibAddress.Bits.Register) &
          *(UINT8 *)((UINTN)PciDeviceBuffer->AndBuffer + LibAddress.Bits.Register)) | 
         *(UINT8 *)((UINTN)PciDeviceBuffer->OrBuffer + LibAddress.Bits.Register);
}

/**
  Writes an 8-bit PCI configuration register.

  Writes the 8-bit PCI configuration register specified by Address with the value specified by Value.
  Value is returned.  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().

  @param  Address     Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  Value       The value to write.

  @return The value written to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentWrite8 (
  IN UINT64                    Address,
  IN UINT8                     Value
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  PCI_DEVICE_BUFFER               *PciDeviceBuffer;

  LibAddress.Data = Address;
  PciDeviceBuffer = GetPciDeviceBuffer (Address);
  if (PciDeviceBuffer == NULL) {
    return 0xFF;
  }
  if (!PciDeviceBuffer->Discoverable) {
    return 0xFF;
  }

  if (PciDeviceBuffer->BufferSize <= LibAddress.Bits.Register) {
    return 0xFF;
  }

  Value = (Value &
          *(UINT8 *)((UINTN)PciDeviceBuffer->AndBuffer + LibAddress.Bits.Register)) | 
         *(UINT8 *)((UINTN)PciDeviceBuffer->OrBuffer + LibAddress.Bits.Register);

  *(UINT8 *)((UINTN)PciDeviceBuffer->Buffer + LibAddress.Bits.Register) = Value;

  PciSegmentLibWriteHook (Address, Value, sizeof(Value));

  return Value;
}

/**
  Performs a bitwise OR of an 8-bit PCI configuration register with an 8-bit value.

  Reads the 8-bit PCI configuration register specified by Address,
  performs a bitwise OR between the read result and the value specified by OrData,
  and writes the result to the 8-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentOr8 (
  IN UINT64                    Address,
  IN UINT8                     OrData
  )
{
  return PciSegmentWrite8 (Address, (UINT8) (PciSegmentRead8 (Address) | OrData));
}

/**
  Performs a bitwise AND of an 8-bit PCI configuration register with an 8-bit value.

  Reads the 8-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  and writes the result to the 8-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.
  If any reserved bits in Address are set, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentAnd8 (
  IN UINT64                    Address,
  IN UINT8                     AndData
  )
{
  return PciSegmentWrite8 (Address, (UINT8) (PciSegmentRead8 (Address) & AndData));
}

/**
  Performs a bitwise AND of an 8-bit PCI configuration register with an 8-bit value,
  followed a  bitwise OR with another 8-bit value.

  Reads the 8-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and the value specified by OrData,
  and writes the result to the 8-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentAndThenOr8 (
  IN UINT64                    Address,
  IN UINT8                     AndData,
  IN UINT8                     OrData
  )
{
  return PciSegmentWrite8 (Address, (UINT8) ((PciSegmentRead8 (Address) & AndData) | OrData));
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in an 8-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.

  @return The value of the bit field read from the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldRead8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit
  )
{
  return BitFieldRead8 (PciSegmentRead8 (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  8-bit register is returned.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  Value     New value of the bit field.

  @return The value written back to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldWrite8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT8                     Value
  )
{
  return PciSegmentWrite8 (
           Address,
           BitFieldWrite8 (PciSegmentRead8 (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in an 8-bit PCI configuration, performs a bitwise OR, and
  writes the result back to the bit field in the 8-bit port.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 8-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized. Extra left bits in OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldOr8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT8                     OrData
  )
{
  return PciSegmentWrite8 (
           Address,
           BitFieldOr8 (PciSegmentRead8 (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in an 8-bit PCI configuration register, performs a bitwise
  AND, and writes the result back to the bit field in the 8-bit register.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 8-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized. Extra left bits in AndData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldAnd8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT8                     AndData
  )
{
  return PciSegmentWrite8 (
           Address,
           BitFieldAnd8 (PciSegmentRead8 (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in an 8-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the 8-bit port.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 8-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldAndThenOr8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT8                     AndData,
  IN UINT8                     OrData
  )
{
  return PciSegmentWrite8 (
           Address,
           BitFieldAndThenOr8 (PciSegmentRead8 (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 16-bit PCI configuration register.

  Reads and returns the 16-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.

  @return The 16-bit PCI configuration register specified by Address.

**/
UINT16
EFIAPI
PciSegmentRead16 (
  IN UINT64                    Address
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  PCI_DEVICE_BUFFER               *PciDeviceBuffer;

  LibAddress.Data = Address;
  PciDeviceBuffer = GetPciDeviceBuffer (Address);
  if (PciDeviceBuffer == NULL) {
    return 0xFFFF;
  }
  if (!PciDeviceBuffer->Discoverable) {
    return 0xFFFF;
  }

  if (PciDeviceBuffer->BufferSize <= LibAddress.Bits.Register) {
    return 0xFFFF;
  }

  return (*(UINT16 *)((UINTN)PciDeviceBuffer->Buffer + LibAddress.Bits.Register) &
          *(UINT16 *)((UINTN)PciDeviceBuffer->AndBuffer + LibAddress.Bits.Register)) | 
         *(UINT16 *)((UINTN)PciDeviceBuffer->OrBuffer + LibAddress.Bits.Register);
}

/**
  Writes a 16-bit PCI configuration register.

  Writes the 16-bit PCI configuration register specified by Address with the value specified by Value.
  Value is returned.  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address     Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  Value       The value to write.

  @return The parameter of Value.

**/
UINT16
EFIAPI
PciSegmentWrite16 (
  IN UINT64                    Address,
  IN UINT16                    Value
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  PCI_DEVICE_BUFFER               *PciDeviceBuffer;

  LibAddress.Data = Address;
  PciDeviceBuffer = GetPciDeviceBuffer (Address);
  if (PciDeviceBuffer == NULL) {
    return 0xFFFF;
  }
  if (!PciDeviceBuffer->Discoverable) {
    return 0xFFFF;
  }

  if (PciDeviceBuffer->BufferSize <= LibAddress.Bits.Register) {
    return 0xFFFF;
  }

  Value = (Value &
          *(UINT16 *)((UINTN)PciDeviceBuffer->AndBuffer + LibAddress.Bits.Register)) | 
         *(UINT16 *)((UINTN)PciDeviceBuffer->OrBuffer + LibAddress.Bits.Register);

  *(UINT16 *)((UINTN)PciDeviceBuffer->Buffer + LibAddress.Bits.Register) = Value;

  PciSegmentLibWriteHook (Address, Value, sizeof(Value));

  return Value;
}

/**
  Performs a bitwise OR of a 16-bit PCI configuration register with
  a 16-bit value.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by OrData, and
  writes the result to the 16-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned. This function
  must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address Address that encodes the PCI Segment, Bus, Device, Function and
                  Register.
  @param  OrData  The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentOr16 (
  IN UINT64                    Address,
  IN UINT16                    OrData
  )
{
  return PciSegmentWrite16 (Address, (UINT16) (PciSegmentRead16 (Address) | OrData));
}

/**
  Performs a bitwise AND of a 16-bit PCI configuration register with a 16-bit value.

  Reads the 16-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  and writes the result to the 16-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentAnd16 (
  IN UINT64                    Address,
  IN UINT16                    AndData
  )
{
  return PciSegmentWrite16 (Address, (UINT16) (PciSegmentRead16 (Address) & AndData));
}

/**
  Performs a bitwise AND of a 16-bit PCI configuration register with a 16-bit value,
  followed a  bitwise OR with another 16-bit value.

  Reads the 16-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and the value specified by OrData,
  and writes the result to the 16-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentAndThenOr16 (
  IN UINT64                    Address,
  IN UINT16                    AndData,
  IN UINT16                    OrData
  )
{
  return PciSegmentWrite16 (Address, (UINT16) ((PciSegmentRead16 (Address) & AndData) | OrData));
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in a 16-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.

  @return The value of the bit field read from the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldRead16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit
  )
{
  return BitFieldRead16 (PciSegmentRead16 (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  16-bit register is returned.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  Value     New value of the bit field.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldWrite16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT16                    Value
  )
{
  return PciSegmentWrite16 (
           Address,
           BitFieldWrite16 (PciSegmentRead16 (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 16-bit PCI configuration, performs a bitwise OR, writes
  the result back to the bit field in the 16-bit port.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 16-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized. Extra left bits in OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldOr16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT16                    OrData
  )
{
  return PciSegmentWrite16 (
           Address,
           BitFieldOr16 (PciSegmentRead16 (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 16-bit PCI configuration register, performs a bitwise
  AND, writes the result back to the bit field in the 16-bit register.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 16-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized. Extra left bits in AndData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldAnd16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT16                    AndData
  )
{
  return PciSegmentWrite16 (
           Address,
           BitFieldAnd16 (PciSegmentRead16 (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 16-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  16-bit port.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 16-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldAndThenOr16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT16                    AndData,
  IN UINT16                    OrData
  )
{
  return PciSegmentWrite16 (
           Address,
           BitFieldAndThenOr16 (PciSegmentRead16 (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 32-bit PCI configuration register.

  Reads and returns the 32-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.

  @return The 32-bit PCI configuration register specified by Address.

**/
UINT32
EFIAPI
PciSegmentRead32 (
  IN UINT64                    Address
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  PCI_DEVICE_BUFFER               *PciDeviceBuffer;

  LibAddress.Data = Address;
  PciDeviceBuffer = GetPciDeviceBuffer (Address);
  if (PciDeviceBuffer == NULL) {
    return 0xFFFFFFFF;
  }
  if (!PciDeviceBuffer->Discoverable) {
    return 0xFFFFFFFF;
  }

  if (PciDeviceBuffer->BufferSize <= LibAddress.Bits.Register) {
    return 0xFFFFFFFF;
  }

  return (*(UINT32 *)((UINTN)PciDeviceBuffer->Buffer + LibAddress.Bits.Register) &
          *(UINT32 *)((UINTN)PciDeviceBuffer->AndBuffer + LibAddress.Bits.Register)) | 
         *(UINT32 *)((UINTN)PciDeviceBuffer->OrBuffer + LibAddress.Bits.Register);
}

/**
  Writes a 32-bit PCI configuration register.

  Writes the 32-bit PCI configuration register specified by Address with the value specified by Value.
  Value is returned.  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address     Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  Value       The value to write.

  @return The parameter of Value.

**/
UINT32
EFIAPI
PciSegmentWrite32 (
  IN UINT64                    Address,
  IN UINT32                    Value
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  PCI_DEVICE_BUFFER               *PciDeviceBuffer;

  LibAddress.Data = Address;
  PciDeviceBuffer = GetPciDeviceBuffer (Address);
  if (PciDeviceBuffer == NULL) {
    return 0xFFFFFFFF;
  }
  if (!PciDeviceBuffer->Discoverable) {
    return 0xFFFFFFFF;
  }

  if (PciDeviceBuffer->BufferSize <= LibAddress.Bits.Register) {
    return 0xFFFFFFFF;
  }

  Value = (Value &
          *(UINT32 *)((UINTN)PciDeviceBuffer->AndBuffer + LibAddress.Bits.Register)) | 
         *(UINT32 *)((UINTN)PciDeviceBuffer->OrBuffer + LibAddress.Bits.Register);

  *(UINT32 *)((UINTN)PciDeviceBuffer->Buffer + LibAddress.Bits.Register) = Value;

  PciSegmentLibWriteHook (Address, Value, sizeof(Value));

  return Value;
}

/**
  Performs a bitwise OR of a 32-bit PCI configuration register with a 32-bit value.

  Reads the 32-bit PCI configuration register specified by Address,
  performs a bitwise OR between the read result and the value specified by OrData,
  and writes the result to the 32-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentOr32 (
  IN UINT64                    Address,
  IN UINT32                    OrData
  )
{
  return PciSegmentWrite32 (Address, PciSegmentRead32 (Address) | OrData);
}

/**
  Performs a bitwise AND of a 32-bit PCI configuration register with a 32-bit value.

  Reads the 32-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  and writes the result to the 32-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentAnd32 (
  IN UINT64                    Address,
  IN UINT32                    AndData
  )
{
  return PciSegmentWrite32 (Address, PciSegmentRead32 (Address) & AndData);
}

/**
  Performs a bitwise AND of a 32-bit PCI configuration register with a 32-bit value,
  followed a  bitwise OR with another 32-bit value.

  Reads the 32-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and the value specified by OrData,
  and writes the result to the 32-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentAndThenOr32 (
  IN UINT64                    Address,
  IN UINT32                    AndData,
  IN UINT32                    OrData
  )
{
  return PciSegmentWrite32 (Address, (PciSegmentRead32 (Address) & AndData) | OrData);
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in a 32-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.

  @return The value of the bit field read from the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldRead32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit
  )
{
  return BitFieldRead32 (PciSegmentRead32 (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  32-bit register is returned.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  Value     New value of the bit field.

  @return The value written back to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldWrite32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT32                    Value
  )
{
  return PciSegmentWrite32 (
           Address,
           BitFieldWrite32 (PciSegmentRead32 (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 32-bit PCI configuration, performs a bitwise OR, and
  writes the result back to the bit field in the 32-bit port.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 32-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized. Extra left bits in OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldOr32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT32                    OrData
  )
{
  return PciSegmentWrite32 (
           Address,
           BitFieldOr32 (PciSegmentRead32 (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 32-bit PCI configuration register, performs a bitwise
  AND, and writes the result back to the bit field in the 32-bit register.


  Reads the 32-bit PCI configuration register specified by Address, performs a bitwise
  AND between the read result and the value specified by AndData, and writes the result
  to the 32-bit PCI configuration register specified by Address. The value written to
  the PCI configuration register is returned.  This function must guarantee that all PCI
  read and write operations are serialized.  Extra left bits in AndData are stripped.
  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   Address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldAnd32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT32                    AndData
  )
{
  return PciSegmentWrite32 (
           Address,
           BitFieldAnd32 (PciSegmentRead32 (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 32-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  32-bit port.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 32-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldAndThenOr32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT32                    AndData,
  IN UINT32                    OrData
  )
{
  return PciSegmentWrite32 (
           Address,
           BitFieldAndThenOr32 (PciSegmentRead32 (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a range of PCI configuration registers into a caller supplied buffer.

  Reads the range of PCI configuration registers specified by StartAddress and
  Size into the buffer specified by Buffer. This function only allows the PCI
  configuration registers from a single PCI function to be read. Size is
  returned. When possible 32-bit PCI configuration read cycles are used to read
  from StartAdress to StartAddress + Size. Due to alignment restrictions, 8-bit
  and 16-bit PCI configuration read cycles may be used at the beginning and the
  end of the range.

  If any reserved bits in StartAddress are set, then ASSERT().
  If ((StartAddress & 0xFFF) + Size) > 0x1000, then ASSERT().
  If Size > 0 and Buffer is NULL, then ASSERT().

  @param  StartAddress  Starting address that encodes the PCI Segment, Bus, Device,
                        Function and Register.
  @param  Size          Size in bytes of the transfer.
  @param  Buffer        Pointer to a buffer receiving the data read.

  @return Size

**/
UINTN
EFIAPI
PciSegmentReadBuffer (
  IN  UINT64                   StartAddress,
  IN  UINTN                    Size,
  OUT VOID                     *Buffer
  )
{
  UINTN                                ReturnValue;

  ASSERT (((StartAddress & 0xFFF) + Size) <= 0x1000);

  if (Size == 0) {
    return Size;
  }

  ASSERT (Buffer != NULL);

  //
  // Save Size for return
  //
  ReturnValue = Size;

  if ((StartAddress & BIT0) != 0) {
    //
    // Read a byte if StartAddress is byte aligned
    //
    *(volatile UINT8 *)Buffer = PciSegmentRead8 (StartAddress);
    StartAddress += sizeof (UINT8);
    Size -= sizeof (UINT8);
    Buffer = (UINT8*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16) && (StartAddress & BIT1) != 0) {
    //
    // Read a word if StartAddress is word aligned
    //
    WriteUnaligned16 (Buffer, PciSegmentRead16 (StartAddress));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  while (Size >= sizeof (UINT32)) {
    //
    // Read as many double words as possible
    //
    WriteUnaligned32 (Buffer, PciSegmentRead32 (StartAddress));
    StartAddress += sizeof (UINT32);
    Size -= sizeof (UINT32);
    Buffer = (UINT32*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16)) {
    //
    // Read the last remaining word if exist
    //
    WriteUnaligned16 (Buffer, PciSegmentRead16 (StartAddress));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  if (Size >= sizeof (UINT8)) {
    //
    // Read the last remaining byte if exist
    //
    *(volatile UINT8 *)Buffer = PciSegmentRead8 (StartAddress);
  }

  return ReturnValue;
}

/**
  Copies the data in a caller supplied buffer to a specified range of PCI
  configuration space.

  Writes the range of PCI configuration registers specified by StartAddress and
  Size from the buffer specified by Buffer. This function only allows the PCI
  configuration registers from a single PCI function to be written. Size is
  returned. When possible 32-bit PCI configuration write cycles are used to
  write from StartAdress to StartAddress + Size. Due to alignment restrictions,
  8-bit and 16-bit PCI configuration write cycles may be used at the beginning
  and the end of the range.

  If any reserved bits in StartAddress are set, then ASSERT().
  If ((StartAddress & 0xFFF) + Size) > 0x1000, then ASSERT().
  If Size > 0 and Buffer is NULL, then ASSERT().

  @param  StartAddress  Starting address that encodes the PCI Segment, Bus, Device,
                        Function and Register.
  @param  Size          Size in bytes of the transfer.
  @param  Buffer        Pointer to a buffer containing the data to write.

  @return The parameter of Size.

**/
UINTN
EFIAPI
PciSegmentWriteBuffer (
  IN UINT64                    StartAddress,
  IN UINTN                     Size,
  IN VOID                      *Buffer
  )
{
  UINTN                                ReturnValue;

  ASSERT (((StartAddress & 0xFFF) + Size) <= 0x1000);

  if (Size == 0) {
    return 0;
  }

  ASSERT (Buffer != NULL);

  //
  // Save Size for return
  //
  ReturnValue = Size;

  if ((StartAddress & BIT0) != 0) {
    //
    // Write a byte if StartAddress is byte aligned
    //
    PciSegmentWrite8 (StartAddress, *(UINT8*) Buffer);
    StartAddress += sizeof (UINT8);
    Size -= sizeof (UINT8);
    Buffer = (UINT8*) Buffer + 1;
  }

  if (Size >= sizeof (UINT16) && (StartAddress & BIT1) != 0) {
    //
    // Write a word if StartAddress is word aligned
    //
    PciSegmentWrite16 (StartAddress, ReadUnaligned16 (Buffer));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*) Buffer + 1;
  }

  while (Size >= sizeof (UINT32)) {
    //
    // Write as many double words as possible
    //
    PciSegmentWrite32 (StartAddress, ReadUnaligned32 (Buffer));
    StartAddress += sizeof (UINT32);
    Size -= sizeof (UINT32);
    Buffer = (UINT32*) Buffer + 1;
  }

  if (Size >= sizeof (UINT16)) {
    //
    // Write the last remaining word if exist
    //
    PciSegmentWrite16 (StartAddress, ReadUnaligned16 (Buffer));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*) Buffer + 1;
  }

  if (Size >= sizeof (UINT8)) {
    //
    // Write the last remaining byte if exist
    //
    PciSegmentWrite8 (StartAddress, *(UINT8*) Buffer);
  }

  return ReturnValue;
}

REGISTER_PCI_DEVICE_STRUCT *
GetPciDevice (
  IN UINT64                    Address
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  UINTN                           Index;

  LibAddress.Data = Address;

  for (Index = 0; Index < mPciDevicesCount; Index++) {
    if ((LibAddress.Bits.Segment == mPciDevices[Index].Address.Bits.Segment) &&
        (LibAddress.Bits.Bus == mPciDevices[Index].Address.Bits.Bus) &&
        (LibAddress.Bits.Device == mPciDevices[Index].Address.Bits.Device) &&
        (LibAddress.Bits.Function == mPciDevices[Index].Address.Bits.Function) ) {
      return &mPciDevices[Index];
    }
  }

  return NULL;
}

PCI_DEVICE_BUFFER *
GetPciDeviceBuffer (
  IN UINT64                    Address
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  UINTN                           Index;

  LibAddress.Data = Address;
  
  for (Index = 0; Index < mPciDevicesCount; Index++) {
    if ((LibAddress.Bits.Segment == mPciDevices[Index].Address.Bits.Segment) &&
        (LibAddress.Bits.Bus == mPciDevices[Index].Address.Bits.Bus) &&
        (LibAddress.Bits.Device == mPciDevices[Index].Address.Bits.Device) &&
        (LibAddress.Bits.Function == mPciDevices[Index].Address.Bits.Function) ) {
      return &mPciDevicesBuffer[Index];
    }
  }

  return NULL;
}

VOID
ProgramPciDevice (
  IN REGISTER_PCI_DEVICE_STRUCT   *PciDevices,
  IN PCI_DEVICE_BUFFER            *PciDevicesBuffer
  )
{
  PCI_TYPE_GENERIC  *Pci;
  UINTN             Index;

  Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
  Pci->Device.Hdr.VendorId   = PciDevices->PciId.VendorId;
  Pci->Device.Hdr.DeviceId   = PciDevices->PciId.DeviceId;
  Pci->Device.Hdr.RevisionID = PciDevices->PciId.RevisionID;
  Pci->Device.Hdr.HeaderType = PciDevices->PciId.HeaderType;
  CopyMem (Pci->Device.Hdr.ClassCode, PciDevices->PciId.ClassCode, sizeof(Pci->Device.Hdr.ClassCode));

  Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
  Pci->Device.Hdr.VendorId   = PciDevices->PciId.VendorId;
  Pci->Device.Hdr.DeviceId   = PciDevices->PciId.DeviceId;
  Pci->Device.Hdr.RevisionID = PciDevices->PciId.RevisionID;
  Pci->Device.Hdr.HeaderType = PciDevices->PciId.HeaderType;
  CopyMem (Pci->Device.Hdr.ClassCode, PciDevices->PciId.ClassCode, sizeof(Pci->Device.Hdr.ClassCode));

  Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
  Pci->Device.Hdr.VendorId   = 0;
  Pci->Device.Hdr.DeviceId   = 0;
  Pci->Device.Hdr.RevisionID = 0;
  Pci->Device.Hdr.HeaderType = 0;
  ZeroMem (Pci->Device.Hdr.ClassCode, sizeof(Pci->Device.Hdr.ClassCode));

  Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
  if (IS_PCI_P2P ((PCI_TYPE00 *)Pci)) {
    for (Index = 0; Index < ARRAY_SIZE(PciDevices->PciConfig.BridgeConfig.Bar); Index++) {
      switch (PciDevices->PciConfig.BridgeConfig.Bar[Index].BarType) {
      case PCI_BAR_TYPE_32BIT_MEM:
      case PCI_BAR_TYPE_32BIT_PMEM:
      case PCI_BAR_TYPE_IO:
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
        Pci->Bridge.Bridge.Bar[Index] = PciDevices->PciConfig.BridgeConfig.Bar[Index].BarType;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
        Pci->Bridge.Bridge.Bar[Index] = PciDevices->PciConfig.BridgeConfig.Bar[Index].BarType;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
        Pci->Bridge.Bridge.Bar[Index] = (UINT32)~(PciDevices->PciConfig.BridgeConfig.Bar[Index].BarSize - 1);
        break;
      case PCI_BAR_TYPE_64BIT_MEM:
      case PCI_BAR_TYPE_64BIT_PMEM:
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
        *(UINT64 *)&Pci->Bridge.Bridge.Bar[Index] = PciDevices->PciConfig.BridgeConfig.Bar[Index].BarType;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
        *(UINT64 *)&Pci->Bridge.Bridge.Bar[Index] = PciDevices->PciConfig.BridgeConfig.Bar[Index].BarType;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
        *(UINT64 *)&Pci->Bridge.Bridge.Bar[Index] = (UINT64)~(PciDevices->PciConfig.BridgeConfig.Bar[Index].BarSize - 1);
        Index++;
        break;
      case PCI_BAR_TYPE_INVALID:
      default:
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
        Pci->Bridge.Bridge.Bar[Index] = 0;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
        Pci->Bridge.Bridge.Bar[Index] = 0;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
        Pci->Bridge.Bridge.Bar[Index] = 0;
        break;
      }
    }
    switch (PciDevices->PciConfig.BridgeConfig.IoBaseLimit.BaseLimitType) {
    case PCI_BASE_LIMIT_TYPE_16BIT_IO:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.IoBaseUpper16 = 0;
      Pci->Bridge.Bridge.IoLimitUpper16 = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.IoBaseUpper16 = 0;
      Pci->Bridge.Bridge.IoLimitUpper16 = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.IoBaseUpper16 = 0;
      Pci->Bridge.Bridge.IoLimitUpper16 = 0;
      // Passthru
    case PCI_BASE_LIMIT_TYPE_32BIT_IO:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.IoBase = PciDevices->PciConfig.BridgeConfig.IoBaseLimit.BaseLimitType;
      Pci->Bridge.Bridge.IoLimit = PciDevices->PciConfig.BridgeConfig.IoBaseLimit.BaseLimitType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.IoBase = PciDevices->PciConfig.BridgeConfig.IoBaseLimit.BaseLimitType;
      Pci->Bridge.Bridge.IoLimit = PciDevices->PciConfig.BridgeConfig.IoBaseLimit.BaseLimitType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.IoBase = (UINT8)~PCI_BASE_LIMIT_TYPE_IO_MASK;
      Pci->Bridge.Bridge.IoLimit = (UINT8)~PCI_BASE_LIMIT_TYPE_IO_MASK;
      break;
      break;
    case PCI_BASE_LIMIT_TYPE_INVALID:
    default:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.IoBase = 0;
      Pci->Bridge.Bridge.IoLimit = 0;
      Pci->Bridge.Bridge.IoBaseUpper16 = 0;
      Pci->Bridge.Bridge.IoLimitUpper16 = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.IoBase = 0;
      Pci->Bridge.Bridge.IoLimit = 0;
      Pci->Bridge.Bridge.IoBaseUpper16 = 0;
      Pci->Bridge.Bridge.IoLimitUpper16 = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.IoBase = 0;
      Pci->Bridge.Bridge.IoLimit = 0;
      Pci->Bridge.Bridge.IoBaseUpper16 = 0;
      Pci->Bridge.Bridge.IoLimitUpper16 = 0;
      break;
    }
    switch (PciDevices->PciConfig.BridgeConfig.MemBaseLimit.BaseLimitType) {
    case PCI_BASE_LIMIT_TYPE_MEM:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.MemoryBase = PciDevices->PciConfig.BridgeConfig.MemBaseLimit.BaseLimitType;
      Pci->Bridge.Bridge.MemoryLimit = PciDevices->PciConfig.BridgeConfig.MemBaseLimit.BaseLimitType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.MemoryBase = PciDevices->PciConfig.BridgeConfig.MemBaseLimit.BaseLimitType;
      Pci->Bridge.Bridge.MemoryLimit = PciDevices->PciConfig.BridgeConfig.MemBaseLimit.BaseLimitType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.MemoryBase = (UINT16)~PCI_BASE_LIMIT_TYPE_MEM_MASK;
      Pci->Bridge.Bridge.MemoryLimit = (UINT16)~PCI_BASE_LIMIT_TYPE_MEM_MASK;
      break;
    case PCI_BASE_LIMIT_TYPE_INVALID:
    default:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.MemoryBase = 0;
      Pci->Bridge.Bridge.MemoryLimit = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.MemoryBase = 0;
      Pci->Bridge.Bridge.MemoryLimit = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.MemoryBase = 0;
      Pci->Bridge.Bridge.MemoryLimit = 0;
      break;
    }
    switch (PciDevices->PciConfig.BridgeConfig.PMemBaseLimit.BaseLimitType) {
    case PCI_BASE_LIMIT_TYPE_32BIT_PMEM:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      // Passthru
    case PCI_BASE_LIMIT_TYPE_64BIT_PMEM:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.PrefetchableMemoryBase = PciDevices->PciConfig.BridgeConfig.PMemBaseLimit.BaseLimitType;
      Pci->Bridge.Bridge.PrefetchableMemoryLimit = PciDevices->PciConfig.BridgeConfig.PMemBaseLimit.BaseLimitType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.PrefetchableMemoryBase = PciDevices->PciConfig.BridgeConfig.PMemBaseLimit.BaseLimitType;
      Pci->Bridge.Bridge.PrefetchableMemoryLimit = PciDevices->PciConfig.BridgeConfig.PMemBaseLimit.BaseLimitType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.PrefetchableMemoryBase = (UINT16)~PCI_BASE_LIMIT_TYPE_MEM_MASK;
      Pci->Bridge.Bridge.PrefetchableMemoryLimit = (UINT16)~PCI_BASE_LIMIT_TYPE_MEM_MASK;
      break;
    case PCI_BASE_LIMIT_TYPE_INVALID:
    default:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.PrefetchableMemoryBase = 0;
      Pci->Bridge.Bridge.PrefetchableMemoryLimit = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.PrefetchableMemoryBase = 0;
      Pci->Bridge.Bridge.PrefetchableMemoryLimit = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.PrefetchableMemoryBase = 0;
      Pci->Bridge.Bridge.PrefetchableMemoryLimit = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      Pci->Bridge.Bridge.PrefetchableBaseUpper32 = 0;
      break;
    }
    switch (PciDevices->PciConfig.BridgeConfig.ExpansionRomBar.BarType) {
    case PCI_BAR_TYPE_32BIT_MEM:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.ExpansionRomBAR = PciDevices->PciConfig.BridgeConfig.ExpansionRomBar.BarType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.ExpansionRomBAR = PciDevices->PciConfig.BridgeConfig.ExpansionRomBar.BarType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.ExpansionRomBAR = (UINT32)~(PciDevices->PciConfig.BridgeConfig.ExpansionRomBar.BarSize - 1);
      break;
    case PCI_BAR_TYPE_INVALID:
    default:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Bridge.Bridge.ExpansionRomBAR = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Bridge.Bridge.ExpansionRomBAR = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Bridge.Bridge.ExpansionRomBAR = 0;
      break;
    }
  } else {
    for (Index = 0; Index < ARRAY_SIZE(PciDevices->PciConfig.DeviceConfig.Bar); Index++) {
      switch (PciDevices->PciConfig.DeviceConfig.Bar[Index].BarType) {
      case PCI_BAR_TYPE_32BIT_MEM:
      case PCI_BAR_TYPE_32BIT_PMEM:
      case PCI_BAR_TYPE_IO:
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
        Pci->Device.Device.Bar[Index] = PciDevices->PciConfig.DeviceConfig.Bar[Index].BarType;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
        Pci->Device.Device.Bar[Index] = PciDevices->PciConfig.DeviceConfig.Bar[Index].BarType;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
        if (PciDevices->PciConfig.DeviceConfig.Bar[Index].BarType != PCI_BAR_TYPE_IO) {
          Pci->Device.Device.Bar[Index] = (UINT32)~(PciDevices->PciConfig.DeviceConfig.Bar[Index].BarSize - 1);
        } else {
          Pci->Device.Device.Bar[Index] = (UINT32)~(PciDevices->PciConfig.DeviceConfig.Bar[Index].BarSize - 1);
        }
        break;
      case PCI_BAR_TYPE_64BIT_MEM:
      case PCI_BAR_TYPE_64BIT_PMEM:
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
        *(UINT64 *)&Pci->Device.Device.Bar[Index] = PciDevices->PciConfig.DeviceConfig.Bar[Index].BarType;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
        *(UINT64 *)&Pci->Device.Device.Bar[Index] = PciDevices->PciConfig.DeviceConfig.Bar[Index].BarType;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
        *(UINT64 *)&Pci->Device.Device.Bar[Index] = (UINT64)~(PciDevices->PciConfig.DeviceConfig.Bar[Index].BarSize - 1);
        Index++;
        break;
      case PCI_BAR_TYPE_INVALID:
      default:
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
        Pci->Device.Device.Bar[Index] = 0;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
        Pci->Device.Device.Bar[Index] = 0;
        Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
        Pci->Device.Device.Bar[Index] = 0;
        break;
      }
    }
    switch (PciDevices->PciConfig.DeviceConfig.ExpansionRomBar.BarType) {
    case PCI_BAR_TYPE_32BIT_MEM:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Device.Device.ExpansionRomBar = PciDevices->PciConfig.DeviceConfig.ExpansionRomBar.BarType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Device.Device.ExpansionRomBar = PciDevices->PciConfig.DeviceConfig.ExpansionRomBar.BarType;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Device.Device.ExpansionRomBar = (UINT32)~(PciDevices->PciConfig.DeviceConfig.ExpansionRomBar.BarSize - 1);
      break;
    case PCI_BAR_TYPE_INVALID:
    default:
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->Buffer;
      Pci->Device.Device.ExpansionRomBar = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->OrBuffer;
      Pci->Device.Device.ExpansionRomBar = 0;
      Pci = (PCI_TYPE_GENERIC *)PciDevicesBuffer->AndBuffer;
      Pci->Device.Device.ExpansionRomBar = 0;
      break;
    }
  }
}

BOOLEAN
IsPciDeviceUnderBridge (
  IN EFI_DEVICE_PATH        *PciDevice,
  IN EFI_DEVICE_PATH        *PciBridge
  )
{
  EFI_DEVICE_PATH *PciDeviceNode;
  EFI_DEVICE_PATH *PciBridgeNode;
  UINTN           Length;

  PciDeviceNode = PciDevice;
  PciBridgeNode = PciBridge;
  while (!IsDevicePathEnd (PciDeviceNode) && !IsDevicePathEnd (PciBridgeNode)) {
    Length = DevicePathNodeLength (PciDeviceNode);
    if ((Length != DevicePathNodeLength (PciBridgeNode)) ||
        (CompareMem (PciDeviceNode, PciBridgeNode, Length) != 0)) {
      break;
    }
    PciDeviceNode = NextDevicePathNode (PciDeviceNode);
    PciBridgeNode = NextDevicePathNode (PciBridgeNode);
  }
  if (IsDevicePathEnd (PciDeviceNode)) {
    return FALSE;
  }
  
  PciDeviceNode = NextDevicePathNode (PciDeviceNode);
  if (IsDevicePathEnd (PciDeviceNode) && IsDevicePathEnd (PciBridgeNode)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

VOID
MakePciDiscoverability (
  IN UINT64                    Address,
  IN UINT8                     SecondaryBus
  )
{
  REGISTER_PCI_DEVICE_STRUCT      *PciBridge;
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  UINTN                           Index;

  PciBridge = GetPciDevice (Address);
  ASSERT (PciBridge != NULL);

  LibAddress.Data = Address;
  
  for (Index = 0; Index < mPciDevicesCount; Index++) {
    if (IsPciDeviceUnderBridge (mPciDevices[Index].DevicePath, PciBridge->DevicePath)) {
      mPciDevices[Index].Address.Bits.Bus = SecondaryBus;
      if (SecondaryBus != 0) {
        mPciDevicesBuffer[Index].Discoverable = TRUE;
      } else {
        mPciDevicesBuffer[Index].Discoverable = FALSE;
      }
    }
  }
}

VOID
PciSegmentLibWriteHook (
  IN UINT64                    Address,
  IN UINT32                    Value,
  IN UINT8                     Size
  )
{
  PCI_SEGMENT_LIB_ADDRESS_STRUCT  LibAddress;
  PCI_DEVICE_BUFFER               *PciDeviceBuffer;
  PCI_TYPE_GENERIC                *Pci;

  LibAddress.Data = Address;
  PciDeviceBuffer = GetPciDeviceBuffer (Address);
  ASSERT (PciDeviceBuffer != NULL);

  Pci = (PCI_TYPE_GENERIC *)PciDeviceBuffer->Buffer;
  if (IS_PCI_P2P ((PCI_TYPE00 *)Pci)) {
    if ((LibAddress.Bits.Register <= PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET) &&
        (LibAddress.Bits.Register + Size > PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET)) {
      MakePciDiscoverability (Address, Pci->Bridge.Bridge.SecondaryBus);
    }
  }
}  

  
BOOLEAN
IsPciDeviceUnderRoot (
  IN EFI_DEVICE_PATH        *PciDevice
  )
{
  EFI_DEVICE_PATH *DevicePath;
  UINTN           PciNodeCount;

  DevicePath = PciDevice;
  PciNodeCount = 0;
  while (!IsDevicePathEnd (DevicePath)) {
    if ((DevicePathType (DevicePath) == HARDWARE_DEVICE_PATH) && (DevicePathSubType (DevicePath) == HW_PCI_DP)) {
      PciNodeCount ++;
    }
    DevicePath = NextDevicePathNode (DevicePath);
  }

  ASSERT(PciNodeCount != 0);
  if (PciNodeCount == 1) {
    return TRUE;
  } else {
    return FALSE;
  }
}

VOID
EFIAPI
RegisterPciDevices (
  IN UINTN                        PciDevicesCount,
  IN REGISTER_PCI_DEVICE_STRUCT   *PciDevices
  )
{
  UINTN  Index;
  VOID   *DataBuffer;
  UINTN  BufferSize;

  mPciDevicesCount = PciDevicesCount;
  mPciDevices = PciDevices;
  for (Index = 0; Index < mPciDevicesCount; Index++) {
    ASSERT (mPciDevices[Index].Address.Bits.Bus == 0);
    ASSERT (mPciDevices[Index].Address.Bits.Register == 0);
    ASSERT (mPciDevices[Index].Address.Bits.Reserved == 0);
  }

  BufferSize = 0x40;

  mPciDevicesBuffer = AllocateZeroPool (sizeof(PCI_DEVICE_BUFFER) * PciDevicesCount);
  DataBuffer = AllocateZeroPool (BufferSize * PciDevicesCount * 3);
  for (Index = 0; Index < mPciDevicesCount; Index++) {
    if (IsPciDeviceUnderRoot (mPciDevices[Index].DevicePath)) {
      mPciDevicesBuffer[Index].Discoverable = TRUE;
    } else {
      mPciDevicesBuffer[Index].Discoverable = FALSE;
    }

    mPciDevicesBuffer[Index].BufferSize = BufferSize;

    mPciDevicesBuffer[Index].Buffer     = (VOID *)((UINTN)DataBuffer + BufferSize * 0 + (BufferSize * 3 * Index));
    mPciDevicesBuffer[Index].OrBuffer   = (VOID *)((UINTN)DataBuffer + BufferSize * 1 + (BufferSize * 3 * Index));
    mPciDevicesBuffer[Index].AndBuffer  = (VOID *)((UINTN)DataBuffer + BufferSize * 2 + (BufferSize * 3 * Index));
    SetMem (mPciDevicesBuffer[Index].AndBuffer, BufferSize, 0xFF);

    ProgramPciDevice (&mPciDevices[Index], &mPciDevicesBuffer[Index]);
  }
}