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

main ENDP

END

