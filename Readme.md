# Boot Guard TOCTOU Vulnerability Mitigation Staging

This branch contains code to mitigate CVE-2019-11098. The changes are
maintained in edk2-staging because the changes are not ready for
product integration and as such are subject to modification due to
design changes and community feedback.

Staging branch contacts:
* Michael Kubacki \<michael.a.kubacki@intel.com\>
* Jian J Wang \<jian.j.wang@intel.com\>

# Background
Intel(r) Boot Guard is a technology which establishes a strong
component based Static Root of Trust for verification and measurement
(S-CRTV/S-CRTM). Protection is provided against malicious modification
of the Initial Boot Block (IBB). A Boot Guard TOCTOU vulnerability in
EDK II firmware was discovered that allows an attacker with physical
access to achieve code execution after the Boot Guard ACM computes and
validates the hash of the IBB and has extended firmware measurements
into the TPM PCR0. This means the firmware will be marked as valid and
have normal PCR0 values even though unsigned code has run. By using
targeted SPI transactions to modify IBB code after IBB is verified,
that code may later be fetched from SPI flash and executed after
Non-Eviction Mode (NEM) is disabled after main memory is initialized.
This attack requires physical access to the SPI flash.

In order to mitigate this vulnerability, global pointers to code and
data to Cache As RAM (CAR) addresses must not exist after NEM is
disabled as addresses in the CAR region map 1:1 to SPI flash MMIO.
Those resources must be moved from CAR to permanent memory and the
global pointer address correspondingly updated. In this set of
changes the process of moving resources from CAR to permanent memory
is referred to as "migration".

# Platform Guidance for the Mitigation Changes
These changes are intended to simply integrate into existing firmware
solutions. For the changes to function as intended, the platform
should follow these guidelines.

1. Always ensure PcdShadowPeimOnBoot and PcdShadowPeimOnS3Boot
   (if platform supports S3) are set if Boot Guard is enabled and
   V=1 or M=1.
2. Ensure that all PEIMs are relocatable. Relocation tables should
   not be stripped.
3. If an Intel(r) Firmware Support Package (FSP) binary solution is
   used, the binary must have these mitigation changes integrated.
4. Avoid maintaing pointers to pre-memory addresses inside embedded
   structures or other non-standard structures that the automatic
   migration code introduced here cannot identify.
5. Migrate the FIT table based on platform requirements for FIT
   access in post-memory.
6. Enable paging after memory initialization and mark any IBB
   verified address ranges as Not Present.
   (example in CpuPaging.c)

# High-Level Migration Required
Resources that must be migrated can be categorized as code or data.

## Code Migration
Global pointers to code are typically registered in pre-memory to
interfaces available later in the boot such as PPIs or callbacks
that will be invoked upon some future event (PEI notification).

The following code pointers are addressed in the current mitigation
posting:
* PEIM-to-PEIM Interfaces (PPIs)
* PEI notifications
* Status code callback handlers

Modules that produce PPIs can register for shadow using the PEI
services which make the PEI core call into the module's entry point
after permanent memory is initialized. Once relocated in permanent
memory, the module can use the ReinstallPpi ( ) API defined in the
Platform Initialization (PI) specification to reinstall the PPI
with a GUID and interface structure in permanent memory. In practice,
this is tedious and error prone so a generic PPI migration solution
is introduced in this mitigation to attempt to automatically migrate
all PPIs discovered in the system during PEI core shadow. A similar
reinstall user interface does not exist for PEI notifications,
however those can be migrated in PEI core as well.

If the PPI descriptor address is not within a PEIM that is being
migrated this solution will not automatically migrate that PPI
in the present implementation. In this case, the module that installs
the PPI should register for shadow and reinstall the PPI in the
post-memory shadow entry of the module.

In summary, PPIs, PEI notifications, and status code callback
handlers will be migrated by the PEI core in this mitigation solution.

## Data Migration
Global data pointers are inherent in the system architecture in
addition to firmware implementation.

Pointers to the following data structures are addressed in the current
mitigation posting:
* Interrupt Descriptor Table (IDT)
   * Migrated in CpuMpPei
* Global Descriptor Table (GDT)
   * Migrated in CpuMpPei
* PEI services table pointer
   * Migrated in PeiCore
* Temporary RAM (T-RAM) heap and stack
   * Migrated in PeiCore (or TemporaryRamMigration ( ))
* PPI list pointer
   * Migrated in PeiCore
* Hand Off Block (HOB) list pointer
   * Migrated in PeiCore
* Firmware Interface Table (FIT)
   * Migrated in PeiFirmwareInterfaceTableLib
* Memory allocation HOB entries
   * Migrated in PeiCore
* Firmware Volume (FV) HOB entries
   * Migrated in PeiCore

# Verifying Migration
Several mechanisms are introduced to simplify checking the migration
of code and data resources.

