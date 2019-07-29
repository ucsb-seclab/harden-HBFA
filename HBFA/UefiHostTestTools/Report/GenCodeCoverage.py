## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import shutil
import platform
from optparse import OptionParser

__prog__ = 'GenCodeCoverage.py'
__copyright__ = 'Copyright (c) 2019, Intel Corporation. All rights reserved.'
__version__ = '{} Version {}'.format(__prog__, '0.1 ')

WORK_DIR = os.getcwd()

## Get System type info
SysType = platform.system()

## HBFA package path
HBFA_PATH = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

## WORKSPACE
workspace = ''

# Check EDKII BUILD WORKSPACE whether be set in system environment variable
if 'WORKSPACE' not in os.environ:
    print("Please set system environment variable 'WORKSPACE' before run this script.")
    os._exit(0)
workspace = os.environ['WORKSPACE']

def CheckTestEnv():
    if SysType == "Windows" and "DRIO_PATH" not in os.environ:
        print("Please set DRIO_PATH in system environment variables.")
        os._exit(0)

def delete_gcda_file(TestModuleBinPath):
    for root, dirs, files in os.walk(os.path.dirname(TestModuleBinPath)):
        for file in files:
            if file.endswith('.gcda'):
                file_path = os.path.join(root, file)
                try:
                    os.remove(file_path)
                except Exception as err:
                    print(err)

def HasIni(path):
    if os.listdir(path) == []:
        return False

    for file in os.listdir(path):
        if file.endswith(".ini"):
            return True

    return False

def CreateGcovTool(path):
    with open(path, 'w') as fd:
        fd.write('#!/bin/bash\nexec llvm-cov gcov "$@"')
    os.system("chmod +x {}".format(path))
    
def CheckIsPeach(path):
    for root, dirs, files in os.walk(path):
        for file in files:
            if file == 'status.txt':
                statusPath=os.path.join(root,file)
                with open(statusPath) as f:
                    contents=f.read()
                    if 'Peach Fuzzing Run' in contents:
                        return True
            elif file.endswith('Initial.Action.bin'):
                return True
    return False

def Run_All_Seeds(TestModuleBinPath, SeedPath, TestIniPath):
    TestModuleBinFolder = os.path.dirname(TestModuleBinPath)
    if SysType == "Windows":
        LogDir = os.path.join(TestModuleBinFolder, "temp", "log")
        if os.path.exists(os.path.dirname(LogDir)):
            shutil.rmtree(os.path.dirname(LogDir))
        os.makedirs(LogDir)
        for file in os.listdir(SeedPath):
            SeedFilePath = os.path.join(SeedPath, file)
            if os.path.isfile(SeedFilePath):
                if "IA32" in TestModuleBinPath:
                    cmd = r"cd {} && %DRIO_PATH%\bin32\drrun.exe -c %DRIO_PATH%\tools\lib32\release\drcov.dll -- {} {}".format(LogDir, TestModuleBinPath, SeedFilePath)
                elif "X64" in TestModuleBinPath:
                    cmd = r"cd {} && %DRIO_PATH%\bin64\drrun.exe -c %DRIO_PATH%\tools\lib64\release\drcov.dll -- {} {}".format(LogDir, TestModuleBinPath, SeedFilePath)
                try:
                    os.system(cmd)
                except Exception as err:
                    print(err)
    elif SysType == "Linux":
        if TestIniPath != "":
            for file in os.listdir(SeedPath):
                SeedFilePath = os.path.join(SeedPath, file)
                if os.path.isfile(SeedFilePath):
                    for IniFile in os.listdir(TestIniPath):
                        IniFilePath = os.path.join(TestIniPath, IniFile)
                        cmd = TestModuleBinPath + ' ' + SeedFilePath + ' ' + IniFilePath
                        try:
                            os.system(cmd)
                        except Exception as err:
                            print(err)
        else:
            for file in os.listdir(SeedPath):
                SeedFilePath = os.path.join(SeedPath, file)
                if os.path.isfile(SeedFilePath):
                    cmd = TestModuleBinPath + ' ' + SeedFilePath
                    try:
                        os.system(cmd)
                    except Exception as err:
                        print(err)

