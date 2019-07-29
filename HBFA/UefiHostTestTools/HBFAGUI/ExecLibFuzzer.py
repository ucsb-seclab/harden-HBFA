## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import sys
import time
import platform
import subprocess
import shutil

try:
    import ConfigParser as configparser
except Exception as e:
    import configparser as configparser


class RunLibFuzzer(object):
    def __init__(self, path, seedspaths=[], select_seeds=False):
        self.HBFAGUI_Path = os.path.dirname(os.path.realpath(__file__))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.arch = self.conf.get('libfuzzer', 'arch').strip()
        self.buildTarget = self.conf.get('libfuzzer', 'BuildTarget').strip()
        self.outputPath = self.conf.get('libfuzzer', 'OutputPath').strip()
        self.test_case_relative_path = 'UefiHostFuzzTestCasePkg' + path.split('UefiHostFuzzTestCasePkg')[1].strip()
        self.run_LibFuzzer_script_path = os.path.join(os.path.dirname(self.HBFAGUI_Path), 'RunLibFuzzer.py')
        self.workspace = os.environ['WORKSPACE']
        self.test_case = os.path.basename(path).split('.inf')[0]
        self.seedspaths = seedspaths
        self.select_seeds = select_seeds
        self.SysType = platform.system()

    def __cleanup_libFuzzer_output_path(self):
        time_stamp = time.localtime(time.time())
        time_stamp = str(time.strftime("%Y_%m_%d_%H_%M_%S", time_stamp))
        for _ in range(0, 3):
            if os.path.exists(os.path.join(self.outputPath, 'libfuzzer_%s' % self.test_case)):
                os.rename(os.path.join(self.outputPath, 'libfuzzer_%s' % self.test_case),
                          os.path.join(self.outputPath, 'libfuzzer_%s_%s' % (self.test_case, time_stamp)))
            else:
                break
        if not os.path.exists(os.path.join(self.outputPath, "libfuzzer_%s") % self.test_case):
            os.makedirs(os.path.join(self.outputPath, "libfuzzer_%s") % self.test_case)

    def __copy_seeds(self,outputpath):
        self.originSeedsPath = self.seedspaths
        self.inputSeedsPath = os.path.join(outputpath, 'Seeds')

        if os.path.exists(self.inputSeedsPath):
            shutil.rmtree(self.inputSeedsPath, ignore_errors=True)
        os.makedirs(self.inputSeedsPath)
        for path in self.originSeedsPath:
            shutil.copyfile(path.strip(),
                            os.path.join(self.inputSeedsPath, os.path.basename(path)))

    def __build_clang(self):
        if self.SysType == "Windows":
            ToolChain = 'CLANGWIN'
        elif self.SysType == "Linux":
            ToolChain = 'CLANG8'
        HBFA_PATH = os.path.dirname(os.path.dirname(self.HBFAGUI_Path))
        Conf_Path = os.path.join(HBFA_PATH, 'UefiHostFuzzTestPkg', 'Conf')
        PkgName = self.test_case_relative_path.split(os.path.sep)[0]
        if self.SysType == "Windows":
            ModuleBinAbsPath = os.path.join(self.workspace, 'Build', PkgName, self.buildTarget + '_' + ToolChain,
                                            self.arch, self.test_case + '.exe')
        elif self.SysType == "Linux":
            ModuleBinAbsPath = os.path.join(self.workspace, 'Build', PkgName, self.buildTarget + '_' + ToolChain,
                                            self.arch, self.test_case)
        PlatformDsc = os.path.join(PkgName, PkgName + '.dsc')

        BuildCmdList = []
        BuildCmdList.append('-p {}'.format(PlatformDsc))
        BuildCmdList.append('-m {}'.format(self.test_case_relative_path))
        BuildCmdList.append('-a {}'.format(self.arch))
        BuildCmdList.append('-b {}'.format(self.buildTarget))
        BuildCmdList.append('-t {}'.format(ToolChain))
        BuildCmdList.append('--conf {}'.format(Conf_Path))
        if self.SysType == "Linux":
            BuildCmdList.append('-t GCC5')

        BuildCmd = 'build ' + ' '.join(BuildCmdList)
        if self.SysType == "Windows":
            ExecCmd = BuildCmd
        elif self.SysType == "Linux":
            SetEnvCmd = "export PATH=$CLANG_PATH:$PATH\n"
            ExecCmd = SetEnvCmd + BuildCmd
        print("Start build Test Module:")
        print(BuildCmd)
        proccess = subprocess.Popen(ExecCmd,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT,
                                    shell=True)
        msg = list(proccess.communicate())
        if sys.version_info[0] == 3:
            for num, submsg in enumerate(msg):
                if submsg is not None:
                    msg[num] = submsg.decode()
        if msg[1]:
            print (msg[0] + msg[1])
        elif "- Done -" not in msg[0]:
            print (msg[0])
        else:
            pass
        if os.path.exists(ModuleBinAbsPath):
            print ("Build Successfully !!!\n")
            return True
        else:
            print ("Build failure, can not find Module Binary file: {}".format(ModuleBinAbsPath))
            return False

    def run(self):
        if self.SysType == 'Linux':
            Is32bit = bool("64" not in platform.machine())
            if (self.arch == "IA32") ^ Is32bit:
                messages = ("For CLANG: {} system cannot support arch {} build.".format("32bit" if Is32bit else "64bit",
                                                                                        self.arch))
                print (messages)
                return messages
        if self.SysType == "Windows":
            if self.arch == "IA32":
                # Need to remove when LLVM support libFuzzer i386
                print("LLVM doesn't support libFuzzer i386 currently.")
                return "LLVM doesn't support libFuzzer i386 currently."
        self.__cleanup_libFuzzer_output_path()
        outputpath = os.path.join(self.outputPath, "libfuzzer_%s") % self.test_case
        if self.select_seeds is True:
            self.__copy_seeds(outputpath)
        else:
            os.makedirs(os.path.join(outputpath, 'Seeds'))
        self.input_seeds_path = os.path.join(outputpath, "Seeds") 

        self.__build_clang()
        self.output_seeds_path = os.path.join(self.outputPath, "libfuzzer_%s" % self.test_case, 'failureSeeds')
        command = 'python %s -a %s -b %s -m %s -i %s -o %s' \
                  % (self.run_LibFuzzer_script_path,
                     self.arch,
                     self.buildTarget,
                     self.test_case_relative_path,
                     self.input_seeds_path,
                     self.output_seeds_path)

        os.system(command)