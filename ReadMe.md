Google Summer of Code 2021 project: Enable Clang/LLVM Build for Microsoft Windows

Summary:
Add clang + gnu make build support for Edk2 BaseTools in windows
1.	Building .exes for the C tools in Edk2 BaseTools using LLVM/Clang in windows.
2.	Switching from nmake to make for Edk2 BaseTools build in windows.

Owner: tianocore

timeline:
2021/06 - 2021/07: building .exes for the C tools in BaseTools using LLVM/Clang
2021/07 - 2021/08: Switching from nmake to make for LLVM/Clang based builds.

links to related materials: 
none

Steps to use:

# In windows:
-------------------------------------------------------------------------------------------------------------
# Clang + make in windows command prompt:
Setup:
1.	Download and install LLVM 11 from https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.0/LLVM-11.0.0-win64.exe. After install, please verify the clang version is 11.0.0. as below.
	
        C:\>"C:\Program Files\LLVM\bin\clang.exe" -v
            clang version 11.0.0
            Target: x86_64-pc-windows-msvc
            Thread model: posix
            InstalledDir: C:\Program Files\LLVM\bin

2.	Download and install nasm and iasl:
		Download nasm compiler http://www.nasm.us/, copy nasm.exe to C:\nasm\ directory.
		Download iasl compiler https://acpica.org/downloads, copy iasl.exe to C:\ASL directory.

3.	Download the latest version Python from https://www.python.org/downloads/ and install it

4.	Download Visual Studio 2015 or 2017 or 2019 and install it, make sure nmake.exe, cl.exe, lib.exe and link.exe be ready.
    The Visual Studio is required only because the Windows SDK Universal C runtime (UCRT) library depends on the MSVC. Please see the dependency description here: “When you install Visual C++, Visual Studio setup installs the subset of the Windows 10 SDK required to use the UCRT.” https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-160. 

5.	In windows command prompt: (please replace %USERNAME% with username in your computer)
	Can directly download the windows version gnumake binary from conan center. Below are the download steps:
        
        C:\Users\%USERNAME%\edk2>C:\Python38\python.exe -m pip install conan
        C:\Users\%USERNAME%\edk2>set PATH=%PATH%;C:\Python38\Scripts\
        C:\Users\%USERNAME%\edk2>conan download make/4.2.1:0a420ff5c47119e668867cdb51baff0eca1fdb68
        C:\Users\%USERNAME%\edk2>C:\Users\%USERNAME%\.conan\data\make\4.2.1\_\_\package\0a420ff5c47119e668867cdb51baff0eca1fdb68\bin\gnumake.exe --version
            GNU Make 4.2.1
            Built for Windows32
            (please check the version)

6.	In windows command prompt: (please replace %USERNAME% with username in your computer)

        C:\Users\%USERNAME%>git clone https://github.com/tianocore/edk2.git edk2
        C:\Users\%USERNAME%\edk2>git submodule update --init
        C:\Users\%USERNAME%\edk2>git submodule update --recursive
        C:\Users\%USERNAME%\edk2>git submodule sync --recursive

Use:

	C:\Users\%USERNAME%\edk2>git clean -dfx (use this command if you have download it before, else please ignore thie command) 
	C:\Users\%USERNAME%\edk2>edksetup.bat    
	C:\Users\%USERNAME%\edk2>C:\Users\%USERNAME%\.conan\data\make\4.2.1\_\_\package\0a420ff5c47119e668867cdb51baff0eca1fdb68\bin\gnumake.exe -w -C BaseTools CXX=llvm

# Clang + nmake in windows command prompt:
Setup:
1.	Download and install LLVM 11 from https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.0/LLVM-11.0.0-win64.exe. After install, please verify the clang version is 11.0.0. as below.

	    C:\>"C:\Program Files\LLVM\bin\clang.exe" -v
            clang version 11.0.0
            Target: x86_64-pc-windows-msvc
            Thread model: posix
            InstalledDir: C:\Program Files\LLVM\bin

2.	Download and install nasm and iasl:
		Download nasm compiler http://www.nasm.us/, copy nasm.exe to C:\nasm\ directory.
		Download iasl compiler https://acpica.org/downloads, copy iasl.exe to C:\ASL directory.

3.	Download the latest version Python from https://www.python.org/downloads/ and install it

4.	Download Visual Studio 2015 or 2017 or 2019 and install it, make sure nmake.exe, cl.exe, lib.exe and link.exe be ready.

5.	In windows command prompt: (please replace %USERNAME% with username in your computer)

        C:\Users\%USERNAME%>git clone https://github.com/tianocore/edk2.git edk2
        C:\Users\%USERNAME%\edk2>git submodule update --init
        C:\Users\%USERNAME%\edk2>git submodule update --recursive
        C:\Users\%USERNAME%\edk2>git submodule sync --recursive

