## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

import os
import platform

workspace_path = os.path.dirname(os.path.realpath(__file__))

import wx
from ExecAFL import RunAFL
from ExecKLEE import RunKLEE
from ExecPeach import RunPeach
from ExecPeachSanitizer import RunPeachSanitizer
from ExecLibFuzzer import RunLibFuzzer
from plugins.GenReport import GenerateInfoAndHtml
from plugins.GenReport import GenerateSummaryInfo
from plugins.GenReport import GenerateSummaryReport
import wx.lib.agw.hyperlink as hl
from plugins.GetTestCase import GetTestCase
from plugins.GetCodeCoverage import GetCodeCoverage
from wx import DefaultSize

try:
    import ConfigParser as configparser
except Exception as e:
    import configparser as configparser


class my_frame(wx.Frame):
    """a new class of Frame"""

    def __init__(self, parent, title):
        self.SysType = platform.system()
        wx.Frame.__init__(self, parent, title=title, size=(840, 650))
        self.panel = wx.Panel(self, -1)
        self.panel.SetBackgroundColour('#DCDCDC')
        self.panel1 = wx.Panel(self.panel, -1, pos=(0, 400), size=(840, 400))
        self.panel1.SetBackgroundColour('#DCDCDC')
        self.panel2 = wx.Panel(self.panel, -1, pos=(450, 155), size=(380, 200))
        self.panel2.SetBackgroundColour('#DCDCDC')
        self.sizer_all = wx.BoxSizer(wx.VERTICAL)
        self.sizer = wx.GridBagSizer(0, 0)

        self.btn_import = wx.Button(self.panel, -1, 'Import')
        self.Bind(wx.EVT_BUTTON, self.OnClick_IMPORT, self.btn_import)
        self.btn_import.SetBackgroundColour("#6CA6CD")
        self.btn_import.SetForegroundColour("white")

        self.btn_method = wx.Button(self.panel, -1, 'Method')
        self.Bind(wx.EVT_BUTTON, self.OnClick_METHOD, self.btn_method)
        self.btn_method.SetBackgroundColour("#6CA6CD")
        self.btn_method.SetForegroundColour("white")

        self.btn_seed = wx.Button(self.panel, -1, 'Seed')
        self.Bind(wx.EVT_BUTTON, self.OnClick_SEED, self.btn_seed)
        self.btn_seed.SetBackgroundColour("#6CA6CD")
        self.btn_seed.SetForegroundColour("white")

        self.btn_exec = wx.Button(self.panel, -1, 'Exec')
        self.Bind(wx.EVT_BUTTON, self.OnClick_EXEC, self.btn_exec)
        self.btn_exec.SetBackgroundColour("#6CA6CD")
        self.btn_exec.SetForegroundColour("white")

        self.btn_report = wx.Button(self.panel, -1, 'Report')
        self.Bind(wx.EVT_BUTTON, self.OnClick_REPORT, self.btn_report)
        self.btn_report.SetBackgroundColour("#6CA6CD")
        self.btn_report.SetForegroundColour("white")

        self.btn_code_coverage = wx.Button(self.panel, -1, 'CodeCoverage')
        self.Bind(wx.EVT_BUTTON, self.OnClick_CODE_COVERAGE, self.btn_code_coverage)
        self.btn_code_coverage.SetBackgroundColour("#6CA6CD")
        self.btn_code_coverage.SetForegroundColour("white")

        self.sizer1 = wx.GridBagSizer(0, 0)
        self.title = wx.StaticText(self.panel, -1, "Host-based Firmware Analyzer", wx.DefaultPosition, wx.DefaultSize,
                                   style=wx.TE_MULTILINE | wx.TE_CENTER)
        self.title.SetForegroundColour("#6CA6CD")
        font = wx.Font(18, wx.DEFAULT, wx.NORMAL, wx.NORMAL)
        self.title.SetFont(font)

        self.sizer1.Add(self.title, pos=(1, 22), flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
        self.sizer.Add(self.btn_import, pos=(1, 2), flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
        self.sizer.Add(self.btn_method, pos=(1, 7), flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
        self.sizer.Add(self.btn_seed, pos=(1, 12), flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
        self.sizer.Add(self.btn_exec, pos=(1, 17), flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
        self.sizer.Add(self.btn_report, pos=(1, 22), flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
        self.sizer.Add(self.btn_code_coverage, pos=(1, 27), flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
        self.sizer_all.Add(self.sizer1, 0, flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=1)
        self.sizer_all.Add(self.sizer)
        self.panel.SetSizer(self.sizer_all)

        self.import_path_list = []
        self.method_list = ['KLEE(STP)', 'AFL', 'libFuzzer', 'Sanitizer', 'Peach']
        self.testcase_text = 0
        self.testmethod_text = 0
        self.seed_text = 0
        self.click_seed_count = 0
        self.select_seeds = False
        self.chang_cases = False
        self.filespaths = []
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(workspace_path, 'Env.conf')
        self.conf.read(self.conf_path)

    def OnClick_IMPORT(self, event):
        self.panel1.Hide()
        self.panel2.Hide()
        self.import_list = []
        GetTestCaseName = GetTestCase()
        self.import_path_list = GetTestCaseName.get_test_case_name_path()
        for path in self.import_path_list:
            self.case = os.path.basename(path).split('.')[0]
            self.import_list.append(self.case)
        if self.testcase_text == 0:
            self.testcase_text = wx.StaticText(self.panel, -1, 'Test Case:', (30, 165), DefaultSize,
                                               style=wx.TE_MULTILINE | wx.TE_LEFT)
        self.import_listbox = wx.ListBox(self.panel, -1, (30, 185), (200, 150), self.import_list, wx.LB_HSCROLL)
        self.import_listbox.SetSelection(2)
        self.import_listbox.SetBackgroundColour('#6CA6CD')
        self.import_listbox.SetForegroundColour("white")
        self.import_item = self.import_listbox.GetSelection()
        print(self.import_list[self.import_item], 'is clicked from Import Box')
        self.chang_cases = True
        self.import_listbox.Bind(wx.EVT_LISTBOX, self.import_onChecked)

    def import_onChecked(self, event):
        self.panel1.Hide()
        self.panel2.Hide()
        self.import_item = self.import_listbox.GetSelection()
        self.chang_cases = True
        print(self.import_list[self.import_item], 'is clicked from Import Box')

    def OnClick_METHOD(self, event):
        self.panel1.Hide()
        self.panel2.Hide()
        if self.testmethod_text == 0:
            self.testmethod_text = wx.StaticText(self.panel, -1, 'Test Method:', (260, 165), DefaultSize)
        self.checkbox = wx.CheckListBox(self.panel, -1, (260, 185), (200, 150), self.method_list)
        self.checkbox.Check(1, check=True)
        self.methods = self.checkbox.GetCheckedStrings()
        print(' '.join(self.methods), 'is clicked from Method Box')
        self.checkbox.SetBackgroundColour('#6CA6CD')
        self.checkbox.SetForegroundColour("white")
        self.checkbox.Bind(wx.EVT_CHECKLISTBOX, self.onChecked)

    def onChecked(self, e):
        self.panel1.Hide()
        self.panel2.Hide()
        self.methods = list(self.checkbox.GetCheckedStrings())
        if 'Sanitizer' in self.methods and 'Peach' in self.methods:
            self.methods.remove('Peach')
            self.methods.remove('Sanitizer')
            self.methods.insert(-1, 'Peach+Sanitizer')
            print('{} are clicked from TEST METHOD BOX'.format(' '.join(self.methods)))
        else:
            print('{} is clicked from TEST METHOD BOX'.format(','.join(self.methods) if len(self.methods) > 0 else None))

    def OnClick_SEED(self, event):
        self.click_seed_count = self.click_seed_count + 1
        self.panel2.Show(show=True)
        if 'KLEE(STP)' in self.methods:
            RunKLEE(self.import_path_list[self.import_item]).change_ktest_to_seed()
            wx.MessageBox('Klee generated seeds has been transfered in output path.Please use test method:AFL to run these seeds.', 'info', wx.OK | wx.ICON_INFORMATION)
            return
        self.seed_paths = wx.TextCtrl(self.panel2, -1, '', (40, 30), size=(320, 150), style=wx.TE_MULTILINE)
        self.seed_paths.SetBackgroundColour('#6CA6CD')
        self.seed_paths.SetForegroundColour("white")
        dlg = wx.FileDialog(self, 'please choose seeds', style=wx.FD_MULTIPLE)
        if dlg.ShowModal() == wx.ID_OK:
            self.filespaths = []
            for filename in dlg.GetFilenames():
                filepath = os.path.join(dlg.GetDirectory(), filename)
                self.filespaths.append(filepath)
            filespaths = '\n'.join(self.filespaths)
            self.seed_paths.AppendText(filespaths)
        if self.click_seed_count == 1:
            self.seeds = wx.StaticText(self.panel2, wx.ID_ANY, "Seeds:", (40, 10), wx.DefaultSize, 10)
        self.select_seeds = True

    def OnClick_EXEC(self, event):
        for m in self.methods:
            if m == 'AFL':
                self.panel1.Hide()
                if self.select_seeds or (not self.select_seeds and not self.chang_cases):
                    runAFl = RunAFL(self.import_path_list[self.import_item], self.filespaths)
                    runAFl.run()
                    self.select_seeds = False
                    self.chang_cases = False
                elif (not self.select_seeds) and self.chang_cases:
                    wx.MessageBox('Please select seeds first!', 'warnings', wx.OK | wx.ICON_INFORMATION)
            elif m == 'KLEE(STP)':
                self.panel1.Hide()
                runKlee = RunKLEE(self.import_path_list[self.import_item])
                ret_klee = runKlee.run()
                if ret_klee:
                    wx.MessageBox(ret_klee, 'errors', wx.OK | wx.ICON_INFORMATION)
            elif m == 'Peach':
                self.panel1.Hide()
                runPeach = RunPeach(self.import_path_list[self.import_item])
                runPeach.run()
            elif m == 'Peach+Sanitizer':
                self.panel1.Hide()
                runPeachS = RunPeachSanitizer(self.import_path_list[self.import_item])
                ret_PS = runPeachS.run()
                if 'ERROR' and 'Pyinstaller' in ret_PS:
                    if self.SysType == "Linux":
                        wx.MessageBox("Please installer pyinstaller first!", 'errors', wx.OK | wx.ICON_INFORMATION)
                    elif self.SysType == "Windows":
                        wx.MessageBox("Please installer pyinstaller and add pyinstaller.exe path to system path first!",
                                      'errors', wx.OK | wx.ICON_INFORMATION)
                if 'For CLANG' and 'system cannot support arch' in ret_PS:
                    wx.MessageBox(ret_PS, 'errors', wx.OK | wx.ICON_INFORMATION)
            elif m == 'libFuzzer':
                self.panel1.Hide()
                runLibFuzzer = RunLibFuzzer(self.import_path_list[self.import_item], self.filespaths, self.select_seeds)
                ret_lib = runLibFuzzer.run()
                if ret_lib:
                    wx.MessageBox(ret_lib, 'errors', wx.OK | wx.ICON_INFORMATION)
                if self.chang_cases:
                    self.select_seeds = False

    def OnClick_REPORT(self, event):
        if 'libFuzzer' in self.methods:
            genReport = GenerateInfoAndHtml(self.import_path_list[self.import_item], ['libFuzzer'])
            genReport.generate_report()
        if 'Peach+Sanitizer' in self.methods:
            genReport = GenerateInfoAndHtml(self.import_path_list[self.import_item], ['Peach+Sanitizer'])
            genReport.generate_report()
        SummaryInfo = GenerateSummaryInfo(self.import_path_list[self.import_item], self.methods[0])
        totalNum, failNum, execTime = SummaryInfo.genSumInfo()
        if len(self.methods) > 1:
            print('Only one summary report is supported on GUI, please check others by browser')
        if self.methods[0] == 'AFL' and self.SysType == "Linux":
            seeds, crashes, hangs, execTime = totalNum, failNum.split(',')[0].strip('('), failNum.split(',')[1].strip(
                ')'), execTime
            self.panel1.Show(show=True)
            execution_time = str(execTime).split(':')[0] + ' hrs, ' + str(execTime).split(':')[1] + ' mins, ' + \
                             str(execTime).split(':')[2] + ' secs'
            report = "%s Test Report \n Summary" % \
                     os.path.basename(self.import_path_list[self.import_item]).split('.inf')[0]
            seeds = "Total Case Numbers:    %s" % seeds
            crash = "Crashes Case Numbers:    %s " % (crashes)
            hang = "Hangs Case Numbers:    %s" % (hangs)
            exec_time = "Execution Time:    %s" % execution_time
            self.REPORTS = wx.StaticText(self.panel1, -1, report, (30, 0), size=(780, 150),
                                         style=wx.TE_MULTILINE | wx.TE_CENTER)
            self.REPORTS.SetBackgroundColour("#6CA6CD")
            self.REPORTS.SetForegroundColour("white")
            self.seeds = wx.StaticText(self.panel1, -1, seeds, (30, 50), size=(780, 50),
                                       style=wx.TE_MULTILINE | wx.TE_LEFT)
            self.seeds.SetForegroundColour("white")
            self.seeds.SetBackgroundColour("#6CA6CD")
            self.OutputPath = self.conf.get('afl', 'OutputPath').strip()
            url = os.path.join(self.OutputPath, 'afl_%s' % self.import_list[self.import_item], 'GdbReport',
                               "DebugReport", "IndexCrashes.html")
            url_hang = os.path.join(self.OutputPath, 'afl_%s' % self.import_list[self.import_item], 'GdbReport',
                                    "DebugReport", "IndexHangs.html")
            self.crash = hl.HyperLinkCtrl(self.panel1, wx.ID_ANY, crash, (30, 70),
                                          URL=url, style=wx.TE_MULTILINE | wx.TE_LEFT)
            self.hang = hl.HyperLinkCtrl(self.panel1, wx.ID_ANY, hang, (30, 90),
                                         URL=url_hang, style=wx.TE_MULTILINE | wx.TE_LEFT)
            self.crash.SetBold(True)
            self.hang.SetBold(True)

            self.crash.SetForegroundColour("#FF0000")
            self.hang.SetForegroundColour("#FF0000")
            self.exec_time = wx.StaticText(self.panel1, -1, exec_time, (30, 110), size=(780, 20),
                                           style=wx.TE_MULTILINE | wx.TE_LEFT)
            self.exec_time.SetForegroundColour("white")
            self.exec_time.SetBackgroundColour("#6CA6CD")
        elif self.methods[0] == 'Peach' or self.methods[0] == 'libFuzzer' or self.methods[0] == 'Peach+Sanitizer' or (
                self.methods[0] == 'AFL' and self.SysType == "Windows"):
            seeds, failNum, execTime = totalNum, failNum, execTime
            self.panel1.Show(show=True)
            execution_time = str(execTime).split(':')[0] + ' hrs, ' + str(execTime).split(':')[1] + ' mins, ' + \
                             str(execTime).split(':')[2] + ' secs'
            report = "%s Test Report \n Summary" % \
                     os.path.basename(self.import_path_list[self.import_item]).split('.inf')[0]
            seeds = "Total Case Numbers:    %s" % seeds
            failedSeeds = "Failure Type Numbers:    %s " % failNum
            exec_time = "Execution Time:    %s" % execution_time
            self.REPORTS = wx.StaticText(self.panel1, -1, report, (30, 0), size=(780, 150),
                                         style=wx.TE_MULTILINE | wx.TE_CENTER)
            self.REPORTS.SetBackgroundColour("#6CA6CD")
            self.REPORTS.SetForegroundColour("white")
            self.seeds = wx.StaticText(self.panel1, -1, seeds, (30, 50), size=(780, 50),
                                       style=wx.TE_MULTILINE | wx.TE_LEFT)
            self.seeds.SetForegroundColour("white")
            self.seeds.SetBackgroundColour("#6CA6CD")
            if self.methods[0] == 'Peach' and self.SysType == "Linux":
                self.OutputPath = self.conf.get('peach', 'OutputPath').strip()
                url = os.path.join(self.OutputPath, 'peach_%s' % self.import_list[self.import_item], 'GdbReport',
                                   "DebugReport", "IndexGdb.html")
            elif self.methods[0] == 'Peach' and self.SysType == "Windows":
                self.OutputPath = self.conf.get('peach', 'OutputPath').strip()
                url = os.path.join(self.OutputPath, 'peach_%s' % self.import_list[self.import_item], 'SanitizerReport',
                                   "DebugReport", "IndexSanitizer.html")
            elif self.methods[0] == 'AFL' and self.SysType == "Windows":
                self.OutputPath = self.conf.get('afl', 'OutputPath').strip()
                url = os.path.join(self.OutputPath, 'afl_%s' % self.import_list[self.import_item], 'SanitizerReport',
                                   "DebugReport", "IndexSanitizer.html")
            elif self.methods[0] == 'libFuzzer':
                self.OutputPath = self.conf.get('libfuzzer', 'OutputPath').strip()
                url = os.path.join(self.OutputPath, 'libfuzzer_%s' % self.import_list[self.import_item],
                                   'SanitizerReport', "DebugReport", "IndexSanitizer.html")
            elif self.methods[0] == 'Peach+Sanitizer':
                self.OutputPath = self.conf.get('peach+sanitizer', 'OutputPath').strip()
                url = os.path.join(self.OutputPath, 'peach+sanitizer_%s' % self.import_list[self.import_item],
                                   'SanitizerReport', "DebugReport", "IndexSanitizer.html")
            self.fail = hl.HyperLinkCtrl(self.panel1, wx.ID_ANY, failedSeeds, (30, 70),
                                         URL=url, style=wx.TE_MULTILINE | wx.TE_LEFT)
            self.fail.SetBold(True)
            self.fail.SetForegroundColour("#FF0000")
            self.exec_time = wx.StaticText(self.panel1, -1, exec_time, (30, 90), size=(780, 20),
                                           style=wx.TE_MULTILINE | wx.TE_LEFT)
            self.exec_time.SetForegroundColour("white")
            self.exec_time.SetBackgroundColour("#6CA6CD")
        SummaryReport = GenerateSummaryReport(self.import_path_list[self.import_item], self.methods)
        SummaryReport.open_summary_report()

    def OnClick_CODE_COVERAGE(self, event):
        getCodeCoverage = GetCodeCoverage(self.import_path_list[self.import_item], self.methods)
        ret = getCodeCoverage.run()
        if ret:
            wx.MessageBox(ret, 'errors', wx.OK | wx.ICON_INFORMATION)


if __name__ == "__main__":
    app = wx.App(False)
    frame = my_frame(None, 'Host-based Firmware Analyzer')
    frame.Show(show=True)
    icon = wx.Icon()
    icon.LoadFile(os.path.join(workspace_path, "image", "intel.png"))
    frame.SetIcon(icon)

    frame.Center()
    app.MainLoop()
