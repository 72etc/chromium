; Copyright (c) 2011 The Chromium Authors. All rights reserved.
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.

  global    mangle(SYMBOL) PRIVATE
  align     function_align

; Non-PIC code is the fastest so use this if possible.
%ifndef PIC
mangle(SYMBOL):
  %assign   stack_offset 0
  PROLOGUE  6, 7, 3, Y, U, V, A, ARGB, WIDTH, TEMP
  extern    mangle(kCoefficientsRgbY)
  jmp       .convertend

.convertloop:
  movzx     TEMPd, BYTE [Uq]
  movq      mm0, [mangle(kCoefficientsRgbY) + 2048 + 8 * TEMPq]
  add       Uq, 1
  movzx     TEMPd, BYTE [Vq]
  paddsw    mm0, [mangle(kCoefficientsRgbY) + 4096 + 8 * TEMPq]
  add       Vq, 1
  movzx     TEMPd, BYTE [Yq]
  movq      mm1, [mangle(kCoefficientsRgbY) + 8 * TEMPq]
  movzx     TEMPd, BYTE [Yq + 1]
  movq      mm2, [mangle(kCoefficientsRgbY) + 8 * TEMPq]
  add       Yq, 2
  paddsw    mm1, mm0
  paddsw    mm2, mm0
  psraw     mm1, 6
  psraw     mm2, 6
  packuswb  mm1, mm2

  ; Multiply ARGB by alpha value.
  movq      mm0, mm1
  pxor      mm2, mm2
  punpcklbw mm0, mm2
  punpckhbw mm1, mm2
  movzx     TEMPd, BYTE [Aq]
  movq      mm2, [mangle(kCoefficientsRgbY) + 6144 + 8 * TEMPq]
  pmullw    mm0, mm2
  psrlw     mm0, 8
  movzx     TEMPd, BYTE [Aq + 1]
  movq      mm2, [mangle(kCoefficientsRgbY) + 6144 + 8 * TEMPq]
  add       Aq, 2
  pmullw    mm1, mm2
  psrlw     mm1, 8
  packuswb  mm0, mm1

  MOVQ      [ARGBq], mm0
  add       ARGBq, 8

.convertend:
  sub       WIDTHq, 2
  jns       .convertloop

  ; If number of pixels is odd then compute it.
  and       WIDTHq, 1
  jz        .convertdone

  movzx     TEMPd, BYTE [Uq]
  movq      mm0, [mangle(kCoefficientsRgbY) + 2048 + 8 * TEMPq]
  movzx     TEMPd, BYTE [Vq]
  paddsw    mm0, [mangle(kCoefficientsRgbY) + 4096 + 8 * TEMPq]
  movzx     TEMPd, BYTE [Yq]
  movq      mm1, [mangle(kCoefficientsRgbY) + 8 * TEMPq]
  paddsw    mm1, mm0
  psraw     mm1, 6
  packuswb  mm1, mm1

  ; Multiply ARGB by alpha value.
  pxor      mm0, mm0
  punpcklbw mm1, mm0
  movzx     TEMPd, BYTE [Aq]
  movq      mm0, [mangle(kCoefficientsRgbY) + 6144 + 8 * TEMPq]
  pmullw    mm1, mm0
  psrlw     mm1, 8
  packuswb  mm1, mm1

  movd      [ARGBq], mm1

.convertdone:
  RET
%endif

; With PIC code we need to load the address of mangle(kCoefficientsRgbY).
; This code is slower than the above version.
%ifdef PIC
mangle(SYMBOL):
  %assign   stack_offset 0
  PROLOGUE  6, 7, 3, Y, U, V, A, ARGB, WIDTH, TEMP
  extern    mangle(kCoefficientsRgbY)
  PUSH      WIDTHq
  DEFINE_ARGS Y, U, V, A, ARGB, TABLE, TEMP
  LOAD_SYM  TABLEq, mangle(kCoefficientsRgbY)
  jmp       .convertend

.convertloop:
  movzx     TEMPd, BYTE [Uq]
  movq      mm0, [TABLEq + 2048 + 8 * TEMPq]
  add       Uq, 1

  movzx     TEMPd, BYTE [Vq]
  paddsw    mm0, [TABLEq + 4096 + 8 * TEMPq]
  add       Vq, 1

  movzx     TEMPd, BYTE [Yq]
  movq      mm1, [TABLEq + 8 * TEMPq]

  movzx     TEMPd, BYTE [Yq + 1]
  movq      mm2, [TABLEq + 8 * TEMPq]
  add       Yq, 2

  ; Add UV components to Y component.
  paddsw    mm1, mm0
  paddsw    mm2, mm0

  ; Down shift and then pack.
  psraw     mm1, 6
  psraw     mm2, 6
  packuswb  mm1, mm2

  ; Unpack and multiply by alpha value, then repack high bytes of words.
  movq      mm0, mm1
  pxor      mm2, mm2
  punpcklbw mm0, mm2
  punpckhbw mm1, mm2
  movzx     TEMPd, BYTE [Aq]
  movq      mm2, [TABLEq + 6144 + 8 * TEMPq]
  pmullw    mm0, mm2
  psrlw     mm0, 8
  movzx     TEMPd, BYTE [Aq + 1]
  movq      mm2, [TABLEq + 6144 + 8 * TEMPq]
  add       Aq, 2
  pmullw    mm1, mm2
  psrlw     mm1, 8
  packuswb  mm0, mm1

  MOVQ      [ARGBq], mm0
  add       ARGBq, 8

.convertend:
  sub       dword [rsp], 2
  jns       .convertloop

  ; If number of pixels is odd then compute it.
  and       dword [rsp], 1
  jz        .convertdone

  movzx     TEMPd, BYTE [Uq]
  movq      mm0, [TABLEq + 2048 + 8 * TEMPq]
  movzx     TEMPd, BYTE [Vq]
  paddsw    mm0, [TABLEq + 4096 + 8 * TEMPq]
  movzx     TEMPd, BYTE [Yq]
  movq      mm1, [TABLEq + 8 * TEMPq]
  paddsw    mm1, mm0
  psraw     mm1, 6
  packuswb  mm1, mm1

  ; Multiply ARGB by alpha value.
  pxor      mm0, mm0
  punpcklbw mm1, mm0
  movzx     TEMPd, BYTE [Aq]
  movq      mm0, [TABLEq + 6144 + 8 * TEMPq]
  pmullw    mm1, mm0
  psrlw     mm1, 8
  packuswb  mm1, mm1

  movd      [ARGBq], mm1

.convertdone:
  POP       TABLEq
  RET
%endif