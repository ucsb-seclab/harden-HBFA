## @file
# Makefiles
#
# Copyright (c) 2007 - 2014, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

MAKEROOT ?= ../..

include $(MAKEROOT)/Makefiles/header.makefile

APPLICATION = $(MAKEROOT)$(SEP)bin$(SEP)$(APPNAME)

.PHONY:all
all: $(MAKEROOT)/bin $(APPLICATION)

$(APPLICATION): $(OBJECTS)
	$(LINKER) -o $(APPLICATION) $(BUILD_LFLAGS) $(OBJECTS) -L$(MAKEROOT)/libs $(LIBS)
ifeq ($(OS),Windows_NT)
	$(CP) $(APPLICATION) $(EDK_TOOLS_PATH)\Bin\Win32
endif

$(OBJECTS): $(MAKEROOT)/Include/Common/BuildVersion.h

include $(MAKEROOT)/Makefiles/footer.makefile
