## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import sys
import platform
import subprocess
import glob
from optparse import OptionParser

__prog__ = 'RunPeach.py'
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

def CheckTestEnv(EnableSanitizer, Arch):
    # Check whether EDKII BUILD WORKSPACE is set in system environment variable
    if 'WORKSPACE' not in os.environ:
        print("Please set system environment variable 'WORKSAPCE' before run this script.")
        os._exit(0)
    global workspace
    workspace = os.environ['WORKSPACE']

    # Check whether HBFA environment is set up
    if 'build_rule.txt' not in os.listdir(Conf_Path) or 'tools_def.txt' not in os.listdir(Conf_Path) or 'target.txt'\
            not in os.listdir(Conf_Path):
        print("Please run HBFAEnvSetup.py before run this script.")
        os._exit(0)

    # Check whether PEACH_PATH is set in system environment variable
    if "PEACH_PATH" not in os.environ:
        print("Please set PEACH_PATH in system environment variables.")
        os._exit(0)

    if SysType == "Windows":
        if Arch == "IA32":
            # Check whether environment variable LLVMx86_PATH has been set
            if EnableSanitizer and "LLVMx86_PATH" not in os.environ:
                print("Please set LLVMx86_PATH in system environment variable.")
                os._exit(0)
        elif Arch == "X64":
            # Check whether environment variable LLVM_PATH has been set
            if EnableSanitizer and "LLVM_PATH" not in os.environ:
                print("Please set LLVM_PATH in system environment variable.")
                os._exit(0)
    else:
        # Check whether environment variable CLANG_PATH has been set
        if EnableSanitizer and "CLANG_PATH" not in os.environ:
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
                    return line.split('=')[1].strip() + ".exe"
                elif SysType == "Linux":
                    return line.split('=')[1].strip()

def GetXmlPath(ModuleInfPath, Arch):
    PossibleXml = os.path.join(os.path.dirname(ModuleInfPath), 'PeachDataModel', os.path.basename(ModuleInfPath).split('.')[0] + '*.xml')
    XmlPaths = glob.glob(PossibleXml)
    if len(XmlPaths) < 1:
        print("Test case XML file: {} is not exists.".format(PossibleXml))
        os._exit(0)
    for XmlPath in XmlPaths:
        if Arch.lower() in XmlPath.lower() and os.path.exists(XmlPath):
            return XmlPath
    if os.path.exists(XmlPaths[-1]):
        return XmlPaths[-1]
    else:
        print("Test case XML file: {} is not exists.".format(XmlPaths[-1]))
        os._exit(0)

def GenTestXml(src_xml, temp_xml, ModuleBinPath, EnableSanitizer):
    if not os.path.exists(src_xml):
        print("Test case xml file: {} is not exists.".format(src_xml))
        os._exit(0)
    with open(src_xml, 'r') as f:
        content = f.read()

    if EnableSanitizer:
        if SysType == 'Windows':
            content = content.replace('TestModule', os.path.join(HBFA_PATH, 'UefiHostTestTools', 'GenSanitizerInfo.exe'))
            content = content.replace('fuzzfile.bin', '-e {} -i fuzzfile.bin -o .'.format(ModuleBinPath), 2)
        elif SysType == "Linux":
            content = content.replace('TestModule', os.path.join(HBFA_PATH, 'UefiHostTestTools', 'GenSanitizerInfo'))
            content = content.replace('fuzzfile.bin', '-e {} -i fuzzfile.bin -o .'.format(ModuleBinPath), 2)
    else:
        content = content.replace('TestModule', ModuleBinPath)

    if os.path.exists(temp_xml):
        os.remove(temp_xml)

    with open(temp_xml, 'w') as f1:
        f1.write(content)

def CheckBuildResult(ModuleBinPath):
    if os.path.exists(ModuleBinPath):
        print("Build Successfully !!!\n")
    else:
        print("Build failure, can not find Module Binary file: {}".format(ModuleBinPath))
        os._exit(0)
    

