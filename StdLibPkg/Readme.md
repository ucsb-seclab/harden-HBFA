# StdLibPkg

## Package description
This branch is used for community development of package containing C standard library and compiler-specific intrinsics.

Enlisted functionality is covered by two libraries
* CStdLib - C standard library built around [pdclib](https://github.com/DevSolar/pdclib) submodule.
* IntrinsicLib - placeholder for all intrinsic functions added by compilers

## What's done?
Currently IA32 and X64 builds of StdLibPkg were verified.

### CStdLib
Functions covered:
* Unsafe functions
* Functions not tied to environment
* Limited file operations on standard I/O. printf/scanf

Other symbols that have been used are currently covered with ASSERT (FALSE) with intent to be later implemented.

### IntrinsicLib
Code files are provided only for MSVC. Most of the intrinsics are implemented as immediate return with intent to be later implemented. Some were taken directly from CryptoPkg.

### Unit tests
pdclib code contains unit tests for various C standard library functions (each as main() symbol). Package contains:
* python script to replace main() symbols with distinct ones
* C standard library built with unit tests
* UEFI application meant to be run under OVMF based on UnitTestFrameworkPkg to execute unit tests.

## Layout
```
> Library                           --- Libraries
    > CStdLib                       --- C standard library
        > DevApp                    --- application to play around with StdLibPkg libraries
        > pdclib                    --- submodule
        > Platform/uefi             --- UEFI-specific pieces used by pdclib
        > Scripts                   --- scripts required by this package
        > UnitTests                 --- source files for unit test applications
        > DxeCStdLib.inf            --- CStdLib INF for DXE stage
        > DxeCStdTestLib.inf        --- CStdLib INF for unit tests in DXE stage
        > TestDxeCStdLibOvmf.inf    --- INF for unit test application to be used under OVMF
    > IntrinsicLib                  --- intrinsics library
> StdLibPkg.dec                     --- include paths, guids, etc.
> StdLibPkg.dsc                     --- standalone build for DXE
> StdLibPkgOvmfUnitTest.dsc         --- standalone build for unit test app for OVMF
```

## Usage
* Add CStdLib and IntrinsicLib paths to target .dsc.
* Add CStdLib and IntrinsicLib to LibraryClasses section in .inf file of target driver/application.
* Add StdLibPkg/StdLibPkg.dec to Packages section in .inf file of target driver/application.
* Compile and run target driver/application.

## Next steps
* ARM/AARCH64/RISCV support (most likely by simply annotating support in .dsc files).
* Align with latest pdclib (changes around string formatting).
* Implementation of functions in IntrinsicLib.
* Socket implementation (own or port from edk2-libc)