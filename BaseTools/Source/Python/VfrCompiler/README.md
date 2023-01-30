# Python VfrCompiler Tool
## Overview
This python VfrCompiler tool is the implementation of the edk2 VfrCompiler tool which locates at https://github.com/tianocore/edk2/tree/master/BaseTools/Source/C/VfrCompile.

### Introduction
The core function of the original C VfrCompiler tool is to convert the vfr file into IFR binary array. However, the vfr format file syntax is relatively uncommon and has poor readability for users. What's more, the C tool does not collect and store extra information except for binary generation. When any modification is needed, developers have to read and deal with the IFR binary, which is quite inefficient. To solve this issue, this python VfrCompiler tool proposes to generate the yaml format file on the basis of the vfr format file. With the yaml file,  the layout and hierarchy of all Opcodes in the vfr file will be saved. Then modification will be much eaiser. In addition, an extra json file will be generated for better variable related information presentation. So the long-term aim of this tool is to cover the same usage as the edk2 C VfrCompiler and extend the yaml/json output for better readability and variability.

### Main update in this commitment
- Update the vfr parser generator from ANTLR to ANTLR v4, which is more readable and is eaiser to add on new functions.
- Generate a yaml file for each vfr file. The yaml file displays all the contents in the Vfr file by different Opcode types in sequence.
- Generate a json file for each vfr file. The json file contains all the default values for different Opcode types in vfr files.

### Known issues

- The python VfrCompiler tool aims to cover the same usage as the edk2 C VfrCompiler. But currently, the python VfrCompiler tool does not support IFR binary generation feature, this feature will be added in future update.
- The python VfrCompiler tool will extend new functions, which is able to compile both the vfr and yaml files. This feature will be added in future update.

### Use with Build System
- To use the VfrCompiler Python Tool with Build System,  please add the ***'--vfr-yaml-enable'***  option in the build command.
- For Example: ***build -p OvmfPkg\OvmfPkgIa32X64.dsc -a IA32 -a X64 -j build.log --vfr-yaml-enable***