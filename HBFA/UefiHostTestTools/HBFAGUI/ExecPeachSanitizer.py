## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import time
import sys
import platform
import subprocess
try:
    import ConfigParser as configparser
except Exception as e:
    import configparser as configparser

class RunPeachSanitizer(object):
    def __init__(self,path):
        self.HBFAGUI_Path = os.path.dirname(os.path.realpath(__file__))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.SysType = platform.system()
        self.arch = self.conf.get('peach+sanitizer', 'arch').strip()
        self.buildTarget = self.conf.get('peach+sanitizer', 'BuildTarget').strip()
        self.test_case = os.path.basename(path).split('.inf')[0].strip()
        self.outputPath = self.conf.get('peach+sanitizer', 'OutputPath').strip()
        self.test_case_relative_path = 'UefiHostFuzzTestCasePkg' + path.split('UefiHostFuzzTestCasePkg')[1].strip()
        self.run_PeachSanitizer_script_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'RunPeach.py')
        self.GenSanitizerInfoBin_path = os.path.dirname(os.path.dirname(__file__))
        self.GenSanitizerInfoBin_name = 'GenSanitizerInfo' if self.SysType == "Linux" else 'GenSanitizerInfo.exe'
        self.PackGenSanitizerInfo_script_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'Script',
                                                             'PackPyToBin.py')

    def __cleanup_peachSanitizer_output_path(self):
        time_stamp = time.time()
        for _ in range(0, 3):
            if os.path.exists(os.path.join(self.outputPath, 'peach+sanitizer_%s' % self.test_case)):
                for root, dirs, files in os.walk(os.path.join(self.outputPath, 'peach+sanitizer_%s' % self.test_case)):
                    for dir in dirs:
                        if '.xml' in dir:
                            time_stamp = dir.split('_')[-1]
                os.rename(os.path.join(self.outputPath, 'peach+sanitizer_%s' % self.test_case),
                          os.path.join(self.outputPath, 'peach+sanitizer_%s_%s' % (self.test_case, time_stamp)))
            else:
                break
        if not os.path.exists(os.path.join(self.outputPath, "peach+sanitizer_%s") % self.test_case):
            os.makedirs(os.path.join(self.outputPath, "peach+sanitizer_%s") % self.test_case)

    def __check_env(self):
        if not os.path.exists(os.path.join(self.GenSanitizerInfoBin_path, self.GenSanitizerInfoBin_name)):
            command = 'python %s' % self.PackGenSanitizerInfo_script_path
            proccess = subprocess.Popen(command,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.STDOUT,
                                        shell=True)
            msg = list(proccess.communicate())
            if sys.version_info[0] == 3:
                for num, submsg in enumerate(msg):
                    if submsg is not None:
                        msg[num] = submsg.decode()
            if msg[1]:
                print(msg[0] + msg[1])
                return False, msg[0]
            elif 'ERROR' in msg[0]:
                print(msg[0])
                return False, msg[0]
            else:
                print('GenSanitizerInfoBin exists')
                return True, 'Success'
        else:
            return True, 'Success'

    def run(self):
        if self.SysType == 'Linux':
            Is32bit = bool("64" not in platform.machine())
            if (self.arch == "IA32") ^ Is32bit:
                messages = ("For CLANG: {} system cannot support arch {} build.".format("32bit" if Is32bit else "64bit",
                                                                                        self.arch))
                print (messages)
                return messages
        ret, msg = self.__check_env()
        if ret:
            self.__cleanup_peachSanitizer_output_path()
            self.output_seeds_path = os.path.join(self.outputPath, "peach+sanitizer_%s") % self.test_case
            command = 'python %s -a %s -b %s -m %s -o %s --enablesanitizer True' % \
                      (self.run_PeachSanitizer_script_path,
                       self.arch,
                       self.buildTarget,
                       self.test_case_relative_path,
                       self.output_seeds_path)
            os.system(command)
            return 'Success'
        else:
            return msg
