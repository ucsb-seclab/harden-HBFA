;
; Implementation of _ftol2().
; Originates from CryptoPkg.
;
; Copyright (c) 2019-2021, Intel Corporation. All rights reserved.<BR>
; 
; SPDX-License-Identifier: BSD-2-Clause-Patent
;

SECTION .text

global ASM_PFX(_ftol2)
ASM_PFX(_ftol2):
    fistp qword [esp-8]
    mov   edx, [esp-4]
    mov   eax, [esp-8]
    ret