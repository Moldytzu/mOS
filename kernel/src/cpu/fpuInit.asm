bits 64

global sseInit, x87Init

x87Init:
    fninit ; init x87
    ret

sseInit:    
    mov rax, cr0
    and ax, 0xFFFB ; clear CR0.EM
    or rax, 0x2 ; set CR0.MP
    mov cr0, rax

    mov rax, cr4
    or ax, 3 << 9 ;set CR4.OSFXSR and CR4.OSXMMEXCPT
    mov cr4, rax

    ret ; return