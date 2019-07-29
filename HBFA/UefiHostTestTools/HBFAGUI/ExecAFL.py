## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import time
import subprocess
import platform
import sys
import shutil

try:
    import ConfigParser as configparser
except Exception as e:
    import configparser as configparser
from plugins.GenReport import GenerateInfoAndHtml


class RunAFL(object):
    def __init__(self, path, seedspaths):
        self.HBFAGUI_Path = os.path.dirname(os.path.realpath(__file__))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.arch = self.conf.get('afl', 'arch').strip()
        self.buildTarget = self.conf.get('afl', 'BuildTarget').strip()
        self.OutputPath = self.conf.get('afl', 'OutputPath').strip()
        self.test_case = os.path.basename(path).split('.inf')[0].strip()
        self.path = path
        self.seedspaths = seedspaths
        self.test_case_relative_path = 'UefiHostFuzzTestCasePkg' + path.split('UefiHostFuzzTestCasePkg')[1]
        self.workspace = os.environ['WORKSPACE']
        self.run_afl_script_path = os.path.join(os.path.dirname(self.HBFAGUI_Path), 'RunAFL.py')
        self.SysType = platform.system()

    def __cleanup_seeds_output_path(self):
        for i in range(0, 3):
            if os.path.exists(os.path.join(self.OutputPath, "afl_%s") % self.test_case):
                if os.path.exists(os.path.join(self.OutputPath, 'afl_%s' % self.test_case, 'plot_data')):
                    content = open(os.path.join(self.OutputPath, 'afl_%s' % self.test_case, 'plot_data'), 'r')
                    lines = content.readlines()
                    content.close()
                    if len(lines) > 1:
                        line1 = lines[1]
                        time_start = int(line1.split(',')[0])
                        time_stamp = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(time_start))
                    else:
                        time_stamp = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(time.time()))
                else:
                    time_stamp = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(time.time()))
                os.rename(os.path.join(self.OutputPath, "afl_%s") % self.test_case,
                          os.path.join(self.OutputPath, "afl_%s_%s") % (self.test_case, time_stamp))
            else:
                break

        if not os.path.exists(os.path.join(self.OutputPath, "afl_%s") % self.test_case):
            os.makedirs(os.path.join(self.OutputPath, "afl_%s") % self.test_case)

    def __copy_seeds(self, outputpath):
        self.originSeedsPath = self.seedspaths
        self.inputSeedsPath = os.path.join(outputpath, 'Seeds')

        if os.path.exists(self.inputSeedsPath):
            shutil.rmtree(self.inputSeedsPath, ignore_errors=True)
        os.makedirs(self.inputSeedsPath)
        for path in self.originSeedsPath:
            shutil.copyfile(path.strip(),
                            os.path.join(self.inputSeedsPath, os.path.basename(path)))

    def __build_clangwin(self):
        ToolChain = 'CLANGWIN'
        HBFA_PATH = os.path.dirname(os.path.dirname(self.HBFAGUI_Path))
        Conf_Path = os.path.join(HBFA_PATH, 'UefiHostFuzzTestPkg', 'Conf')
        PkgName = self.test_case_relative_path.split(os.path.sep)[0]
        ModuleBinAbsPath = os.path.join(self.workspace, 'Build', PkgName, self.buildTarget + '_' + ToolChain, self.arch,
                                        self.test_case + '.exe')
        PlatformDsc = os.path.join(PkgName, PkgName + '.dsc')

        BuildCmdList = []
        BuildCmdList.append('-p {}'.format(PlatformDsc))
        BuildCmdList.append('-m {}'.format(self.test_case_relative_path))
        BuildCmdList.append('-a {}'.format(self.arch))
        BuildCmdList.append('-b {}'.format(self.buildTarget))
        BuildCmdList.append('-t {}'.format(ToolChain))
        BuildCmdList.append('--conf {}'.format(Conf_Path))

        BuildCmd = 'build ' + ' '.join(BuildCmdList)
        print ("Start build Test Module:")
        print (BuildCmd)
        proccess = subprocess.Popen(BuildCmd,
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
        if self.SysType == "Windows":
            self.__build_clangwin()
        self.__cleanup_seeds_output_path()
        outputpath = os.path.join(self.OutputPath, "afl_%s") % self.test_case
        self.__copy_seeds(outputpath)
        self.output_seeds_path = os.path.join(self.OutputPath, "afl_%s") % self.test_case
        self.input_seeds_path = os.path.join(self.output_seeds_path, "Seeds")
        command = "python %s -a %s -b %s -m %s -i %s -o %s" \
                  % (self.run_afl_script_path,
                     self.arch,
                     self.buildTarget,
                     self.test_case_relative_path,
                     self.input_seeds_path,
                     self.output_seeds_path)
        os.system(command)
        time.sleep(4)
        for _ in range(0, 5):
            if os.path.exists(os.path.join(self.output_seeds_path, 'fuzzer_stats')):
                generateInfo = GenerateInfoAndHtml(self.path, ['AFL'])
                generateInfo.generate_report()
                break
            else:
                time.sleep(1)


