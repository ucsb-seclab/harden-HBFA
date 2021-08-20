## @file
# GNU/Linux makefile for 'GenCrc32' module build.
#
# Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
MAKEROOT ?= ..

ifeq ($(OS),Windows_NT)
  APPNAME = GenCrc32.exe
else
  APPNAME = GenCrc32
endif

LIBS = -lCommon

OBJECTS = GenCrc32.o

include $(MAKEROOT)/Makefiles/app.makefile
