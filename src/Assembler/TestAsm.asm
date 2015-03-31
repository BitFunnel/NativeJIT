; Description:
; 
; Revision date:

.data
myMessage BYTE "MASM program example",0dh,0ah,0

.code
main PROC

    mov rcx, 16;
    mov rax, 1234567812345678h
    mov rbx, 2108765432108765h
    ; expect 2238767832348778

l0:
    cmp rax, rbx
    jae l1

    ; rbx is greater - shift its high bits into the low bits of rax
    shld rax,rbx, 4
    jmp l2

l1:

    ; rax is greater - rotate its high bits into the low bits of rax
    rol rax, 4
l2:

    ; both cases, shift the high order bits of rbx out of the way
    shl rbx, 4

    ;; decrement the loop counter and go back to the top if there are more fields
    dec rcx
    jnz l0
    ret


shld rax, rbx, 4
;movd xmm1, rax
;vprotq xmm0, xmm1, 4

add [rcx + 5], rax
add rax, [rcx + 5]
movd xmm0, rax
movsd xmm0, xmm1
; vmovq xmm0, rax       // Illegal instruction
ret

; Another special case
add r13, [r13]
mov r13, [r13]
mov rax, [rbp]
mov rax, [rbp + 12h]
mov rbx, [r13 + 34h]
mov rcx, [rbp + 1234h]

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
and r13b, 34h
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
; Lea
;
lea rax, [rsi]
lea rax, [rsi + 12h]
lea rax, [rsi - 12h]
lea rax, [rsi + 1234h]
lea rax, [rsi - 1234h]
lea rax, [rsi + 12345678h]
lea rax, [rsi - 12345678h]
lea rbp, [r12]
lea rbp, [r12 + 87h]
lea rbp, [r12 - 789ABCDEh]
lea rbp, [rsp + 20h]
lea rsp, [rbp - 20h]

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
mov al, 0
mov al, 34h
mov ax, 56h
mov ax, 5678h
mov eax, 12h
mov eax, 1234h
mov eax, 12345678h
mov rax, 12h
mov rax, 1234h
mov rax, 12345678h
mov rax, 80000000h
mov rax, -1

; mov r, imm - general purpose register case
mov bl, 0
mov bl, 34h
mov r13b, 34h
mov cx, 56h
mov dx, 5678h
mov ebp, 12h
mov ebp, 1234h
mov ebp, 12345678h
mov r12, 12h
mov r12, 1234h
mov r12, 12345678h
mov r12, 80000000h
mov rbx, 1234567812345678h
mov rsp, 1234567812345678h
mov r12, 1234567812345678h


; mov [r + offset], r with zero, byte, word, and dword offsets
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

LL1:
    jz LL1
    jz near ptr LL1
    jz LL1
    jz near ptr LL1
;    jz far ptr LL1


;
; Imul
;

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

imul cx, 56h
imul cx, 80h
imul dx, 5678h
imul ebp, 12h
imul ebp, 1234h
imul ebp, 12345678h
imul r12, 12h
imul r12, 1234h
imul r12, 12345678h
imul r12, -1

call rax
call rsp
call rbp
call r12
call r13

mov rax, 07fffffffffffffffh

movd xmm1, rax
movd xmm1, rcx
movd xmm1, r8
movd xmm1, rbp
movd xmm1, r12

movd xmm0, rcx
movd xmm1, rcx
movd xmm2, rcx
movd xmm5, rcx
movd xmm12, rcx

movd xmm1, eax
movd xmm1, ecx
movd xmm1, r8d
movd xmm1, ebp
movd xmm1, r12d

movd xmm0, ecx
movd xmm1, ecx
movd xmm2, ecx
movd xmm5, ecx
movd xmm12, ecx

; movss
movss xmm1, xmm2
movss xmm0, xmm12
movss xmm5, xmm12
movss xmm5, xmm3
movss xmm13, xmm5
movss xmm0, xmm15

movss xmm0, dword ptr [r12]
movss xmm4, dword ptr [rcx + 12h]
movss xmm5, dword ptr [rsi + 1234h]
movss xmm12, dword ptr [rdi + 12345678h]

movss dword ptr [r12], xmm0
movss dword ptr [rcx + 12h], xmm4
movss dword ptr [rsi + 1234h], xmm5
movss dword ptr [rdi + 12345678h], xmm12

; movsd
movsd xmm1, xmm2
movsd xmm0, xmm12
movsd xmm5, xmm12
movsd xmm5, xmm3
movsd xmm13, xmm5
movsd xmm0, xmm15

