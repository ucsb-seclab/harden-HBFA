#
# Python script that produces wrapper files for pdclib source files
# to reuse unit tests already provided by pdclib.
#
# pdclib stdlib source files contain main() functions which execute
# unit tests on the function they implement. In order to avoid
# having multiple main() symbols in one unit test application, this script
# makes those function symbols distinct.
#
#  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#

import sys
import os

# Location for test folders and prototype header
ut_path = 'UnitTests'

# Prototype header name
ut_proto_filename = 'UnitTestPrototypes.h'

# Dictionary holding all the test classes
test_classes = dict()

# string dir files
test_classes['string'] = [
    ['pdclib/functions/string/memchr.c',            'pdctest_memchr'],
    ['pdclib/functions/string/memcmp.c',            'pdctest_memcmp'],
    # ['pdclib/functions/string/memcpy_s.c',          'pdctest_memcpy_s'],
    ['pdclib/functions/string/memcpy.c',            'pdctest_memcpy'],
    # ['pdclib/functions/string/memmove_s.c',         'pdctest_memmove_s'],
    ['pdclib/functions/string/memmove.c',           'pdctest_memmove'],
    # ['pdclib/functions/string/memset_s.c',          'pdctest_memset_s'],
    ['pdclib/functions/string/memset.c',            'pdctest_memset'],
    # ['pdclib/functions/string/strcat_s.c',          'pdctest_strcat_s'],
    ['pdclib/functions/string/strcat.c',            'pdctest_strcat'],
    ['pdclib/functions/string/strchr.c',            'pdctest_strchr'],
    ['pdclib/functions/string/strcmp.c',            'pdctest_strcmp'],
    ['pdclib/functions/string/strcoll.c',           'pdctest_strcoll'],
    # ['pdclib/functions/string/strcpy_s.c',          'pdctest_strcpy_s'],
    ['pdclib/functions/string/strcpy.c',            'pdctest_strcpy'],
    ['pdclib/functions/string/strcspn.c',           'pdctest_strcspn'],
    # ['pdclib/functions/string/strerror_s.c',        'pdctest_strerror_s'],
    ['pdclib/functions/string/strerror.c',          'pdctest_strerror'],
    # ['pdclib/functions/string/strerrorlen_s.c',     'pdctest_strerrorlen_s'],
    ['pdclib/functions/string/strlen.c',            'pdctest_strlen'],
    # ['pdclib/functions/string/strncat_s.c',         'pdctest_strncat_s'],
    ['pdclib/functions/string/strncat.c',           'pdctest_strncat'],
    ['pdclib/functions/string/strncmp.c',           'pdctest_strncmp'],
    # ['pdclib/functions/string/strncpy_s.c',         'pdctest_strncpy_s'],
    ['pdclib/functions/string/strncpy.c',           'pdctest_strncpy'],
    ['pdclib/functions/string/strpbrk.c',           'pdctest_strpbrk'],
    ['pdclib/functions/string/strrchr.c',           'pdctest_strrchr'],
    ['pdclib/functions/string/strspn.c',            'pdctest_strspn'],
    ['pdclib/functions/string/strstr.c',            'pdctest_strstr'],
    # ['pdclib/functions/string/strtok_s.c',          'pdctest_strtok_s'],
    # ['pdclib/functions/string/strtok.c',            'pdctest_strtok'],
    ['pdclib/functions/string/strxfrm.c',           'pdctest_strxfrm']
]

# ctype dir files
test_classes['ctype'] = [
    ['pdclib/functions/ctype/isalnum.c',            'pdctest_isalnum'],
    ['pdclib/functions/ctype/isalpha.c',            'pdctest_isalpha'],
    ['pdclib/functions/ctype/isblank.c',            'pdctest_isblank'],
    ['pdclib/functions/ctype/iscntrl.c',            'pdctest_iscntrl'],
    ['pdclib/functions/ctype/isdigit.c',            'pdctest_isdigit'],
    ['pdclib/functions/ctype/isgraph.c',            'pdctest_isgraph'],
    ['pdclib/functions/ctype/islower.c',            'pdctest_islower'],
    ['pdclib/functions/ctype/isprint.c',            'pdctest_isprint'],
    ['pdclib/functions/ctype/ispunct.c',            'pdctest_ispunct'],
    ['pdclib/functions/ctype/isspace.c',            'pdctest_isspace'],
    ['pdclib/functions/ctype/isupper.c',            'pdctest_isupper'],
    ['pdclib/functions/ctype/isxdigit.c',           'pdctest_isxdigit'],
    ['pdclib/functions/ctype/tolower.c',            'pdctest_tolower'],
    ['pdclib/functions/ctype/toupper.c',            'pdctest_toupper'],
]

# inttypes dir files
test_classes['inttypes'] = [
    ['pdclib/functions/inttypes/imaxabs.c',         'pdctest_imaxabs'],
    ['pdclib/functions/inttypes/imaxdiv.c',         'pdctest_imaxdiv'],
    ['pdclib/functions/inttypes/strtoimax.c',       'pdctest_strtoimax'],
    ['pdclib/functions/inttypes/strtoumax.c',       'pdctest_strtoumax'],
]

##
# Function to generate test wrapper
#
def generate_test_wrapper(test_entry, class_path):
    source_file_path = test[0]
    test_symbol = test[1]
    target_file_path = class_path + '/' + os.path.basename(source_file_path)

    print('Creating test file wrapper: ' + target_file_path)

    with open(target_file_path, 'w') as target_file:
        # Include info about auto-generated file
        target_file.write('// This is an auto-generated test file wrapper.\n\n')

        # Include common test header
        target_file.write('#include "UnitTestCommon.h"\n\n')

        # Enable test mode
        target_file.write('#define TEST\n\n')

        # Redefine main symbol into distinct symbol
        target_file.write('#define main    ' + test_symbol + '\n\n')

        # Include appropriate source file
        target_file.write('#include \"' + source_file_path + '\"\n')


##
# Main script flow
#

# Get location of CStdLib. Start from script location
script_path = os.path.realpath(__file__)
script_dir_path = os.path.dirname(script_path)
cstdlib_path = os.path.dirname(script_dir_path)

# Establish target path for test files and classes
target_path = cstdlib_path + '/' + ut_path

target_proto_path = target_path + '/' + ut_proto_filename

# Process each test class
with open(target_proto_path, 'w') as proto_file:

    # Add autogen header to test function prototype header
    proto_file.write('// This is an auto-generated header.\n\n')

    for test_class in test_classes:
        class_path = target_path + '/' + test_class
        print('Creating tests of class: ' + test_class + ' under path: ' + class_path)

        # Create test class directory
        if not os.path.isdir(class_path):
            try:
                os.mkdir(class_path)
            except:
                print('Could not create directory: ' + class_path)
                sys.exit(1)

        # Create test files
        for test in test_classes[test_class]:
            generate_test_wrapper(test, class_path)
            proto_file.write('int {}(void);\n'.format(test[1]))

sys.exit(0)