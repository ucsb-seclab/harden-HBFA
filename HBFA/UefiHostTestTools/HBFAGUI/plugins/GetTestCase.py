## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os

try:
    import ConfigParser as configparser
except Exception as e:
    import configparser as configparser


class GetTestCase(object):
    def __init__(self):
        self.HBFAGUI_Path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        self.conf = configparser.ConfigParser()
        self.conf_path = os.path.join(self.HBFAGUI_Path, 'Env.conf')
        self.conf.read(self.conf_path)
        self.HBFA_package_path = os.path.dirname(os.path.dirname(self.HBFAGUI_Path))
        self.filePath_list = []

    def get_test_case_name_path(self):
        HBFA_package_path = os.path.join(self.HBFA_package_path, 'UefiHostFuzzTestCasePkg')
        for root, dirs, files in os.walk(os.path.join(HBFA_package_path, 'TestCase')):
            for file in files:
                if file.endswith('.inf') and os.path.basename(file).startswith('Test') and 'InstrumentHookLib' not in file:
                    filepath = os.path.join(root, file)
                    fileName = file.split('.')[0]
                    self.filePath_list.append(filepath)
        self.filePath_list.sort(key=lambda filepath: os.path.basename(filepath).split('.')[0])
        return self.filePath_list