## Catching Invalid Accesses
In order to catch an access to a verified IBB range after NEM is
disabled, a page table may be set up in post-memory PEI that marks
the IBB range as Not Present. This causes a page fault which allows
the user to quickly identify the address that was accessed and caused
the issue by inspecting the value in the CR2 register.

In the example exception handler print message, code execution was
attempted at the invalid address 0xFFF75630. An address map may be
used to map that address to a specific function.
```
!!!! IA32 Exception Type - 0E(#PF - Page-Fault)  CPU Apic ID - 00000000 !!!!
ExceptionData - 00000000  I:0 R:0 U:0 W:0 P:0 PK:0 SS:0 SGX:0
EIP  - FFF75630, CS  - 00000010, EFLAGS - 00010046
EAX  - ACA54C9C, ECX - 3BC1F6DE, EDX - ACA54CA0, EBX - 00000000
ESP  - ACA4F930, EBP - ACA4FA14, ESI - ACA4FA10, EDI - 00000040
DS   - 00000018, ES  - 00000018, FS  - 00000018, GS  - 00000018, SS - 00000018
CR0  - 80000013, CR2 - FFF75630, CR3 - BDFF3000, CR4 - 00000620
DR0  - 00000000, DR1 - 00000000, DR2 - 00000000, DR3 - 00000000
DR6  - FFFF0FF0, DR7 - 00000400
GDTR - ACA5B4B0 0000003F, IDTR - BE1FF004 0000010F
LDTR - 00000000, TR - 00000000
FXSAVE_STATE - ACA4F670
```

## Debug Print Messages
In a DEBUG build, messages will be printed to the DEBUG output that
show before and after migration details. Some examples of those output
are shown below to aid in finding the messages in DEBUG output.

Many of the debug print messages are defined as DEBUG_VERBOSE. To see
these messages, set BIT22 ("Detailed debug message")  in the PCDs
PcdFixedDebugPrintErrorLevel / PcdDebugPrintErrorLevel. This should
specifically be set for PeiCore, CpuMpPei, and the module that
FirmwareInterfaceTableLib is linked against (if used).