def GenCodeCoverage(TestModuleBinPath, ReportPath):
    TestModuleBinFolder = os.path.dirname(TestModuleBinPath)
    if SysType == "Windows":
        LogDir = os.path.join(TestModuleBinFolder, "temp", "log")
        TempDir = os.path.join(TestModuleBinFolder, "temp")
        if "IA32" in TestModuleBinPath:
            try:
                os.system(r"cd {} && %DRIO_PATH%\tools\bin32\drcov2lcov.exe -dir {} -src_filter {}".format(TempDir, LogDir, workspace.lower()))
                os.system(r"cd {} && perl %DRIO_PATH%\tools\bin32\genhtml coverage.info".format(TempDir))
            except Exception as err:
                print(err)
        elif "X64" in TestModuleBinPath:
            try:
                os.system(r"cd {} && %DRIO_PATH%\tools\bin64\drcov2lcov.exe -dir {} -src_filter {}".format(TempDir, LogDir, workspace.lower()))
                os.system(r"cd {} && perl %DRIO_PATH%\tools\bin64\genhtml coverage.info".format(TempDir))
            except Exception as err:
                print(err)

        if os.path.exists(ReportPath):
            shutil.rmtree(ReportPath)
        shutil.copytree(os.path.join(TestModuleBinFolder, "temp"), ReportPath)
    elif SysType == "Linux":
        try:
            if "CLANG8" in TestModuleBinFolder:
                GcovToolPath = os.path.join(WORK_DIR, 'llvm-gcov.sh')
                if not os.path.exists(GcovToolPath):
                    CreateGcovTool(GcovToolPath)
                os.system("lcov --capture --directory {} --gcov-tool {} --output-file coverage.info".format(TestModuleBinFolder, GcovToolPath))
                os.remove(GcovToolPath)
            else:
                os.system("lcov --capture --directory {} --output-file coverage.info".format(TestModuleBinFolder))
            os.system("lcov -r coverage.info '*UefiHostTestPkg*' --output-file coverage.info".format(TestModuleBinFolder))
        except Exception as err:
            print(err)

        try:
            os.system("genhtml coverage.info --output-directory {}".format(ReportPath))
            os.remove('coverage.info')
        except Exception as err:
            print(err)


    print("Please view code coverage report in {}".format(ReportPath))


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
    Parser.add_option("-e", "--execbinary", action="callback", type="string", dest="ModuleBin", callback=SingleCheckCallback,
        help="Test module binary file name.")
    Parser.add_option("-d", "--dir", action="callback", type="string", dest="SeedPath", callback=SingleCheckCallback,
        help="Test output seed directory path.")
    Parser.add_option("-t", "--testini", action="callback", type="string", dest="TestIniPath", callback=SingleCheckCallback,
        help="Test ini files path for ErrorInjection, only for ErrorInjection.")
    Parser.add_option("-r", "--report", action="callback", type="string", dest="ReportPath", callback=SingleCheckCallback,
        help="Generated code coverage report path.")

    (Opt, Args) = Parser.parse_args()
    return (Opt, Args)

def main():
    (Option, Target) = MyOptionParser()

    CheckTestEnv()

    if not Option.ModuleBin:
        print("Test module binary path should be set once by command -e MODULEBIN, --execbinary=MODULEBIN.")
        os._exit(0)
    elif not os.path.exists(Option.ModuleBin):
        print("Test module binary path: {} is no exits.".format(os.path.abspath(Option.ModuleBin)))
        os._exit(0)
    else:
        ModuleBinPath = Option.ModuleBin

    if "CLANG8" not in ModuleBinPath:
        if not Option.SeedPath:
            print("Test output seed directory path should be set once by command -d SEEDPATH, --dir=SEEDPATH.")
            os._exit(0)
        elif not os.path.exists(Option.SeedPath):
            print("Test output seed directory path:{} is not exists.".format(os.path.abspath(Option.SeedPath)))
            os._exit(0)
        else:
            OutputSeedPath = Option.SeedPath

        if not Option.TestIniPath:
            TestIniPath = ""
        elif not os.path.exists(Option.TestIniPath):
            print("Test ini path:{} is not exists.".format(os.path.abspath(Option.TestIniPath)))
            os._exit(0)
        elif not HasIni(Option.TestIniPath):
            print("No .ini file in {}".format(Option.TestIniPath))
            os._exit(0)
        else:
            TestIniPath = Option.TestIniPath

    if not Option.ReportPath:
        ReportPath = os.path.join(WORK_DIR, 'CodeCoverageReport')
    elif os.path.isabs(Option.ReportPath):
        ReportPath = os.path.join(Option.ReportPath, 'CodeCoverageReport')
    elif not os.path.isabs(Option.ReportPath):
        ReportPath = os.path.join(WORK_DIR, Option.ReportPath, 'CodeCoverageReport')
    else:
        print("Please check the input report path.")
        os._exit(0)

    IsPeach = CheckIsPeach(OutputSeedPath)
    if SysType == "Linux" and "CLANG8" not in ModuleBinPath and not IsPeach:
        # delete .gcda files before collect code coverage
        delete_gcda_file(ModuleBinPath)

    if "CLANG8" not in ModuleBinPath and not IsPeach:
        # Run binary with all seeds
        Run_All_Seeds(ModuleBinPath, OutputSeedPath, TestIniPath)

    # Generate Code coverage report
    GenCodeCoverage(ModuleBinPath, ReportPath)

if __name__ == "__main__":
    main()
