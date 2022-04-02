# Python BaseTools
## Overview
This branch is for porting tools currently implemented in C to Python. This removes the need to run a C compiler to build the tools required to build.

The branch owner: Bob Feng <bob.c.feng@intel.com>

## Feature introduction
The following tolls will port to Python implementation:
- GenCrc32 (Easiest)
- Compress tools
  - LzmaCompress
  - BrotliCompress
  - PyEfiCompressor
  - TianoCompress
- PI Spec Vol3
  - GenSec
  - GenFfs
  - GenFv
- Utilities
  - VolInfo
  - EfiRom
  - DevicePath
- PE/COFF spec
  - GenFw
- VFR Spec
  - VFRCompile (Hardest)

## Timeline
Target for 2022 Q2 except VFRCompile

