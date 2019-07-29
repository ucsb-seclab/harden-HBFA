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


class RunKLEE(object):
    def __init__(self, path):
        self.HBFAGUI_Path = os.path.dirname(os.path.realpath(__file__))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.arch = self.conf.get('klee(stp)', 'arch').strip()
        self.buildTarget = self.conf.get('klee(stp)', 'BuildTarget').strip()
        self.outputPath = self.conf.get('klee(stp)', 'OutputPath').strip()
        self.test_case_relative_path = 'UefiHostFuzzTestCasePkg' + path.split('UefiHostFuzzTestCasePkg')[1].strip()
        self.test_case = os.path.basename(path).split('.inf')[0].strip()
        self.run_klee_script_path = os.path.join(os.path.dirname(self.HBFAGUI_Path), 'RunKLEE.py')
        self.klee_path = os.path.join(self.outputPath, "klee_%s"%self.test_case)
        self.klee_seeds_path = os.path.join(self.klee_path, 'GeneratedSeeds')
        self.change_ktest_to_seed_script_path = os.path.join(
            os.path.dirname(self.HBFAGUI_Path), 'Script', 'TransferKtestToSeed.py')
        self.sysType = platform.system()

    def make_klee_seeds_dirs(self):
        time_stamp = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(time.time()))
        if os.path.exists(self.klee_path):
            if os.path.exists(os.path.join(self.klee_path, 'GeneratedSeeds', "info")):
                file = open(os.path.join(self.klee_path, 'GeneratedSeeds', "info"), 'r')
                lines = file.readlines()
                file.close()
                for line in lines:
                    if 'Started' in line:
                        time_stamp = line.strip().split('Started')[1].strip(':').strip()
                        break
            os.rename(self.klee_path, self.klee_path + '_' + time_stamp)

    def run(self):
        if self.sysType == "Windows":
            message = "KLEE(STP) is not supported on windows now."
            print (message)
            return message
        self.make_klee_seeds_dirs()
        command = "python %s -a %s -b %s -m %s -o %s" % (
            self.run_klee_script_path, self.arch, self.buildTarget, self.test_case_relative_path, self.klee_seeds_path)
        os.system(command)

    def change_ktest_to_seed(self):
        if os.path.exists(self.klee_seeds_path):
            command = 'python %s %s' % (self.change_ktest_to_seed_script_path, self.klee_seeds_path)
            ret = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
            out = list(ret.communicate())
            if sys.version_info[0] == 3:
                for num, submsg in enumerate(out):
                    if submsg is not None:
                        out[num] = submsg.decode()
            if out[1]:
                print ('change ktests to seeds failed' + out[1])