movsd xmm0, mmword ptr [r12]
movsd xmm4, mmword ptr [rcx + 12h]
movsd xmm5, mmword ptr [rsi + 1234h]
movsd xmm12, mmword ptr [rdi + 12345678h]

movsd mmword ptr [r12], xmm0
movsd mmword ptr [rcx + 12h], xmm4
movsd mmword ptr [rsi + 1234h], xmm5
movsd mmword ptr [rdi + 12345678h], xmm12

addsd xmm1, xmm2
addsd xmm0, xmm12
mulsd xmm5, xmm12
mulsd xmm5, xmm3
subsd xmm13, xmm5
subsd xmm0, xmm15

addsd xmm0, mmword ptr [r12]
addsd xmm4, mmword ptr [rcx + 12h]
mulsd xmm5, mmword ptr [rsi + 1234h]
subsd xmm12, mmword ptr [rdi + 12345678h]

addss xmm1, xmm2
addss xmm0, xmm12
mulss xmm5, xmm12
mulss xmm5, xmm3
subss xmm13, xmm5
subss xmm0, xmm15

addss xmm0, dword ptr [r12]
addss xmm4, dword ptr [rcx + 12h]
mulss xmm5, dword ptr [rsi + 1234h]
subss xmm12, dword ptr [rdi + 12345678h]

;
; MovZX
;

; 1 byte to 2, 4 and 8.
movzx bx, bl
movzx bx, r12b
movzx r9w, dl
movzx bx, byte ptr [rcx + 12h]
movzx bx, byte ptr [r9 + 34h]

movzx ebx, bl
movzx ebx, r12b
movzx r9d, dl
movzx ebx, byte ptr [rcx + 12h]
movzx ebx, byte ptr [r9 + 34h]

movzx rbx, bl
movzx rbx, r12b
movzx r9, dl
movzx rbx, byte ptr [rcx + 12h]
movzx rbx, byte ptr [r9 + 34h]

; 2 bytes to 4 and 8
movzx ebx, bx
movzx ebx, r12w
movzx r9d, dx
movzx ebx, word ptr [rcx + 12h]
movzx ebx, word ptr [r9 + 34h]

movzx rbx, bx
movzx rbx, r12w
movzx r9, dx
movzx rbx, word ptr [rcx + 12h]
movzx rbx, word ptr [r9 + 34h]

;
; CvtSI2SD/CvtSI2SS
;

cvtsi2ss xmm1, eax
cvtsi2ss xmm1, rax
cvtsi2ss xmm9, rbx
cvtsi2ss xmm1, r8
cvtsi2ss xmm1, dword ptr [rcx + 12h]
cvtsi2ss xmm1, dword ptr [r9 + 34h]
cvtsi2ss xmm1, qword ptr [rcx + 56h]

cvtsi2sd xmm1, eax
cvtsi2sd xmm1, rax
cvtsi2sd xmm9, rbx
cvtsi2sd xmm1, r8
cvtsi2sd xmm1, dword ptr [rcx + 12h]
cvtsi2sd xmm1, dword ptr [r9 + 34h]
cvtsi2sd xmm1, qword ptr [rcx + 56h]

;
; CvtTSD2SI/CvtTSS2SI
;

cvttss2si eax, xmm1
cvttss2si rax, xmm1
cvttss2si rbx, xmm9
cvttss2si r8, xmm1
cvttss2si ebx, dword ptr [rcx + 12h]
cvttss2si ebx, dword ptr [r9 + 34h]
cvttss2si rbx, dword ptr [rcx + 56h]

cvttsd2si eax, xmm1
cvttsd2si rax, xmm1
cvttsd2si rbx, xmm9
cvttsd2si r8, xmm1
cvttsd2si ebx, mmword ptr [rcx + 12h]
cvttsd2si ebx, mmword ptr [r9 + 34h]
cvttsd2si rbx, mmword ptr [rcx + 56h]

;
; Conversion, float - cvtss2sd and cvtsd2ss
;

cvtss2sd xmm1, xmm1
cvtss2sd xmm2, xmm9
cvtss2sd xmm2, dword ptr [rcx + 20h]
cvtss2sd xmm2, dword ptr [r9 + 200h]

cvtsd2ss xmm1, xmm1
cvtsd2ss xmm2, xmm9
cvtsd2ss xmm2, qword ptr [rcx + 20h]
cvtsd2ss xmm2, qword ptr [r9 + 200h]


;
; Shift/rotate
;

rol al, cl
sal ebx, cl
shr r12, cl

rol rax, 3
sal bl, 4
shr r12d, 5

shld edx, esi, cl
shld r12, rbp, cl
shld rbp, r12, cl

main ENDP

END

