## @file
# Makefiles
#
# Copyright (c) 2007 - 2014, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

include $(MAKEROOT)/Makefiles/header.makefile

ifeq ($(OS),Windows_NT)
  LIBRARY = $(MAKEROOT)/libs/$(LIBNAME).lib
else
  LIBRARY = $(MAKEROOT)/libs/lib$(LIBNAME).a
endif

all: $(MAKEROOT)/libs $(LIBRARY)

include $(MAKEROOT)/Makefiles/footer.makefile
