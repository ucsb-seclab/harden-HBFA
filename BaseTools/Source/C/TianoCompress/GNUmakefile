## @file
# GNU/Linux makefile for 'TianoCompress' module build.
#
# Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
MAKEROOT ?= ..

ifeq ($(OS),Windows_NT)
  APPNAME = TianoCompress.exe
else
  APPNAME = TianoCompress
endif

LIBS = -lCommon

OBJECTS = TianoCompress.o

include $(MAKEROOT)/Makefiles/app.makefile
