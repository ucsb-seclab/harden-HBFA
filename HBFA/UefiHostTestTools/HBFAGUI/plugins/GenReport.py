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
import webbrowser as web


class GenerateInfoAndHtml(object):

    def __init__(self, casepath, methods):
        self.HBFAGUI_Path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.sections = self.conf.sections()
        self.methodsList = methods
        self.case = os.path.basename(casepath).split('.inf')[0].strip()
        self.casepath = casepath
        self.workspace = os.environ['WORKSPACE']
        self.script_path = os.path.join(os.path.dirname(self.HBFAGUI_Path), 'Report')
        self.gen_report_script_name = "ReportMain.py"
        self.SysType = platform.system()

    def generate_report(self):
        for m in self.methodsList:
            self.SeedsOutputPath = self.conf.get(m.lower(), 'OutputPath').strip()
            self.arch = self.conf.get(m.lower(), 'arch').strip()
            self.BuildTarget = self.conf.get(m.lower(), 'BuildTarget').strip()
            self.gcc_execute = os.path.join(self.workspace, 'Build', 'UefiHostFuzzTestCasePkg',
                                            '%s_GCC5' % self.BuildTarget.strip(), self.arch, self.case)
            self.clangwin_execute = os.path.join(self.workspace, 'Build', 'UefiHostFuzzTestCasePkg',
                                                 '%s_CLANGWIN' % self.BuildTarget.strip(), self.arch,
                                                 self.case + '.exe')
            self.clang_execute = os.path.join(self.workspace, 'Build', 'UefiHostFuzzTestCasePkg',
                                              '%s_CLANG8' % self.BuildTarget.strip(), self.arch, self.case)
            if m in ['AFL', 'Peach']:
                self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case),
                                                     'GdbReport' if self.SysType == "Linux" else 'SanitizerReport')
                if m == "AFL":
                    self.inputpath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case))
                elif m == "Peach":
                    for root, dirs, files in os.walk(
                            os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case))):
                        if 'status.txt' in files:
                            self.inputpath = root
                command = "python " + \
                          os.path.join(self.script_path, self.gen_report_script_name) + \
                          " -i %s -e %s -r %s -s 1" \
                          % (self.inputpath, self.gcc_execute if self.SysType == "Linux" else self.clangwin_execute,
                             self.reportOutputPath)
                print(command)
                try:
                    out = open(os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case), 'report_debug.log'),
                               'w')
                    popen = subprocess.Popen(command, stdout=out.fileno(), stderr=subprocess.STDOUT, shell=True)
                except Exception as e:
                    print('generate report failed:%s' % e)
                finally:
                    out.flush()
                    out.close()

            else:
                if m == "Peach+Sanitizer":
                    self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case),
                                                         'SanitizerReport')
                    self.inputpath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case))
                elif m == "libFuzzer":
                    self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case),
                                                         'SanitizerReport')
                    self.inputpath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case),
                                                  'failureSeeds')
                    print(self.reportOutputPath, self.inputpath)
                command = "python " + \
                          os.path.join(self.script_path, self.gen_report_script_name) + \
                          " -i %s -e %s -r %s" \
                          % (self.inputpath, self.clang_execute if self.SysType == "Linux" else self.clangwin_execute,
                             self.reportOutputPath)
                print(command)
                try:
                    os.system(command)
                except Exception as e:
                    print('generate report failed:%s' % e)


