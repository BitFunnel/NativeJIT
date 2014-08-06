; Description:
; 
; Revision date:

.data
myMessage BYTE "MASM program example",0dh,0ah,0

.code
main PROC

;
; Mod/RM special cases for RSP and R12 and [RBP] ==> [RBP + disp8]
;
sub rbx, [r12]
sub rdi, [r12 + 12h]
sub rbp, [r12 + 1234h]
sub r10, [r12 + 12345678h]

;
; Mod/RM case for R8..R15
;

;
; REX W bit. X bit.
;

;
; Group1 addressing mode permutations for a single opcode.
;

; direct-direct
add al, cl
add bx, dx
add esi, eax
add rax, rbx
add r8, r9
add rsp, r12

; direct-indirect with zero, byte, word, and double word offsets
add cl, [rax]
add bl, [rcx + 12h]
add r9b, [rsi + 100h]
add r15b, [rdi + 12345678h]

cmp dl, [rdx]
cmp cx, [rcx + 12h]
cmp r9w, [rsi + 1234h]
cmp r11w, [rdi + 12345678h]

or esp, [r9]
or edx, [rcx + 12h]
or esi, [rsi + 1234h]
or r11d, [rdi + 12345678h]

sub rbx, [r12]
sub rdi, [rcx + 12h]
sub rbp, [rsi + 1234h]
sub r10, [rdi + 12345678h]

; direct-immediate register 0 case
or al, 34h
or ax, 56h
or ax, 5678h
or eax, 12h
or eax, 1234h
or eax, 12345678h
or rax, 12h
or rax, 1234h
or rax, 12345678h

; direct-immediate general purpose register case
and bl, 34h
and cx, 56h
and dx, 5678h
and ebp, 12h
and ebp, 1234h
and ebp, 12345678h
and r12, 12h
and r12, 1234h
and r12, 12345678h


;
; One example of each Group1 opcode.
;
add rdx, [rsi + 56h]
cmp rdx, [rsi + 56h]
or rdx, [rsi + 56h]
cmp rdx, [rsi + 56h]
; adc, sbb, and, xor not supported.

;
; Imul
;

;
; Lea
;
lea rax, [rsi]
lea rax, [rsi + 12h]
lea rax, [rsi + 1234h]
lea rax, [rsi + 12345678h]
lea rbp, [r12]
lea rbp, [r12 + 87h]
lea rbp, [r12 + 87654321h]
lea rsp, [rbp + 0FFFFFFE0h]
lea rbp, [rsp + 20h]

;
; Mov
;
mov al, cl
mov bx, dx
mov esi, eax
mov rax, rbx
mov r8, r9
mov rsp, r12

mov cl, [rax]
mov bl, [rcx + 12h]
mov r9b, [rsi + 100h]
mov r15b, [rdi + 12345678h]

mov dl, [rdx]
mov cx, [rcx + 12h]
mov r9w, [rsi + 1234h]
mov r11w, [rdi + 12345678h]

mov esp, [r9]
mov edx, [rcx + 12h]
mov esi, [rsi + 1234h]
mov r11d, [rdi + 12345678h]

mov rbx, [r12]
mov rdi, [rcx + 12h]
mov rbp, [rsi + 1234h]
mov r10, [rdi + 12345678h]

;
; Push/Pop
;
pop rax
pop rbp
pop r12
push rbx
push rbp
push r12

;
; Ret
;
ret

;
; Shift
;


  add rax, rbx
  add al, bl
  add rax, [rbx + 12h]
  add rax, [rbx + 12345678h]
  add al, 12h
  add rax, 12h
  add rax, 12345678h

L1:
  sub rax, rbx

  cmp rax, r9;

  lea rax, [rbx + 12h]

  pop rax;
  pop r8;
  push rax;
  push r8;

  ret

  cmp rax, 07fh
  cmp rbx, 07fh

  cmp rax, 07fffh
  cmp rbx, 07fffh

  cmp rax, 07fffffffh
  cmp rbx, 07fffffffh

;  cmp rax, 07fffffffffffffffh
;  cmp rbx, 07fffffffffffffffh

  imul rax, rbx;
  imul r8, r9;
  imul ax, bx;

  mov rax, 07fffffffffffffffh
main ENDP

END