Use:    

    C:\Users\%USERNAME%\edk2>git clean -dfx (use this command if you have download it before, else please ignore thie command)    
    C:\Users\%USERNAME%\edk2>edksetup.bat ForceRebuild clang 

# MSVC + nmake in windows command prompt:
Setup:
1.	Download and install LLVM 11 from https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.0/LLVM-11.0.0-win64.exe. After install, please verify the clang version is 11.0.0. as below.
	
        C:\>"C:\Program Files\LLVM\bin\clang.exe" -v
            clang version 11.0.0
            Target: x86_64-pc-windows-msvc
            Thread model: posix
            InstalledDir: C:\Program Files\LLVM\bin

2.	Download and install nasm and iasl:
		Download nasm compiler http://www.nasm.us/, copy nasm.exe to C:\nasm\ directory.
		Download iasl compiler https://acpica.org/downloads, copy iasl.exe to C:\ASL directory.

3.	Download the latest version Python from https://www.python.org/downloads/ and install it

4.	Download Visual Studio 2015 or 2017 or 2019 and install it, make sure nmake.exe, cl.exe, lib.exe and link.exe be ready.

5.	In windows command prompt: (please replace %USERNAME% with username in your computer)

        C:\Users\%USERNAME%>git clone https://github.com/tianocore/edk2.git edk2
        C:\Users\%USERNAME%\edk2>git submodule update --init
        C:\Users\%USERNAME%\edk2>git submodule update --recursive
        C:\Users\%USERNAME%\edk2>git submodule sync --recursive

Use:

    C:\Users\%USERNAME%\edk2>git clean -dfx (use this command if you have download it before, else please ignore thie command)
    C:\Users\%USERNAME%\edk2>edksetup.bat ForceRebuild

# In Linux:
*********************************
# GCC + make:
Setup:
1.	Download and install LLVM 11:
	Create a folder called llvm and open it,

        %username%:~/llvm$ wget https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.0/clang+llvm-11.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz
        %username%:~/llvm$ tar -xvf clang+llvm-11.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz
        %username%:~/llvm$ ./clang+llvm-11.0.0-x86_64-linux-gnu-ubuntu-20.04/bin/clang -v
            clang version 11.0.0 (https://github.com/llvm/llvm-project.git 0160ad802e899c2922bc9b29564080c22eb0908c)
            Target: x86_64-unknown-linux-gnu
            Thread model: posix

2.	Download and install nasm and iasl:

	    %username%:~/edk2-3$ sudo apt-get install build-essential git uuid-dev iasl nasm

3.	Download the latest version Python from https://www.python.org/downloads/ and install it

4.	Init:

        %USERNAME%:~$ git clone https://github.com/tianocore/edk2.git edk2
        %USERNAME%:~/edk2$ git submodule update --init
        %USERNAME%:~/edk2$ git submodule update --recursive
        %USERNAME%:~/edk2$ git submodule sync --recursive
Use:  

    %USERNAME%:~/edk2$ git clean -dfx	(use this command if you have download it before, else please ignore thie command)   
    %USERNAME%:~/edk2$ source edksetup.sh   
    %USERNAME%:~/edk2$ make -C BaseTools/ 

# Clang + make:
Setup:
1.	Download and install LLVM 11:
	Create a folder called llvm and open it,
    
        %username%:~/llvm$ wget https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.0/clang+llvm-11.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz
        %username%:~/llvm$ tar -xvf clang+llvm-11.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz
        %username%:~/llvm$ ./clang+llvm-11.0.0-x86_64-linux-gnu-ubuntu-20.04/bin/clang -v
            clang version 11.0.0 (https://github.com/llvm/llvm-project.git 0160ad802e899c2922bc9b29564080c22eb0908c)
            Target: x86_64-unknown-linux-gnu
            Thread model: posix

2.	Download and install nasm and iasl:
        
        %username%:~/edk2-3$ sudo apt-get install build-essential git uuid-dev iasl nasm

3.	Download the latest version Python from https://www.python.org/downloads/ and install it

4.	Init:

        %USERNAME%:~$ git clone https://github.com/tianocore/edk2.git edk2
        %USERNAME%:~/edk2$ git submodule update --init
        %USERNAME%:~/edk2$ git submodule update --recursive
        %USERNAME%:~/edk2$ git submodule sync --recursive

Use:  Open file edk2  

    %USERNAME%:~/edk2$ git clean -dfx	(use this command if you have download it before, else please ignore thie command)    
    %USERNAME%:~/edk2$ export CLANG_BIN=~/llvm/clang+llvm-11.0.0-x86_64-linux-gnu-ubuntu-20.04/bin/     
    %USERNAME%:~/edk2$ source edksetup.sh    
    %USERNAME%:~/edk2$ make -C BaseTools/ CXX=llvm
	

