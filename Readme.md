# Multiple Controllers Support

This branch is used to evaluate methods to update the FmpDevicePkg to support
drivers that manage multiple controllers and also provide a firmware update
capability to each managed controller. The primary use case is the update of
a PCI Option ROM when multiple adapters are present.

The branch also makes enhancement to the ESRT when multiple same controllers
exist in one system.

This branch addresses the following TianoCore Bugzilla issues:
https://bugzilla.tianocore.org/show_bug.cgi?id=1525

Branch owner(s):
Michael D Kinney <michael.d.kinney@intel.com>

## Related Modules
  The following modules are related to **Multiple Controllers Support** solution
  * **FmpDevicePkg\FmpDxe\FmpDxe.inf** - Driver to manage multiple controllers
  and provide the firmware update capability to each managed controller.
  * **FmpDevicePkg\CapsuleUpdatePolicyDxe\CapsuleUpdatePolicyDxe.inf** - Driver
  to produce the Capsule Update Policy Protocol using the services of the
  CapsuleUpdatePolicyLib class. The protocol is a private interface to the
  FmpDevicePkg
  * **FmpDevicePkg\Library\CapsuleUpdatePolicyLibOnProtocol\CapsuleUpdatePolicyLibOnProtocol.inf** -
  CapsuleUpdatePolicyLib instance that uses the services of the Capsule Update
  Policy Protocol produced by CapsuleUpdatePolicyDxe
  * **FmpDevicePkg\Library\CapsuleUpdatePolicyLibNull\CapsuleUpdatePolicyLibNull.inf** -
  Null CapsuleUpdatePolicyLib instance and the template for platform specific implementation
  * **FmpDevicePkg\Library\FmpDeviceLibNull\FmpDeviceLibNull.inf** - Null
  FmpDeviceLib instance and the template for platform specific implementation
  * **MdeModulePkg\Universal\EsrtFmpDxe\EsrtFmpDxe.inf** - Driver to produce
  ESRT entries based only on FMP Protocol instances

## Major Enhancements
  To extend UEFI Capsule-based firmware updates from system firmware and
  integrated devices to multiple plugin devices, the main enhancements are:
  * Add 3 APIs to FmpDeviceLib
    * RegisterFmpUninstaller()
    * FmpDeviceSetContext()
    * FmpDeviceGetHardwareInstance()
  * Distinguish UEFI variable of each device with hardware instance value
  * Convert 4 separate UEFI variables to the single merged variable for efficient
  use of UEFI variable store
  * To overcome the limited PCI option ROM size, introduce the PCD
  PcdFmpDeviceStorageAccessEnable to build the min or full version of FmpDxe from
  same source
  * Enhance DEBUG message with unique mImageIdName from PcdFmpDeviceImageIdName
  * Detect the error that multiple FMP instances have the same pair of GUID and
  HardwareInstance
  * Merge multiple FMP instances together into a single ESRT entry if they have
  same GUID
    * **Version** is lowest value among set with same GUID
    * **LowestSupportedVersion** is lowest value among set with same GUID
    * **LastAttemptStatus** is the status of the first FMP info structure with
    the same GUID that has an error status. If none of the FMP info structures
    for the same GUID has an error status, then the status is success
    * **LastAttemptVersion** is the version value associated with the FMP info
    instance used to fill in the **LastAttemptStatus** value

## Validation and Multiple Controllers Examples
  OvmfPkg is leveraged to give multiple controllers example for validation usage.
  OVMF changes in this staging branch are only to test the new features and will
  not be integrated into edk2/master.
  * Multiple Controller Examples:
    * [OvmfPkg E1000 Example](OvmfPkg/README_CAPSULES.md)
  * OvmfPkg Changes:
    * Update Varstore.fdf.inc to support variable in PEI phase
    * Add capsule support in PlatformPei module
    * Add logic to process UEFI Capsules before and after the system lock point
    in PlatformBootManagerLib
    * Add E1000 FMP driver and FmpDeviceLib instance
    * Add Sample device FmpDeviceLib instance

## Promote to EDKII Trunk
  If a subset feature or a bug fix in this staging branch could meet below
  requirement, it could be promoted to EDKII trunk and removed from this staging
  branch:
  * Meet all EDKII required quality criteria
  * Ready for product integration

## Timeline
  | Time | Event | Related Modules |
  |:----:|:-----:|:---------------:|
  |2019.01| Initial the feature development. | Refer to "Related Modules" |
  |...|...|...|

# EDK II Project

A modern, feature-rich, cross-platform firmware development environment
for the UEFI and PI specifications from www.uefi.org.

