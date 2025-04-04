## @file
# Transfer report.log file into HTML file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import errno
import subprocess
from optparse import OptionParser
from GetSeedsList import *
from GenGdbInfo import *
from GenGdbHtmlReport import *
from GenSummaryInfo import *
from GenSanitizerInfo import *
from GenSanitizerHtmlReport import *
from GenSummaryReport import *
import psutil

__prog__ = 'ReportMain.py'
__copyright__ = 'Copyright (c) 2019, Intel Corporation. All rights reserved.'
__version__ = '{} Version {}'.format(__prog__, '0.1 ')

class GenerateFinalReport(object):
    def __init__(self, execute, input, output, methods, sleep):
        self.__execute = execute
        self.__input = input
        self.__output = output
        self.__methods = methods
        self.__sleep = sleep
        self.__reportType = self.__ReportType()

    def __ReportType(self):
        return "gdb" if "gcc5" in self.__execute.lower() else "sanitizer"

    def __CallCommand(self, CommandLine):
        Cm = subprocess.Popen(CommandLine,
                              stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE,
                              shell=True)
        msg = list(Cm.communicate())
        return msg if python_version == 2 else (msg[0].decode() if msg[0] else msg[0], msg[1].decode() if msg[1] else msg[1])

    def __CheckStatus(self):
        Pid = None
        if self.__methods.lower() == 'afl':
            statusFile = os.path.join(self.__input, 'fuzzer_stats')
            if os.path.exists(statusFile):
                f = open(statusFile, 'r')
                lines = f.readlines()
                f.close()
                for line in lines:
                    if 'fuzzer_pid' in line.lower():
                        tmpPid = line.split(":")[-1].strip()
                        if tmpPid.isdigit():
                            tmpPid = int(tmpPid)
                            p=psutil.Process(tmpPid)
                            status=p.status()
                            if ' process no longer exists'  not in status:
                                Pid = tmpPid
                            else:
                                Pid =None
                                print(status)
                        break

        elif self.__methods.lower() == 'peach':
            flag=False
            # TODO: Same Binary run in parallel
            statusFile = os.path.join(self.__input, 'status.txt')
            if os.path.exists(statusFile):
                f = open(statusFile, 'r')
                lines = f.readlines()
                f.close()
                for line in lines:
                    if 'command line:' in line.lower():
                        Commandline = ":".join(line.split(":")[1:]).strip()
                        break
                pids=psutil.pids()
                for pid in pids:
                    if psutil.pid_exists(pid) == True:
                        try:
                            p=psutil.Process(pid)
                            if p.name()=="Peach.exe" or p.name()=="peach":
                                if Commandline == p.cmdline()[-1]:
                                    flag=True
                                    break
                        except Exception as err:
                            print(err)
                            continue
                if flag:
                    sp=psutil.Process(pid)  
                    status=p.status()
                    if ' process no longer exists' not in status:
                        Pid = pid
                    else:
                        Pid =None
                        print(status)

        # TODO: Check libfuzzer status
        return Pid


    def __GenDebugReport(self, seedList, aflError):
        if self.__ReportType() == "gdb":
            cfgPath = os.path.join(self.__output, "HBFA.GDB{}.cfg".format(
                ("." + aflError.capitalize()) if aflError in ["crashes", "hangs"] else ""))
            genCfg = GenGdbInfo(self.__execute, cfgPath)
            genCfg.Run(seedList, False if aflError else True)
            genHtml = GenGdbHtmlReport(cfgPath, self.__output, aflError)
            genHtml.GenerateHtml()
        elif self.__ReportType() == 'sanitizer':
            if self.__methods.lower() != "peach+sanitizer":
                genCfg = GenSanitizerInfo(self.__execute, seedList, self.__output, True, aflError)
                genCfg.Run()
            genHtml = GenSanitizerHtmlReport(self.__output if self.__methods.lower() != "peach+sanitizer" else self.__input, aflError, self.__output)
            genHtml.GenHtml()

    def __GenSummaryReport(self):
        genSumNum = GenSummaryInfo(self.__input, self.__output, self.__methods, self.__reportType, True)
        totalNum, failNum, execTime = genSumNum.GenSumInfo()
        genSummary = GenSummaryReport(self.__execute, self.__output, self.__methods, self.__reportType, failNum, totalNum, execTime)
        genSummary.GenSumReport()

    def GenerateReport(self):
        getSeedsList = GetSeedsList(self.__methods, self.__input, False)
        while True:
            seedsList = getSeedsList.SeekNewSeed()
            if self.__methods.lower() == "afl":
                aflErrorType = ["crashes", "hangs"]
                try:
                    for index, subList in enumerate(seedsList):
                        if len(subList) == 0:
                            self.__GenDebugReport(subList, aflErrorType[index])
                        else:
                            for seed in subList:
                                self.__GenDebugReport([seed], aflErrorType[index])
                        self.__GenSummaryReport()
                except Exception as e:
                    print(e)
            elif "peach" in self.__methods.lower():
                try:
                    if len(seedsList) == 0:
                        self.__GenDebugReport(seedsList, "")
                    else:
                        for seed in seedsList:
                            self.__GenDebugReport([seed], "")
                    self.__GenSummaryReport()
                except Exception as e:
                    print(e)
            elif self.__methods.lower() == 'libfuzzer':
                try:
                    if len(seedsList) == 0:
                        self.__GenDebugReport(seedsList, "gdb")
                    else:
                        for seed in seedsList:
                            self.__GenDebugReport([seed], "gdb")
                    self.__GenSummaryReport()
                except Exception as e:
                    print(e)
            if self.__sleep:
                time.sleep(int(self.__sleep))
            else:
                break
            if not self.__CheckStatus():
                break


