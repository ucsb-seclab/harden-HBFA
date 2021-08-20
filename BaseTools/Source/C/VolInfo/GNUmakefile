## @file
# GNU/Linux makefile for 'VolInfo' module build.
#
# Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
MAKEROOT ?= ..

ifeq ($(OS),Windows_NT)
  APPNAME = VolInfo.exe
else
  APPNAME = VolInfo
endif

OBJECTS = VolInfo.o

include $(MAKEROOT)/Makefiles/app.makefile

LIBS = -lCommon


