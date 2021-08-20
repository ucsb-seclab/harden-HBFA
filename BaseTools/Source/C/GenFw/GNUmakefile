## @file
# GNU/Linux makefile for 'GenFw' module build.
#
# Copyright (c) 2009 - 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
MAKEROOT ?= ..

ifeq ($(OS),Windows_NT)
  APPNAME = GenFw.exe
else
  APPNAME = GenFw
endif

OBJECTS = GenFw.o ElfConvert.o Elf32Convert.o Elf64Convert.o

include $(MAKEROOT)/Makefiles/app.makefile

LIBS = -lCommon
ifeq ($(CYGWIN), CYGWIN)
  LIBS += -L/lib/e2fsprogs -luuid
endif

ifeq ($(LINUX), Linux)
  LIBS += -luuid
endif

