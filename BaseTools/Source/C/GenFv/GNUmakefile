## @file
# GNU/Linux makefile for 'GenFv' module build.
#
# Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
MAKEROOT ?= ..

ifeq ($(OS),Windows_NT)
  APPNAME = GenFv.exe
else
  APPNAME = GenFv
endif

OBJECTS = GenFv.o GenFvInternalLib.o

include $(MAKEROOT)/Makefiles/app.makefile

LIBS = -lCommon
ifeq ($(CYGWIN), CYGWIN)
  LIBS += -L/lib/e2fsprogs -luuid
endif

ifeq ($(LINUX), Linux)
  LIBS += -luuid
endif

