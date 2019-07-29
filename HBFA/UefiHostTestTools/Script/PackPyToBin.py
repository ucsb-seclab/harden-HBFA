## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import sys
import subprocess
import platform

class PackPyToBinary(object):
    def __init__(self):
        self.hbfa_tools_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        self.tobe_pack_script_path = os.path.join(self.hbfa_tools_path, 'Report')
        self.tobe_pack_script_name = 'GenSanitizerInfo.py'
        self.outputpath = self.hbfa_tools_path
        self.SysType = platform.system()

    def pack(self):
        if self.SysType == "Linux":
            command = 'pyinstaller -y --onefile %s --distpath %s' % (os.path.join(self.tobe_pack_script_path, self.tobe_pack_script_name), self.outputpath)
        else:
            command='pyinstaller.exe -y --onefile %s --distpath %s' % (os.path.join(self.tobe_pack_script_path, self.tobe_pack_script_name), self.outputpath)
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
            os._exit(0)
        elif 'pyinstaller: not found' in msg[0] or "'pyinstaller.exe' is not recognized as an internal or external command" in msg[0]:
            raise Exception ("ERROR: please use command 'pip install Pyinstaller' to installer pyinstaller,and please add pyinstaller.exe path to system path on windows")
            os._exit(0)
        else:
            pass

if __name__ == "__main__":
    PackBin = PackPyToBinary()
    PackBin.pack()