### PPI and PEI Notification Migration Messages
1. PEI Callback Notifications
```
Lists before EvacuateTempRam ( ):
CallbackNotify[ 0] {49EDB1C1-BF21-4761-BB12-EB0031AABB39} at 0xFFECCFE4 (CAR)
CallbackNotify[ 1] {EA7CA24B-DED5-4DAD-A389-BF827E8F9B38} at 0xFFECCFF0 (CAR)
CallbackNotify[ 2] {0ECC666B-4662-47F9-9DD5-D096FF7DA49E} at 0x869D5000 (Post-Memory)
CallbackNotify[ 3] {605EA650-C65C-42E1-BA80-91A52AB618C6} at 0xFFED1150 (CAR)
CallbackNotify[ 4] {F894643D-C449-42D1-8EA8-85BDD8C65BDE} at 0xFFEE72A4 (CAR)
.   .   .
Lists after EvacuateTempRam ( ):
CallbackNotify[ 0] {49EDB1C1-BF21-4761-BB12-EB0031AABB39} at 0x8A1B2EC4 (Post-Memory)
CallbackNotify[ 1] {EA7CA24B-DED5-4DAD-A389-BF827E8F9B38} at 0x8A1B2ED0 (Post-Memory)
CallbackNotify[ 2] {0ECC666B-4662-47F9-9DD5-D096FF7DA49E} at 0x869D5000 (Post-Memory)
CallbackNotify[ 3] {605EA650-C65C-42E1-BA80-91A52AB618C6} at 0x8A077150 (Post-Memory)
CallbackNotify[ 4] {F894643D-C449-42D1-8EA8-85BDD8C65BDE} at 0x8A08D2A4 (Post-Memory)
.   .   .
```
2. PEI Notify Notifications
```
Lists before EvacuateTempRam ( ):
DispatchNotify[ 0] {DCD0BE23-9586-40F4-B643-06522CED4EDE} at 0xFFECD038 (CAR)
.   .   .
Lists after EvacuateTempRam ( ):
DispatchNotify[ 0] {DCD0BE23-9586-40F4-B643-06522CED4EDE} at 0x8A1B2F18 (Post-Memory)
.   .   .
```
3. PPIs
```
Lists before EvacuateTempRam ( ):
PPI[ 0] {8C8CE578-8A3D-4F1C-9935-896185C32DD3} at 0x8A1B2F0C (Post-Memory)
PPI[ 1] {5473C07A-3DCB-4DCA-BD6F-1E9689E7349A} at 0x8A1B2E88 (Post-Memory)
PPI[ 2] {B9E0ABFE-5979-4914-977F-6DEE78C278A6} at 0x8A1B2F88 (Post-Memory)
PPI[ 3] {CEAB683C-EC56-4A2D-A906-4053FA4E9C16} at 0x869D500C (Post-Memory)
PPI[ 4] {6F8C2B35-FEF4-448D-8256-E11B19D61077} at 0x869D5018 (Post-Memory)
PPI[ 5] {2F3962B2-57C5-44EC-9EFC-A69FD302032B} at 0x869D5024 (Post-Memory)
PPI[ 6] {0ECC666B-4662-47F9-9DD5-D096FF7DA49E} at 0x869D5030 (Post-Memory)
PPI[ 7] {06E81C58-4AD7-44BC-8390-F10265F72480} at 0xFFED115C (CAR)
PPI[ 8] {01F34D25-4DE2-23AD-3FF3-36353FF323F1} at 0xFFED1168 (CAR)
PPI[ 9] {4D8B155B-C059-4C8F-8926-06FD4331DB8A} at 0xFFED1138 (CAR)
PPI[10] {A60C6B59-E459-425D-9C69-0BCC9CB27D81} at 0xFFED1144 (CAR)
.   .   .
Lists after EvacuateTempRam ( ):
PPI[ 0] {8C8CE578-8A3D-4F1C-9935-896185C32DD3} at 0x8A1B2F0C (Post-Memory)
PPI[ 1] {5473C07A-3DCB-4DCA-BD6F-1E9689E7349A} at 0x8A1B2E88 (Post-Memory)
PPI[ 2] {B9E0ABFE-5979-4914-977F-6DEE78C278A6} at 0x8A1B2F88 (Post-Memory)
PPI[ 3] {CEAB683C-EC56-4A2D-A906-4053FA4E9C16} at 0x869D500C (Post-Memory)
PPI[ 4] {6F8C2B35-FEF4-448D-8256-E11B19D61077} at 0x869D5018 (Post-Memory)
PPI[ 5] {2F3962B2-57C5-44EC-9EFC-A69FD302032B} at 0x869D5024 (Post-Memory)
PPI[ 6] {0ECC666B-4662-47F9-9DD5-D096FF7DA49E} at 0x869D5030 (Post-Memory)
PPI[ 7] {06E81C58-4AD7-44BC-8390-F10265F72480} at 0x8A07715C (Post-Memory)
PPI[ 8] {01F34D25-4DE2-23AD-3FF3-36353FF323F1} at 0x8A077168 (Post-Memory)
PPI[ 9] {4D8B155B-C059-4C8F-8926-06FD4331DB8A} at 0x8A077138 (Post-Memory)
PPI[10] {A60C6B59-E459-425D-9C69-0BCC9CB27D81} at 0x8A077144 (Post-Memory)
.   .   .
```
### FV and FV HOB Migration Messages
4. FV and PEIM Migration
```
FV[01] at 0xFF8A0000.
  Migrating FV[1] from 0xFF8A0000 to 0x89FF6000
  FV buffer range from 0x89FF6000 to 0x8A066000
    Child FV[02] is being migrated.
    Child FV offset = 0x180.
    Child migrated FV header at 0x89FF6180.
    Migrating FileHandle  0 PlatformVTdInfoSamplePei
    Migrating FileHandle  1 IntelVTdPmrPei
    .   .   .
```
5. FV HOB Migration
All FV HOB Base Addresses (BA) should be in permanent memory ranges.
```
  Found FV HOB.
     BA=0000000089FF6180  L=0000000000010000
     BA=0000000089FF6180  L=0000000000010000
     BA=0000000089FF6180  L=0000000000010000
     BA=0000000089F960A0  L=0000000000000078
  .   .   .
```
### GDT, IDT, and FIT Migration Messages
6. GDT Migration
Address 0xFFFFDB10 is in the CAR address range.
```
Dumping GDT (before migration):
GDT at 0xFFFFDB10. Entries: 8. Size: 0x40

  Entry[0000]
    Base = 0x0
    Limit  = 0x0
    Access Bytes:
      Type: 0x0
        Accessed             : 0x0
        RW                   : 0x0
        Direction/Conforming : 0x0
        Executable           : 0x0
      Descriptor Type (S)    : 0x0
      Privilege (DPL)        : 0x0
      Present (P)            : 0x0
      Flags:
        AVL                  : 0x0
        L                    : 0x0
        DB                   : 0x0
        G                    : 0x0
  Entry[0001]
    Base = 0x0
    Limit  = 0xFFFFF
    Access Bytes:
      Type: 0x2
        Accessed             : 0x0
        RW                   : 0x1
        Direction/Conforming : 0x0
        Executable           : 0x0
      Descriptor Type (S)    : 0x1
      Privilege (DPL)        : 0x0
      Present (P)            : 0x1
      Flags:
        AVL                  : 0x0
        L                    : 0x0
        DB                   : 0x1
        G                    : 0x1
.   .   .
```
After migration, GDT is at 0x869DBAF0, an address in DRAM.
```
Dumping GDT (after migration):
GDT at 0x869DBAF0. Entries: 8. Size: 0x40

  Entry[0000]
    Base = 0x0
    Limit  = 0x0
    Access Bytes:
      Type: 0x0
        Accessed             : 0x0
        RW                   : 0x0
        Direction/Conforming : 0x0
        Executable           : 0x0
      Descriptor Type (S)    : 0x0
      Privilege (DPL)        : 0x0
      Present (P)            : 0x0
      Flags:
        AVL                  : 0x0
        L                    : 0x0
        DB                   : 0x0
        G                    : 0x0
  Entry[0001]
    Base = 0x0
    Limit  = 0xFFFFF
    Access Bytes:
      Type: 0x2
        Accessed             : 0x0
        RW                   : 0x1
        Direction/Conforming : 0x0
        Executable           : 0x0
      Descriptor Type (S)    : 0x1
      Privilege (DPL)        : 0x0
      Present (P)            : 0x1
      Flags:
        AVL                  : 0x0
        L                    : 0x0
        DB                   : 0x1
        G                    : 0x1
.   .   .
```
7. IDT Migration
Address 0x8A1B4004 is already in permanent memory. However, the
interrupt handler entries are in CAR address ranges.
```
Dumping IDT:
IDT at 0x8A1B4004. Entries: 34. Size: 0x110

  Entry[000]
    Offset      = 0xFFFF9890
    Selector    = 0x10
    Gate Type   = Interrupt (32-bit) (0x8E)
  Entry[001]
    Offset      = 0xFFFF989A
    Selector    = 0x10
    Gate Type   = Interrupt (32-bit) (0x8E)
  Entry[002]
    Offset      = 0xFFFF98A4
    Selector    = 0x10
    Gate Type   = Interrupt (32-bit) (0x8E)
```
After migration, all entries are now in DRAM.
```
Dumping IDT:
IDT at 0x8A1B4004. Entries: 34. Size: 0x110

  Entry[000]
    Offset      = 0x869DC250
    Selector    = 0x10
    Gate Type   = Interrupt (32-bit) (0x8E)
  Entry[001]
    Offset      = 0x89A1B6AA
    Selector    = 0x10
    Gate Type   = Interrupt (32-bit) (0x8E)
  Entry[002]
    Offset      = 0x89A1B6B4
    Selector    = 0x10
    Gate Type   = Interrupt (32-bit) (0x8E)
```
8. FIT Migration
If FIT is present, PeiFirmwareInterfaceTableLib can be used
to move the table to permanent memory. The API in
FirmwareInterfaceTableLib.h should be used to get the FIT
address instead of the traditional pointer location in
0xFFFFFFC0.

