## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import sys
import platform
import subprocess
from optparse import OptionParser

__prog__ = 'RunLibFuzzer.py'
__copyright__ = 'Copyright (c) 2019, Intel Corporation. All rights reserved.'
__version__ = '{} Version {}'.format(__prog__, '0.1 ')

## WORKSPACE
workspace = ""

## HBFA package path
HBFA_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))

## Conf directory
Conf_Path = os.path.join(HBFA_PATH, 'UefiHostFuzzTestPkg', 'Conf')

## Get PYTHON version
PyVersion = sys.version_info[0]

## Get System type info
SysType = platform.system()

## Get System arch info
Is32bit = bool("64" not in platform.machine())

## build tool chain
if SysType == "Windows":
    ToolChain = "LIBFUZZERWIN"
elif SysType == "Linux":
    ToolChain = "LIBFUZZER"

def CheckTestEnv(Arch):
    # Check whether EDKII BUILD WORKSPACE is set in system environment variable
    if 'WORKSPACE' not in os.environ:
        print("Please set system environment variable 'WORKSPACE' before run this script.")
        os._exit(0)
    global workspace
    workspace = os.environ['WORKSPACE']

    # Check whether HBFA environment is set up
    if 'build_rule.txt' not in os.listdir(Conf_Path) or 'tools_def.txt' not in os.listdir(Conf_Path) or 'target.txt'\
            not in os.listdir(Conf_Path):
        print("Please run HBFAEnvSetup.py before run this script.")
        os._exit(0)

    if SysType == "Windows":
        if Arch == "IA32":
            # Need to remove when LLVM support libFuzzer i386
            print("LLVM doesn't support libFuzzer i386 currently.")
            os._exit(0)
            # Check whether environment variable LLVMx86_PATH has been set
            if "LLVMx86_PATH" not in os.environ:
                print("Please set LLVMx86_PATH in system environment variable.")
                os._exit(0)
        elif Arch == "X64":
            # Check whether environment variable LLVM_PATH has been set
            if "LLVM_PATH" not in os.environ:
                print("Please set LLVM_PATH in system environment variable.")
                os._exit(0)
    else:
        # Check whether environment variable CLANG_PATH has been set
        if "CLANG_PATH" not in os.environ:
            print("Please set CLANG_PATH in system environment variable.")
            os._exit(0)

def GetPkgName(path):
    return path.split(os.path.sep)[0]

