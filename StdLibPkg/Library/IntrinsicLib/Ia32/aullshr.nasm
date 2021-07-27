;
; Implementation of _aullshr()
; Originates from CryptoPkg.
;
; Copyright (c) 2014-2021, Intel Corporation. All rights reserved.<BR>
; 
; SPDX-License-Identifier: BSD-2-Clause-Patent
;

SECTION .text

global ASM_PFX(_aullshr)
ASM_PFX(_aullshr):
    ;
    ; Checking: Only handle 64bit shifting or more
    ;
    cmp     cl, 64
    jae     _Exit

    ;
    ; Handle shifting between 0 and 31 bits
    ;
    cmp     cl, 32
    jae     More32
    shrd    eax, edx, cl
    shr     edx, cl
    ret

    ;
    ; Handle shifting of 32-63 bits
    ;
More32:
    mov     eax, edx
    xor     edx, edx
    and     cl, 31
    shr     eax, cl
    ret

    ;
    ; Invalid number (less then 32bits), return 0
    ;
_Exit:
    xor     eax, eax
    xor     edx, edx
    ret