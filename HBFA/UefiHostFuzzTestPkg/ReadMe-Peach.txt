How to run Peach with UefiHostTestPkg in OS?
==============
Peach:
Prepare Peach in Linux
1)	install mono-complete "sudo apt-get install mono-complete"
2)	goto https://sourceforge.net/projects/peachfuzz/, "Project Activity"
	Download https://sourceforge.net/projects/peachfuzz/files/Peach/3.1/peach-3.1.124-linux-x86-release.zip/download
	or https://sourceforge.net/projects/peachfuzz/files/Peach/3.1/peach-3.1.124-linux-x86_64-release.zip/download
3)	"cp peach-3.1.124-linux-x86_64-release.zip /home/<user name>/Env/peach"
4)	"cd /home/<user name>/Env/peach"
5)	"unzip peach-3.1.124-linux-x86_64-release.zip"
6)	Add below content at the end of ~/.bashrc:
	  export PEACH_PATH=<PEACH_PATH>
	  export PATH=$PATH:$PEACH_PATH
	For example:
	  export PEACH_PATH=/home/<user name>/Env/peach
	  export PATH=$PATH:$PEACH_PATH
7)	try "peach samples/HelloWorld.xml"
	If you find the screen has lots output, that means the peach runs successfully.

Prepare Peach in Windows
1)	Install Debugging Tools for Windows, go to https://developer.microsoft.com/zh-cn/windows/downloads/windows-10-sdk download winsdksetup.exe, run and just select "Debugging Tools for Windows" for install.
2)	Download Peach windows pre build binary from http://www.peach.tech/resources/peachcommunity/ and setup Peach environment according to http://community.peachfuzzer.com/v3/Installation.html.
3)	After Peach setup done, please add Peach installed location to system environment variable.
	  set PEACH_PATH=<PEACH_PATH>
	For example:
	  set PEACH_PATH=C:\PEACH
4)	Add %PEACH_PATH% to system environment variable PATH.
5)	try "peach samples\HelloWorld.xml"
	If you find the screen has lots output, that means the peach runs successfully.
	NOTE: for error about Peach.Core.OS.Windows.dll, please right click file -> Properties -> General -> Select "Unblock" box. 



Build Test App.
Build in Linux
1)	Build all test with GCC5 or CLANG8 tool chain. (some test will use GCC5 and some test will use CLANG8)
	build -p UefiHostFuzzTestCasePkg/UefiHostFuzzTestCasePkg.dsc -a X64 -t GCC5
	or
	python edk2-staging/HBFA/UefiHostTestTools/HBFAEnvSetup.py
	cp edk2-staging/HBFA/UefiHostFuzzTestPkg/Conf/build_rule.txt edk2/Conf/build_rule.txt
	cp edk2-staging/HBFA/UefiHostFuzzTestPkg/Conf/tools_def.txt edk2/Conf/tools_def.txt
	export PATH=$CLANG_PATH:$PATH
	build -p UefiHostFuzzTestCasePkg/UefiHostFuzzTestCasePkg.dsc -a X64 -t CLANG8
   
Build in Windows
1)	Build all test with VS2015x86 or CLANGWIN tool chain. (some test will use VS2015x86 and some test will use CLANGWIN)
	build -p UefiHostFuzzTestCasePkg\UefiHostFuzzTestCasePkg.dsc -a X64 -t VS2015x86
	or
	python edk2-staging\HBFA\UefiHostTestTools\HBFAEnvSetup.py
	copy edk2-staging\HBFA\UefiHostFuzzTestPkg\Conf\build_rule.txt edk2\Conf\build_rule.txt
	copy edk2-staging\HBFA\UefiHostFuzzTestPkg\Conf\tools_def.txt edk2\Conf\tools_def.txt
	build -p UefiHostFuzzTestCasePkg\UefiHostFuzzTestCasePkg.dsc -a X64 -t CLANGWIN



Write EDKII Peach PIT file
1)	Read http://community.peachfuzzer.com/v3/PeachQuickStart.html
2)	Write PIT file in "HBFA/UefiHostFuzzTestCasePkg/TestCase/XXX/PeachDataModel/TestXXX.xml"
2.1)	Define DataModel.
  <DataModel name="VARIABLE_STORE_HEADER">
    <Blob name="Signature" length="16" value="782cf3aa7b949a43a1802e144ec37792" valueType="hex" mutable="false"/>
    <Number name="Size" size="32" value="00001000" valueType="hex"/>
    ......
  </DataModel>

2.2)	Define Executable and Arguments
    <Monitor class="LinuxDebugger">
      <Param name="Executable" value="<...>/Build/UefiHostFuzzTestCasePkg/DEBUG_GCC5/IA32/TestUsb" />
      <Param name="Arguments" value="fuzzfile.bin" />
    </Monitor>
	NOTE: you need to set the binary you built as executable file in PIT file.



Run Peach
1)	"peach HBFA/UefiHostFuzzTestCasePkg/TestCase/XXX/PeachDataModel/TestXXX.xml"
2)	You will see screen has lots of output. Have fun.

=============================================
[2052,-,-] Performing iteration
[*] Fuzzing: TheDataModel.DataElement_0
[*] Mutator: DataElementDuplicateMutator
Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!
[2053,-,-] Performing iteration
[*] Fuzzing: TheDataModel.DataElement_0
[*] Mutator: StringMutator
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
[2055,-,-] Performing iteration
[*] Fuzzing: TheDataModel.DataElement_0
[*] Mutator: StringMutator
134
