## @file
# GNU/Linux makefile for 'EfiRom' module build.
#
# Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
MAKEROOT ?= ..

ifeq ($(OS),Windows_NT)
  APPNAME = EfiRom.exe
else
  APPNAME = EfiRom
endif

LIBS = -lCommon

OBJECTS = EfiRom.o

include $(MAKEROOT)/Makefiles/app.makefile
