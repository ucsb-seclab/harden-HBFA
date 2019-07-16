How to run LibFuzzer in OS?
==============
Install Clang8 in Linux
1) goto http://clang.llvm.org/get_started.html, read the content.
2) Check out LLVM:
Change directory to where you want the llvm directory placed.
$ svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm
3) Check out Clang:
$ cd llvm/tools
$ svn co http://llvm.org/svn/llvm-project/cfe/trunk clang
$ cd ../..
4) Check out extra Clang tools: (optional)
$ cd llvm/tools/clang/tools
$ svn co http://llvm.org/svn/llvm-project/clang-tools-extra/trunk extra
$ cd ../../../..
5) Check out Compiler-RT:
$ cd llvm/projects
$ svn co http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt
$ cd ../..
6) Check out libcxx: (only required to build and run Compiler-RT tests on OS X, optional otherwise)
$ cd llvm/projects
$ svn co http://llvm.org/svn/llvm-project/libcxx/trunk libcxx
$ cd ../..
7) Build LLVM and Clang:
$ mkdir build (in-tree build is not supported)
$ cd build
$ cmake -G "Unix Makefiles" ../llvm
$ make
This builds both LLVM and Clang for debug mode.
8) Try it out (assuming you add llvm/build/bin to your path):
    export CLANG_PATH=/home/<user name>/Env/llvm/build/bin
    export ASAN_SYMBOLIZER_PATH=$CLANG_PATH/llvm-symbolizer
    export PATH=$CLANG_PATH:$PATH
$ clang --help
you will find clang-8 below:
OVERVIEW: clang LLVM compiler

USAGE: clang-8 [options] <inputs>

===============
Install Clang in Windows (currently CLAGN8)
***NOTE: current CLAGN8 support Sanitizer (IA32/X64) and LIBFUZZER (X64) in Windows.

1) goto http://releases.llvm.org/download.html
2) download pre-build binary.
3) See http://releases.llvm.org/8.0.0/docs/ReleaseNotes.html
4) Please install 64bit exe for X64 build and 32bit exe for IA32 build.
   setup LLVM_PATH=C:\Program Files\LLVM
   setup LLVMx86_PATH=C:\Program Files (x86)\LLVM

   Add %LLVM_PATH%\bin and %LLVM_PATH%\lib\clang\8.0.0\lib\windows to PATH.
   For IA32 build, add %LLVMx86_PATH%\lib\clang\8.0.0\lib\windows to PATH.

Build clang in windows (skip if clang is installed in above)
0) read https://llvm.org/docs/GettingStartedVS.html
1) read "Getting Started"
2) check out LLVM
   mkdir winllvm
   cd winllvm
   svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm
   cd llvm\tools
   svn co http://llvm.org/svn/llvm-project/cfe/trunk clang
   cd llvm\projects
   svn co http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt
   setup LLVM_PATH=<...>\winllvm
3) use CMake to generate project file.
   1) download CMake (http://www.cmake.org/)
   2) open visual studio dev command prompt, as administrator, run
      cd winllvm\llvm
      mkdir llvm-build
      cd llvm-build
      cmake -G"Visual Studio 14 2015" ..\llvm

      mkdir llvm-build64
      cd llvm-build64
      cmake -G"Visual Studio 14 2015 Win64" ..\llvm
4) start visual studio build
   1) build
      devenv LLVM.sln /Build Release /Project clang
      cd ..
   2) add env path
      Add %LLVM_PATH%\winllvm\llvm-build[64]\Release\bin to system PATH environment variable.

===========================
Run Clang in Linux
1) cat HBFA/UefiHostFuzzTestPkg/Conf/tools_def.customized >> Conf/tools_def.txt
2) cp HBFA/UefiHostFuzzTestPkg/Conf/build_rule.customized Conf/build_rule.txt
3) build with "CLANG8" as toolchain.
   build -p UefiHostFuzzTestCasePkg/UefiHostFuzzTestCasePkg.dsc -a X64 -t CLANG8
   
Run Clang in Windows
1) type HBFA\UefiHostFuzzTestPkg\Conf\tools_def.customized >> Conf\tools_def.txt
2) copy HBFA\UefiHostFuzzTestPkg\Conf\build_rule.customized to Conf\build_rule.txt
3) build with "CLANGWIN" as toolchain.
   build -p UefiHostFuzzTestCasePkg\UefiHostFuzzTestCasePkg.dsc -a X64 -t CLANGWIN

===========================
Run LibFuzzer in Linux:
1) goto https://llvm.org/docs/LibFuzzer.html, read the content.
2) cat HBFA/UefiHostFuzzTestPkg/Conf/tools_def.customized >> Conf/tools_def.txt
3) cp HBFA/UefiHostFuzzTestPkg/Conf/build_rule.customized Conf/build_rule.txt
4) build -p UefiHostFuzzTestCasePkg/UefiHostFuzzTestCasePkg.dsc -a X64 -t LIBFUZZER
5) mkdir NEW_CORPUS_DIR
6) cp HBFA/UefiHostFuzzTestCasePkg/Seed/XXX/Raw/Xxx.bin NEW_CORPUS_DIR
7) ./Build/UefiHostFuzzTestCasePkg/DEBUG_LIBFUZZER/X64/TestXxx  NEW_CORPUS_DIR

Run Clang in Windows
1) goto https://llvm.org/docs/LibFuzzer.html, read the content.
2) type HBFA\UefiHostFuzzTestPkg\Conf\tools_def.customized >> Conf\tools_def.txt
3) copy HBFA\UefiHostFuzzTestPkg\Conf\build_rule.customized to Conf\build_rule.txt
4) build -p UefiHostFuzzTestCasePkg\UefiHostFuzzTestCasePkg.dsc -a X64 -t LIBFUZZERWIN
5) mkdir NEW_CORPUS_DIR
6) copy HBFA\UefiHostFuzzTestCasePkg/Seed/XXX/Raw/Xxx.bin NEW_CORPUS_DIR
7) ./Build/UefiHostFuzzTestCasePkg/DEBUG_LIBFUZZERWIN/X64/TestXxx  NEW_CORPUS_DIR

===========================
Sanitizer Coverage:
1) goto http://clang.llvm.org/docs/SanitizerCoverage.html, read the content.
2) ASAN_OPTIONS=coverage=1 ./Build/UefiHostFuzzTestCasePkg/DEBUG_LIBFUZZER/X64/TestXxx
3) sancov -symbolize TestXxx.123.sancov my_program > TestXxx.123.symcov
4) python3 tools/sancov/coverage-report-server.py --symcov TestXxx.123.symcov --srcpath <source code root>
