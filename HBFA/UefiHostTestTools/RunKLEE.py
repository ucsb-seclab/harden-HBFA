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

__prog__='RunKLEE.py'
__copyright__ = 'Copyright (c) 2019, Intel Corporation. All rights reserved.'
__version__ = '{} Version {}'.format(__prog__, '0.1 ')

## Get System type info
SysType = platform.system()

## build tool chain
ToolChain = "KLEE"

## WORKSPACE
workspace = ""

## KLEE_SRC_PATH
klee_src_path = ""

## HBFA package path
HBFA_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))

## Conf directory
Conf_Path = os.path.join(HBFA_PATH, 'UefiHostFuzzTestPkg', 'Conf')

## Get PYTHON version
PyVersion = sys.version_info[0]

def CheckTestEnv():
    # Need to remove if support KLEE in Windows
    if SysType == 'Windows':
        print("KLEE is not supported in Windows currently.")
        os._exit(0)
        
    # Check whether EDKII BUILD WORKSPACE is set in system environment variable
    if 'WORKSPACE' not in os.environ:
        print("Please set system environment variable 'WORKSAPCE' before run this script.")
        os._exit(0)
    global workspace
    workspace = os.environ['WORKSPACE']

    # Check whether KLEE_SRC_PATH is set in system environment variable
    if 'KLEE_SRC_PATH' not in os.environ:
        print("Please set system environment variable 'KLEE_SRC_PATH' before run this script.")
        os._exit(0)
    global klee_src_path
    klee_src_path = os.environ['KLEE_SRC_PATH']

    # Check whether HBFA environment is set up
    if 'build_rule.txt' not in os.listdir(Conf_Path) or 'tools_def.txt' not in os.listdir(Conf_Path) or 'target.txt'\
            not in os.listdir(Conf_Path):
        print("Please run HBFAEnvSetup.py before run this script.")
        os._exit(0)

def GetPkgName(path):
    return path.split(os.path.sep)[0]

def GetModuleBinName(ModuleInfPath):
    with open(ModuleInfPath, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if 'BASE_NAME' in line:
                return line.split('=')[1].strip()

def CheckBuildResult(ModuleBinPath):
    if os.path.exists(ModuleBinPath):
        print("Build Successfully !!!\n")
    else:
        print("Can not find Module Binary file: {}".format(ModuleBinPath))
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
    BuildCmdList.append('-DTEST_WITH_KLEE')
    BuildCmdList.append('--disable-include-path-check')

    BuildCmd = 'build ' + ' '.join(BuildCmdList)
    SetEnvCmd = "export PATH=$KLEE_BIN_PATH:$PATH\nexport SCRIPT_PATH={}/UefiHostFuzzTestPkg/Conf/LLVMLink.py\nexport LLVM_COMPILER=clang\n".format(HBFA_PATH)
    print("Start build Test Module:")
    print(BuildCmd)
    proccess = subprocess.Popen(SetEnvCmd + BuildCmd,
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

def RunKLEE(TestModuleBinPath, OutPutPath):
    SetEnvCmd = "export PATH=$KLEE_BIN_PATH:$PATH\n"
    SetLimit = "ulimit -s unlimited\n"
    KLEE_CMD = "klee --only-output-states-covering-new -output-dir={}  {}".format(OutPutPath, TestModuleBinPath)
    print("Start run KLEE test:")
    print(KLEE_CMD)
    subprocess.Popen("gnome-terminal --geometry=90x30 -e 'bash -c \"{};exec bash\" ' ".format(SetEnvCmd + SetLimit + KLEE_CMD), 
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
    Parser.add_option("-o", "--output", action="callback", type="string", dest="Output", callback=SingleCheckCallback,
        help="Test output path for Klee.")
    (Opt, Args) = Parser.parse_args()
    return (Opt, Args)

def main():
    (Option, Target) = MyOptionParser()
    TargetArch = Option.TargetArch
    BuildTarget = Option.BuildTarget

    CheckTestEnv()

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
        print("ModuleFile path: {} is no exits or not the relative path for HBFA".format(Option.ModuleFile))
        os._exit(0)
    else:
        ModuleFilePath = Option.ModuleFile

    if not Option.Output:
        print("OutputSeed path should be set once by command -o OUTPUT, --output=OUTPUT.")
        os._exit(0)
    else:
        OutputSeedPath = Option.Output
        if os.path.exists(OutputSeedPath):
            print("OutputSeedPath:{} is already exists, please change another directory.".format(OutputSeedPath))
            os._exit(0)
        else:
            if not os.path.exists(os.path.dirname(OutputSeedPath)):
                os.makedirs(os.path.dirname(OutputSeedPath))
                

    TestModuleBinPath = Build(TargetArch, BuildTarget, ModuleFilePath)
    RunKLEE(TestModuleBinPath, OutputSeedPath)

if __name__ == "__main__":
    main()