def GetModuleBinName(ModuleInfPath):
    with open(ModuleInfPath, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if 'BASE_NAME' in line:
                if SysType == "Windows":
                    return line.split('=')[1].strip() + '.exe'
                elif SysType == "Linux":
                    return line.split('=')[1].strip()

def CheckBuildResult(ModuleBinPath):
    if os.path.exists(ModuleBinPath):
        print("Build Successfully !!!\n")
    else:
        print("Build failure, can not find Module Binary file: {}".format(ModuleBinPath))
        os._exit(0)
    

def Build(Arch, Target, ModuleFilePath):
    PkgName = GetPkgName(ModuleFilePath)  
    ModuleFileAbsPath = os.path.join(HBFA_PATH, ModuleFilePath)
    ModuleBinName = GetModuleBinName(ModuleFileAbsPath)
    ModuleBinAbsPath = os.path.join(workspace, 'Build', PkgName, Target + '_' + ToolChain, Arch, ModuleBinName)
    PlatformDsc = os.path.join(PkgName, PkgName+'.dsc')

    BuildCmdList = []
    BuildCmdList.append('-p {}'.format(PlatformDsc))
    BuildCmdList.append('-m {}'.format(ModuleFilePath))
    BuildCmdList.append('-a {}'.format(Arch))
    BuildCmdList.append('-b {}'.format(Target))
    BuildCmdList.append('-t {}'.format(ToolChain))
    BuildCmdList.append('--conf {}'.format(Conf_Path))

    BuildCmd = 'build ' + ' '.join(BuildCmdList)
    if SysType == "Windows":
        ExecCmd = BuildCmd
    elif SysType == "Linux":
        SetEnvCmd = "export PATH=$CLANG_PATH:$PATH\n"
        ExecCmd = SetEnvCmd + BuildCmd
    print("Start build Test Module:")
    print(BuildCmd)
    proccess = subprocess.Popen(ExecCmd,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT,
                                shell=True)
    msg = list(proccess.communicate())
    if PyVersion == 3:
        for num, submsg in enumerate(msg):
            if submsg is not None:
                msg[num] = submsg.decode()

    if msg[1]:
        print(msg[0] + msg[1])
        os._exit(0)
    elif "- Done -" not in msg[0]:
        print(msg[0])
        os._exit(0)
    else:
        pass
    CheckBuildResult(ModuleBinAbsPath)
    return ModuleBinAbsPath

def RunLibFuzzer(TestModuleBinPath, InputPath, OutputPath):
    LibFuzzer_CMD = "{} {} -rss_limit_mb=0 -artifact_prefix={}".format(TestModuleBinPath, InputPath if InputPath else '', OutputPath + ('/' if SysType == "Linux" else '\\'))
    print("Start run LibFuzzer test:")
    print(LibFuzzer_CMD)
    if SysType == "Windows":
        ExecCmd = 'start cmd /k "{}"'.format(LibFuzzer_CMD)
    elif SysType == "Linux":
        ExecCmd = "gnome-terminal --geometry=90x30 -e 'bash -c \"{};exec bash\" ' ".format(LibFuzzer_CMD)
    subprocess.Popen(ExecCmd,
                     stdout=subprocess.PIPE,
                     stderr=subprocess.STDOUT,
                     shell=True)

gParamCheck = []
def SingleCheckCallback(option, opt_str, value, parser):
    if option not in gParamCheck:
        setattr(parser.values, option.dest, value)
        gParamCheck.append(option)
    else:
        parser.error("Option %s only allows one instance in command line!" % option)

# Parse command line options
def MyOptionParser():
    Parser = OptionParser(description=__copyright__, version=__version__, prog=__prog__, usage="python %prog [options][argument]")
    Parser.add_option("-a", "--arch", action="callback", type="choice", choices=['IA32', 'X64', 'ARM', 'AARCH64'], dest="TargetArch", default="IA32", callback=SingleCheckCallback,
        help="ARCHS is one of list: IA32, X64, ARM or AARCH64, which overrides target.txt's TARGET_ARCH definition.")
    Parser.add_option("-b", "--buildtarget", action="callback", type="string", dest="BuildTarget", default="DEBUG", callback=SingleCheckCallback,
        help="Using the TARGET to build the platform, overriding target.txt's TARGET definition.")
    Parser.add_option("-m", "--module", action="callback", type="string", dest="ModuleFile", callback=SingleCheckCallback,
        help="Build the module specified by the INF file name argument.")
    Parser.add_option("-i", "--input", action="callback", type="string", dest="InputSeed", callback=SingleCheckCallback,
        help="Test input seed path.")
    Parser.add_option("-o", "--output", action="callback", type="string", dest="Output", callback=SingleCheckCallback,
        help="Test output path for LibFuzzer.")
    (Opt, Args) = Parser.parse_args()
    return (Opt, Args)

def main():
    (Option, Target) = MyOptionParser()
    TargetArch = Option.TargetArch
    BuildTarget = Option.BuildTarget

    CheckTestEnv(TargetArch)
    
    if (SysType == 'Linux'):
        if (TargetArch == "IA32") ^ Is32bit:
            print("For CLANG: {} system cannot support arch {} build.".format("i386" if Is32bit else "x64_86", TargetArch))
            os._exit(0)

    if not Option.ModuleFile:
        print("ModuleFile should be set once by command -m MODULEFILE, --module=MODULEFILE.")
        os._exit(0)
    elif os.path.isabs(Option.ModuleFile):
        if Option.ModuleFile.startswith(HBFA_PATH):
            ModuleFilePath = os.path.relpath(Option.ModuleFile, HBFA_PATH)
        else:
            print("ModuleFile path: {} should be start with {}.".format(Option.ModuleFile, HBFA_PATH))
            os._exit(0)
    elif not os.path.exists(os.path.join(HBFA_PATH, Option.ModuleFile)):
        print("ModuleFile path: {} is no exits or not the relative path for HBFA".format(os.path.abspath(Option.InputSeed)))
        os._exit(0)
    else:
        ModuleFilePath = Option.ModuleFile

    OutputPath = Option.Output if Option.Output else os.path.join(os.getcwd(), 'failureSeeds')
    if not os.path.isabs(OutputPath):
        OutputPath = os.path.join(os.getcwd(), OutputPath)
    if not os.path.exists(OutputPath):
        try:
            os.makedirs(OutputPath)
        except Exception as err:
            print(err)
    elif os.path.exists(os.path.join(OutputPath, 'fuzzfile.bin')):
        print("OutputPath:{} has already been used for LibFuzzer test, please change another directory.".format(OutputPath))
        os._exit(0)
    print("LibFuzzer output will be generated in current directory:{}".format(OutputPath))

    InputSeedPath = Option.InputSeed if Option.InputSeed else os.path.join(os.getcwd(), 'Seeds')
    if not os.path.isabs(InputSeedPath):
        InputSeedPath = os.path.join(os.getcwd(), InputSeedPath)
    if not os.path.exists(InputSeedPath):
        try:
            os.makedirs(InputSeedPath)
        except Exception as err:
            print(err)

    TestModuleBinPath = Build(TargetArch, BuildTarget, ModuleFilePath)
    RunLibFuzzer(TestModuleBinPath, InputSeedPath, OutputPath)

if __name__ == "__main__":
    main()