if __name__ == '__main__':
    parse = OptionParser(description=__copyright__, version=__version__, prog=__prog__, usage="python %prog [options][argument]")
    parse.add_option("-e", "--execbinary", dest="ModuleBin", help="Test module binary file name.")
    parse.add_option("-i", "--input", dest="ResultPath", help="Test result path for test method.", default=None)
    parse.add_option("-r", "--report", dest="ReportPath", help="Generated report path.", default=os.getcwd())
    parse.add_option("-t", "--testmethods", dest="TestMethods", help="Test method's name. Must be one of [afl, peach, libfuzzer]. Will be auto detected for default.", default=None)
    parse.add_option("-s", "--sleep", dest="SleepTime", help="In run time mode, # of seconds to sleep between checking for new seed files", default=None)

    options, args = parse.parse_args()
    if options.ResultPath:
        if options.TestMethods:
            if options.TestMethods.lower() == 'afl':
                if not os.path.exists(os.path.join(options.ResultPath, "fuzzer_stats")):
                    print("Input AFL fuzzing test's output directory: {} is illegal, please check the directory.".format(
                        os.path.abspath(options.ResultPath)))
                    os._exit(0)
            elif options.TestMethods.lower() == 'peach':
                if not os.path.exists(os.path.join(options.ResultPath, "status.txt")):
                    print("Input Peach's output directory: {} is illegal, please check the directory.".format(
                        os.path.abspath(options.ResultPath)))
                    os._exit(0)
            elif options.TestMethods.lower() == 'peach+sanitizer':
                if not os.path.exists(os.path.join(options.ResultPath, "fuzzfile")):
                    print("Input Peach + Sanitizer's output directory: {} is illegal, please check the directory.".format(
                        os.path.abspath(options.ResultPath)))
                    os._exit(0)
            elif options.TestMethods.lower() == 'libfuzzer':
                pass
        else:
            if os.path.exists(os.path.join(options.ResultPath, "fuzzer_stats")):
                options.TestMethods = 'afl'
            elif os.path.exists(os.path.join(options.ResultPath, "status.txt")):
                options.TestMethods = "peach"
            elif os.path.exists(os.path.join(options.ResultPath, "fuzzfile")):
                options.TestMethods = "peach+sanitizer"
            else:
                options.TestMethods = "libfuzzer"
        if options.ModuleBin:
            if not os.path.exists(options.ModuleBin):
                print("Input executable test binary file path: {} is not existed.".format(os.path.abspath(options.ModuleBin)))
                os._exit(0)
            if options.TestMethods == "peach+sanitizer" and "gcc5" in options.ModuleBin.lower():
                print("Input executable test binary file path: {} is not suit test methods peach+sanitizer.".format(
                    os.path.abspath(options.ModuleBin)))
                os._exit(0)

            ReportPath = os.path.join(options.ReportPath, "DebugReport")
            if os.path.exists(ReportPath):
                timeStamp = time.strftime('%Y%m%d%H%M%S', time.localtime(os.path.getctime(ReportPath)))
                backupReportPath = os.path.join(options.ReportPath, "{}_{}".format("DebugReport", timeStamp))
                os.rename(ReportPath, backupReportPath)
            if not os.path.exists(ReportPath):
                os.makedirs(ReportPath)
            gfr = GenerateFinalReport(options.ModuleBin, options.ResultPath, ReportPath, options.TestMethods, options.SleepTime)
            gfr.GenerateReport()
        else:
            print ("Please -e input path for executable test binary file.")
            os._exit(0)
    else:
        print ("Please -i input directory that generated by test methods.")
        os._exit(0)