As in the other cases, this is example output only. FIT may not
be present and results may vary.
```
[0] - Found FIT entry of type 0.
[1] - Found but ignoring FIT entry of type 1.
[2] - Found FIT entry of type 2.
[3] - Found FIT entry of type 10.
[4] - Found FIT entry of type 11.
  Found FIT object. Size 0x353. Address 0xFFCBFBC0.
[5] - Found FIT entry of type 12.
  Found FIT object. Size 0x51A. Address 0xFFCBF680.
[6] - Found FIT entry of type 16.
[7] - Found FIT entry of type 16.
[8] - Found FIT entry of type 16.
[9] - Found FIT entry of type 16.
[10] - Found FIT entry of type 16.
Source FIT entries = 0xB.
Destination FIT entries = 0xA.
Destination FIT size = 0x940
Copying FIT entry of type 0 from 0xFFCBFF20 to 0x8C92F930.
Copying FIT entry of type 2 from 0xFFCBFF40 to 0x8C92F940.
Copying FIT entry of type 10 from 0xFFCBFF50 to 0x8C92F950.
Copying FIT entry of type 11 from 0xFFCBFF60 to 0x8C92F960.
Copying FIT object for entry of type 11 of size 0x353 from 0xFFCBFBC0 to 0x8C92F9E0.
Copying FIT entry of type 12 from 0xFFCBFF70 to 0x8C92F970.
Copying FIT object for entry of type 12 of size 0x51A from 0xFFCBF680 to 0x8C92FD40.
Copying FIT entry of type 16 from 0xFFCBFF80 to 0x8C92F980.
Copying FIT entry of type 16 from 0xFFCBFF90 to 0x8C92F990.
Copying FIT entry of type 16 from 0xFFCBFFA0 to 0x8C92F9A0.
Copying FIT entry of type 16 from 0xFFCBFFB0 to 0x8C92F9B0.
Copying FIT entry of type 16 from 0xFFCBFFC0 to 0x8C92F9C0.
```