class GenerateSummaryInfo(object):
    def __init__(self, path, method):
        self.HBFAGUI_Path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.method = method
        self.case = os.path.basename(path).split('.inf')[0].strip()
        self.script_path = os.path.join(os.path.dirname(self.HBFAGUI_Path), 'Report')
        self.gen_sumInfo_script_path = "GenSummaryInfo.py"
        self.SysType = platform.system()

    def genSumInfo(self):
        if self.method == "AFL":
            self.SeedsOutputPath = self.conf.get('afl', 'OutputPath').strip()
            self.inputpath = os.path.join(self.SeedsOutputPath, '%s_%s' % (self.method.lower(), self.case))
            self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (self.method.lower(), self.case),
                                                 'GdbReport' if self.SysType == "Linux" else 'SanitizerReport',
                                                 'DebugReport')
        elif self.method == "Peach":
            self.SeedsOutputPath = self.conf.get('peach', 'OutputPath').strip()
            self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (self.method.lower(), self.case),
                                                 'GdbReport' if self.SysType == "Linux" else 'SanitizerReport',
                                                 'DebugReport')
            for root, dirs, files in os.walk(
                    os.path.join(self.SeedsOutputPath, '%s_%s' % (self.method.lower(), self.case))):
                if 'status.txt' in files:
                    self.inputpath = root
        elif self.method == "libFuzzer":
            self.SeedsOutputPath = self.conf.get('libfuzzer', 'OutputPath').strip()
            self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (self.method.lower(), self.case),
                                                 'SanitizerReport', 'DebugReport')
            self.inputpath = os.path.join(self.SeedsOutputPath, '%s_%s' % (self.method.lower(), self.case),
                                          'failureSeeds')
        elif self.method == "Peach+Sanitizer":
            self.SeedsOutputPath = self.conf.get('peach+sanitizer', 'OutputPath').strip()
            self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (self.method.lower(), self.case),
                                                 'SanitizerReport', 'DebugReport')
            self.inputpath = os.path.join(self.SeedsOutputPath, '%s_%s' % (self.method.lower(), self.case))

        try:
            command = "python %s -i %s -o %s -m %s -t %s" \
                      % (os.path.join(self.script_path, self.gen_sumInfo_script_path), \
                         self.inputpath, \
                         self.reportOutputPath, \
                         self.method.lower(), \
                         'gdb' if ((
                                               self.method == "AFL" or self.method == 'Peach') and self.SysType == "Linux") else 'sanitizer')
            print(command)
            ret = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
            sumInfo = ret.communicate()[0]
            sumInfo = sumInfo.decode() if sys.version_info[0] == 3 else sumInfo
            sumInfoList = sumInfo.split(';')
            totalNum, failNum, execTime = sumInfoList[0].strip(), sumInfoList[1].strip(), sumInfoList[2].strip()
            return totalNum, failNum, execTime
        except Exception as e:
            print("ERROR: Get report sum info failed:%s" % e)


class GenerateSummaryReport(object):
    def __init__(self, path, methods):
        self.HBFAGUI_Path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.methodsList = methods
        self.case = os.path.basename(path).split('.inf')[0].strip()
        self.SysType = platform.system()

    def open_summary_report(self):
        for m in self.methodsList:
            if m == 'AFL':
                self.SeedsOutputPath = self.conf.get('afl', 'OutputPath').strip()
                self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case),
                                                     'GdbReport' if self.SysType == "Linux" else 'SanitizerReport')
                web.open_new(os.path.join(self.reportOutputPath, 'DebugReport',
                                          "GdbSummaryReport.html" if self.SysType == "Linux" else "SanitizerSummaryReport.html"))
            elif m == 'Peach':
                self.SeedsOutputPath = self.conf.get('peach', 'OutputPath').strip()
                self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case),
                                                     'GdbReport' if self.SysType == "Linux" else 'SanitizerReport')
                web.open_new(os.path.join(self.reportOutputPath, 'DebugReport',
                                          "GdbSummaryReport.html" if self.SysType == "Linux" else "SanitizerSummaryReport.html"))
            elif m == 'Peach+Sanitizer':
                self.SeedsOutputPath = self.conf.get('peach+sanitizer', 'OutputPath').strip()
                self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case),
                                                     'SanitizerReport')
                web.open_new(os.path.join(self.reportOutputPath, 'DebugReport', "SanitizerSummaryReport.html"))
            elif m == 'libFuzzer':
                self.SeedsOutputPath = self.conf.get('libfuzzer', 'OutputPath').strip()
                self.reportOutputPath = os.path.join(self.SeedsOutputPath, '%s_%s' % (m.lower(), self.case),
                                                     'SanitizerReport')
                web.open_new(os.path.join(self.reportOutputPath, 'DebugReport', "SanitizerSummaryReport.html"))
            else:
                print ('report not supported')       