def Build(Arch, Target, ModuleFilePath, EnableSanitizer):
    ## For toolchain, windows use VS2015x86, Linux use GCC5 if not enable sanitizer and use CLANG8 if enable sanitizer
    if SysType == "Windows":
        if EnableSanitizer:
            ToolChain = "CLANGWIN"
        else:
            ToolChain = "VS2015x86"
    elif SysType == "Linux":
        if EnableSanitizer:
            ToolChain = "CLANG8"
        else:
            ToolChain = "GCC5"

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
    print("Start build Test Module:")
    print(BuildCmd)
    if SysType == "Windows":
        ExecCmd = BuildCmd
    elif SysType == "Linux":
        SetEnvCmd = "export PATH=$CLANG_PATH:$PATH\n"
        ExecCmd = SetEnvCmd + BuildCmd
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

def RunPeachSanitizer(ModuleFilePath, Arch, TestModuleBinPath, OutputPath, EnableSanitizer):
    ModuleFileAbsPath = os.path.join(HBFA_PATH, ModuleFilePath)
    TestXmlPath = GetXmlPath(ModuleFileAbsPath, Arch)
    TempTestXmlPath = os.path.join(os.path.dirname(TestXmlPath), ('test_san' if EnableSanitizer else 'test') + '_{}'.format(Arch.lower()) + '.xml')
    GenTestXml(TestXmlPath, TempTestXmlPath, TestModuleBinPath, EnableSanitizer)
    PEACH_CMD = "peach {}".format(TempTestXmlPath)
    print("Start run Peach test:")
    print(PEACH_CMD)

    if SysType == "Windows":
        SetExecEnvCmd = "set PATH=%PEACH_PATH%;%PATH%"
        ExecCmd = 'start cmd /k "cd/d {} && {}  && {}"'.format(OutputPath, SetExecEnvCmd, PEACH_CMD)
    elif SysType == "Linux":
        SetExecEnvCmd = "export PATH=$PEACH_PATH:$PATH\n"
        ExecCmd = "gnome-terminal --geometry=90x30 -e 'bash -c \"cd {} && {};exec bash\" ' ".format(OutputPath, SetExecEnvCmd + PEACH_CMD)
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
    Parser.add_option("-o", "--output", action="callback", type="string", dest="Output", callback=SingleCheckCallback,
        help="Test output path for PEACH.")
    Parser.add_option("--enablesanitizer", action="store_true", dest="EnableSanitizer", default=False,
        help="Using --enablesanitizer to enable sanitizer for peach test.")

    (Opt, Args) = Parser.parse_args()
    return (Opt, Args)

def main():
    (Option, Target) = MyOptionParser()
    TargetArch = Option.TargetArch
    BuildTarget = Option.BuildTarget
    EnableSanitizer = Option.EnableSanitizer

    CheckTestEnv(EnableSanitizer, TargetArch)

    if (SysType == 'Linux'):
        if (TargetArch == "IA32") ^ Is32bit and EnableSanitizer:
            print("For CLANG: {} system cannot support arch {} build.".format("i386" if Is32bit else "x86_64", TargetArch))
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
        print("ModuleFile path: {} is no exits or not the relative path for HBFA".format(Option.ModuleFile))
        os._exit(0)
    else:
        ModuleFilePath = Option.ModuleFile

    if not Option.Output:
        OutputPath = os.getcwd()
        print("PEACH output will be generated in current directory:{}".format(OutputPath))
    else:
        OutputPath = Option.Output
        if not os.path.isabs(OutputPath):
            OutputPath = os.path.join(os.getcwd(), OutputPath)
        if not os.path.exists(OutputPath):
            try:
                os.makedirs(OutputPath)
            except Exception as err:
                print(err)
        elif os.path.exists(os.path.join(OutputPath, 'fuzzfile.bin')):
            print("OutputPath:{} has already been used for peach test, please change another directory.".format(OutputPath))
            os._exit(0)


    TestModuleBinPath = Build(TargetArch, BuildTarget, ModuleFilePath, EnableSanitizer)
    RunPeachSanitizer(ModuleFilePath, TargetArch, TestModuleBinPath, OutputPath, EnableSanitizer)

if __name__ == "__main__":
    main()


