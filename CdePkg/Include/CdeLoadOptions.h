/*!
@copyright
	Copyright (c) 2019, MinnowWare. All rights reserved.
	This program and the accompanying materials are licensed and made
	available under the terms and conditions of the BSD License
	which accompanies this distribution.  The full text of the license
	may be found at
	http://opensource.org/licenses/bsd-license.php
	THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
	WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
@file
	CdeLoadOptionsDxe.h
@brief this file provides the FILE_GUID to command line / Filename + LoadOptions mapping
@todo
*/
    {/*FILE_GUID, EfiCallerIdGuid*/{ 0xCDE000FF, 0x4C0B, 0x4994, { 0xA2, 0x9E, 0x09, 0xD8, 0xA3, 0xCB, 0xF7, 0xE2 }}, "templatePEI" },
    {/*FILE_GUID, EfiCallerIdGuid*/{ 0xCDE000FF, 0x669C, 0x4A4D, { 0x9E, 0xB8, 0x05, 0x03, 0x48, 0xA3, 0xA5, 0xDE }}, "templateDXE" },

    {/*FILE_GUID, EfiCallerIdGuid*/{ 0xCDE000FF, 0xB3EB, 0x44D3, { 0xB4, 0x5C, 0xA4, 0xE8, 0x8E, 0x43, 0xA2, 0x37 }}, "argcv abc \"d e f g\" \\\"1 23" },
    {/*FILE_GUID, EfiCallerIdGuid*/{ 0xCDE000FF, 0xD745, 0x4676, { 0x87, 0x87, 0xC5, 0x5C, 0xE9, 0x9B, 0x39, 0xED }}, "argcv abc \"d e f g\" \\\"1 23" },

    {/*FILE_GUID, EfiCallerIdGuid*/{ 0xCDE000FF, 0xF737, 0x4515, { 0xAD, 0x5C, 0x7C, 0x8E, 0x32, 0x5A, 0x2B, 0x0F }},"systeminterfaceDXE" },
    {/*FILE_GUID, EfiCallerIdGuid*/{ 0xCDE000FF, 0xCC17, 0x4C1C, { 0x86, 0x3D, 0x76, 0x60, 0xD3, 0x31, 0xF1, 0x4A }},"systeminterfacePEI" },

    {/*FILE_GUID, EfiCallerIdGuid*/{ 0xCDE00007, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }},"UnknownDriver" },
	/* add more drivername/commandline pairs here...*/
        //CDE000FF - FAF3 - 49E1 - B9FF - 1A0BE4440DAF