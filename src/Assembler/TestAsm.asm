; Description:
; 
; Revision date:

.data
myMessage BYTE "MASM program example",0dh,0ah,0

.code
main PROC
  add rax, rbx
  add al, bl
  add rax, [rbx + 12h]
  add rax, [rbx + 12345678h]
  add al, 12h
  add rax, 12h
  add rax, 12345678h

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

  cmp rax, 07fffffffffffffffh
  cmp rbx, 07fffffffffffffffh

  imul rax, rbx;
  imul r8, r9;
  imul ax, bx;


main ENDP

END

