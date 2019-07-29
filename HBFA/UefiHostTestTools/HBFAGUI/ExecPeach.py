## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import sys
import time
import subprocess
import platform

try:
    import ConfigParser as configparser
except Exception as e:
    import configparser as configparser
from plugins.GenReport import GenerateInfoAndHtml


class RunPeach(object):
    def __init__(self, path):
        self.HBFAGUI_Path = os.path.dirname(os.path.realpath(__file__))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.arch = self.conf.get('peach', 'arch').strip()
        self.buildTarget = self.conf.get('peach', 'BuildTarget').strip()
        self.test_case = os.path.basename(path).split('.inf')[0].strip()
        self.path = path
        self.workspace = os.environ['WORKSPACE']
        self.outputPath = self.conf.get('peach', 'OutputPath').strip()
        self.test_case_relative_path = 'UefiHostFuzzTestCasePkg' + path.split('UefiHostFuzzTestCasePkg')[1].strip()
        self.run_PeachSanitizer_script_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'RunPeach.py')
        self.SysType = platform.system()

    def __cleanup_peach_output_path(self):
        time_stamp = time.time()
        for _ in range(0, 3):
            if os.path.exists(os.path.join(self.outputPath, 'peach_%s' % self.test_case)):
                for root, dirs, files in os.walk(os.path.join(self.outputPath, 'peach_%s' % self.test_case)):
                    for dir in dirs:
                        if '.xml' in dir:
                            time_stamp = dir.split('_')[-1]
                os.rename(os.path.join(self.outputPath, 'peach_%s' % self.test_case),
                          os.path.join(self.outputPath, 'peach_%s_%s' % (self.test_case, time_stamp)))
            else:
                break
        if not os.path.exists(os.path.join(self.outputPath, "peach_%s") % self.test_case):
            os.makedirs(os.path.join(self.outputPath, "peach_%s") % self.test_case)

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
        self.__cleanup_peach_output_path()
        self.output_seeds_path = os.path.join(self.outputPath, "peach_%s") % self.test_case
        command = 'python %s -a %s -b %s -m %s -o %s' \
                  % (self.run_PeachSanitizer_script_path,
                     self.arch,
                     self.buildTarget,
                     self.test_case_relative_path,
                     self.output_seeds_path)
        os.system(command)
        for _ in range(0, 5):
            if os.path.exists(os.path.join(self.output_seeds_path, 'logtest')):
                generateInfo = GenerateInfoAndHtml(self.path, ['Peach'])
                generateInfo.generate_report()
                break
            else:
                time.sleep(1)