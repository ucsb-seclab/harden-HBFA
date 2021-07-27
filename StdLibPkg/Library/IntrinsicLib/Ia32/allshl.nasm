;
; Implementation of _allshl()
; Originates from CryptoPkg.
;
; Copyright (c) 2014-2021, Intel Corporation. All rights reserved.<BR>
; 
; SPDX-License-Identifier: BSD-2-Clause-Patent
;

SECTION .text

global ASM_PFX(_allshl)
ASM_PFX(_allshl):
    ;
    ; Handle shifting of 64 or more bits (return 0)
    ;
    cmp     cl, 64
    jae     short ReturnZero

    ;
    ; Handle shifting of between 0 and 31 bits
    ;
    cmp     cl, 32
    jae     short More32
    shld    edx, eax, cl
    shl     eax, cl
    ret

    ;
    ; Handle shifting of between 32 and 63 bits
    ;
More32:
    mov     edx, eax
    xor     eax, eax
    and     cl, 31
    shl     edx, cl
    ret

ReturnZero:
    xor     eax,eax
    xor     edx,edx
    ret