The majority of the content in the EDK II open source project uses a
[BSD-2-Clause Plus Patent License](License.txt).  The EDK II open source project
contains the following components that are covered by additional licenses:
* [BaseTools/Source/C/BrotliCompress](BaseTools/Source/C/BrotliCompress/LICENSE)
* [MdeModulePkg/Library/BrotliCustomDecompressLib](MdeModulePkg/Library/BrotliCustomDecompressLib/LICENSE)
* [BaseTools/Source/C/LzmaCompress](BaseTools/Source/C/LzmaCompress/LZMA-SDK-README.txt)
* [MdeModulePkg/Library/LzmaCustomDecompressLib](MdeModulePkg/Library/LzmaCustomDecompressLib/LZMA-SDK-README.txt)
* [IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/Sdk](IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LZMA-SDK-README.txt)
* [BaseTools/Source/C/VfrCompile/Pccts](BaseTools/Source/C/VfrCompile/Pccts/RIGHTS)
* [EdkCompatibilityPkg/Other/Maintained/Tools/Pccts](EdkCompatibilityPkg/Other/Maintained/Tools/Pccts/README)
* [MdeModulePkg/Universal/RegularExpressionDxe/Oniguruma](MdeModulePkg/Universal/RegularExpressionDxe/Oniguruma/README)
* [OvmfPkg](OvmfPkg/License.txt)
* [CryptoPkg/Library/OpensslLib/openssl](CryptoPkg/Library/OpensslLib/openssl/LICENSE)

The EDK II Project is composed of packages.  The maintainers for each package
are listed in [Maintainers.txt](Maintainers.txt).

# Resources
* [TianoCore](http://www.tianocore.org)
* [EDK II](https://github.com/tianocore/tianocore.github.io/wiki/EDK-II)
* [Getting Started with EDK II](https://github.com/tianocore/tianocore.github.io/wiki/Getting-Started-with-EDK-II)
* [Mailing Lists](https://github.com/tianocore/tianocore.github.io/wiki/Mailing-Lists)
* [TianoCore Bugzilla](https://bugzilla.tianocore.org)
* [How To Contribute](https://github.com/tianocore/tianocore.github.io/wiki/How-To-Contribute)
* [Release Planning](https://github.com/tianocore/tianocore.github.io/wiki/EDK-II-Release-Planning)
* [UDK2017](https://github.com/tianocore/edk2/releases/tag/vUDK2017)
* [UDK2018](https://github.com/tianocore/edk2/releases/tag/vUDK2018)
* [edk2-stable201811](https://github.com/tianocore/edk2/releases/tag/edk2-stable201811)

# Code Contributions
To make a contribution to a TianoCore project, follow these steps.
1. Create a change description in the format specified below to
   use in the source control commit log.
2. Your commit message must include your `Signed-off-by` signature
3. Submit your code to the TianoCore project using the process
   that the project documents on its web page.  If the process is
   not documented, then submit the code on development email list
   for the project.
4. It is preferred that contributions are submitted using the same
   copyright license as the base project. When that is not possible,
   then contributions using the following licenses can be accepted:
   * BSD (2-clause): http://opensource.org/licenses/BSD-2-Clause
   * BSD (3-clause): http://opensource.org/licenses/BSD-3-Clause
   * MIT: http://opensource.org/licenses/MIT
   * Python-2.0: http://opensource.org/licenses/Python-2.0
   * Zlib: http://opensource.org/licenses/Zlib

   For documentation:
   * FreeBSD Documentation License
     https://www.freebsd.org/copyright/freebsd-doc-license.html

   Contributions of code put into the public domain can also be
   accepted.

   Contributions using other licenses might be accepted, but further
   review will be required.

# Developer Certificate of Origin

Your change description should use the standard format for a
commit message, and must include your `Signed-off-by` signature.

In order to keep track of who did what, all patches contributed must
include a statement that to the best of the contributor's knowledge
they have the right to contribute it under the specified license.

The test for this is as specified in the [Developer's Certificate of
Origin (DCO) 1.1](https://developercertificate.org/). The contributor
certifies compliance by adding a line saying

  Signed-off-by: Developer Name <developer@example.org>

where `Developer Name` is the contributor's real name, and the email
address is one the developer is reachable through at the time of
contributing.

```
Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

# Sample Change Description / Commit Message

```
From: Contributor Name <contributor@example.com>
Subject: [Repository/Branch PATCH] Pkg-Module: Brief-single-line-summary

Full-commit-message

Signed-off-by: Contributor Name <contributor@example.com>
```

## Notes for sample patch email

* The first line of commit message is taken from the email's subject
  line following `[Repository/Branch PATCH]`. The remaining portion of the
  commit message is the email's content.
* `git format-patch` is one way to create this format

## Definitions for sample patch email

* `Repository` is the identifier of the repository the patch applies.
  This identifier should only be provided for repositories other than
  `edk2`. For example `edk2-BuildSpecification` or `staging`.
* `Branch` is the identifier of the branch the patch applies. This
  identifier should only be provided for branches other than `edk2/master`.
  For example `edk2/UDK2015`, `edk2-BuildSpecification/release/1.27`, or
  `staging/edk2-test`.
* `Module` is a short identifier for the affected code or documentation. For
  example `MdePkg`, `MdeModulePkg/UsbBusDxe`, `Introduction`, or
  `EDK II INF File Format`.
* `Brief-single-line-summary` is a short summary of the change.
* The entire first line should be less than ~70 characters.
* `Full-commit-message` a verbose multiple line comment describing
  the change.  Each line should be less than ~70 characters.
* `Signed-off-by` is the contributor's signature identifying them
  by their real/legal name and their email address.
