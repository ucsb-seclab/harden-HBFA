## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import platform
try:
    import ConfigParser as configparser
except Exception as e:
    import configparser as configparser
import webbrowser as web

class GetCodeCoverage(object):
    def __init__(self, path, methods):
        self.HBFAGUI_Path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.workspace = os.environ['WORKSPACE']
        self.methodList = methods
        self.test_case = os.path.basename(path).split('.inf')[0].strip()
        self.run_afl_script_path = os.path.join(os.path.dirname(self.HBFAGUI_Path), 'Report', 'GenCodeCoverage.py')
        self.sysType = platform.system()

    def run(self):
        for m in self.methodList:
            self.arch = self.conf.get(m.lower(), 'arch')
            self.buildTarget = self.conf.get(m.lower(), 'BuildTarget').strip()
            self.OutputPath = self.conf.get(m.lower(), 'OutputPath')
            self.gcc5_binary_path = os.path.join(self.workspace, 'Build', 'UefiHostFuzzTestCasePkg',
                                                 '%s_GCC5' % (self.buildTarget), self.arch, self.test_case)
            self.clang8_binary_path = os.path.join(self.workspace, 'Build', 'UefiHostFuzzTestCasePkg',
                                                   '%s_CLANG8' % (self.buildTarget), self.arch, self.test_case)
            self.vs_binary_path = os.path.join(self.workspace, 'Build', 'UefiHostFuzzTestCasePkg',
                                               '%s_VS2015x86' % (self.buildTarget), self.arch, self.test_case + '.exe')
            if m.lower() == 'afl':
                if self.sysType == 'Linux':
                    self.binary_path = self.gcc5_binary_path
                else:
                    self.binary_path = self.vs_binary_path
                self.seeds_path = os.path.join(self.OutputPath, '%s_%s' % (m.lower(), self.test_case), 'queue')
            elif m.lower() == 'peach':
                if self.sysType == 'Windows':
                    return 'Peach codeCoverage is not supported on windows now.'
                self.binary_path = self.gcc5_binary_path
                self.seeds_path = os.path.join(self.OutputPath, '%s_%s' % (m.lower(), self.test_case))
            elif m.lower() == 'peach+sanitizer':
                if self.sysType == 'Windows':
                    return '%s codeCoverage is not supported on windows now.' % m.lower()
                self.binary_path = self.clang8_binary_path
                self.seeds_path = os.path.join(self.OutputPath, '%s_%s' % (m.lower(), self.test_case))
            elif m.lower() == 'libfuzzer':
                if self.sysType == 'Windows':
                    return '%s codeCoverage is not supported on windows now.' % m.lower()
                self.binary_path = self.gcc5_binary_path
                self.seeds_path = os.path.join(self.OutputPath, '%s_%s' % (m.lower(), self.test_case),'Seeds')           
            else:
                return '%s codeCoverage is not supported now.' % m.lower()
            self.report_path = os.path.join(self.OutputPath, '%s_%s' % (m.lower(), self.test_case))
            command = "python %s -e %s -d %s -r %s" \
                      % (self.run_afl_script_path,
                         self.binary_path,
                         self.seeds_path,
                         self.report_path)
            os.system(command)
            web.open_new(os.path.join(self.report_path, 'CodeCoverageReport', "index.html"))