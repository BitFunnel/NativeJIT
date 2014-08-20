; Description:
; 
; Revision date:

.data
myMessage BYTE "MASM program example",0dh,0ah,0

.code
main PROC

; Another special case
add r13, [r13]
mov r13, [r13]

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

; mov r, imm - register 0 case
mov al, 34h
mov ax, 56h
mov ax, 5678h
mov eax, 12h
mov eax, 1234h
mov eax, 12345678h
mov rax, 12h
mov rax, 1234h
mov rax, 12345678h

; mov r, imm - general purpose register case
mov bl, 34h
mov cx, 56h
mov dx, 5678h
mov ebp, 12h
mov ebp, 1234h
mov ebp, 12345678h
mov r12, 12h
mov r12, 1234h
mov r12, 12345678h
mov rbx, 1234567812345678h
mov rsp, 1234567812345678h
mov r12, 1234567812345678h


; mov [r + offset], r with zero, byte, word, and double word offsets
mov [rax], cl
mov [rcx + 12h], bl
mov [rsi + 100h], r9b
mov [rdi + 12345678h], r15b

mov [rdx], dl
mov [rcx + 12h], cx
mov [rsi + 1234h], r9w
mov [rdi + 12345678h], r11w

mov [r9], esp
mov [rcx + 12h], edx
mov [rsi + 1234h], esi
mov [rdi + 12345678h], r11d

mov [r12], rbx
mov [rcx + 12h], rdi
mov [rsi + 1234h], rbp
mov [rdi + 12345678h], r10


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

L1:
    jz L1
    jz near ptr L1
    jz L1
    jz near ptr L1
;    jz far ptr L1


imul bx, cx
imul ebx, ecx
imul rbx, rcx

imul rax, rbx;
imul r8, r9;
imul ax, bx;

imul cx, [rcx + 12h]
imul r9w, [rsi + 1234h]
imul r11w, [rdi + 12345678h]

imul esp, [r9]
imul edx, [rcx + 12h]
imul esi, [rsi + 1234h]
imul r11d, [rdi + 12345678h]

imul rbx, [r12]
imul rdi, [rcx + 12h]
imul rbp, [rsi + 1234h]
imul r10, [rdi + 12345678h]

;;imul bl, 34h
imul cx, 56h
imul dx, 5678h
imul ebp, 12h
imul ebp, 1234h
imul ebp, 12345678h
imul r12, 12h
imul r12, 1234h
imul r12, 12345678h

call rax
call rsp
call rbp
call r12
call r13

  mov rax, 07fffffffffffffffh
main ENDP

END

