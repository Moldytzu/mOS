bits 64

section .text

errno: resd 1

global errno, _mmain
extern main

; first function that is called
; rdi=argc
; rsi=argv
_mmain:
    call main
    
    mov rdi, 0   ; sys_exit
    mov rsi, rax ; pass error code
    syscall      ; exit