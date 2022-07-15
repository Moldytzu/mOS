bits 64

global sseInit, x87Init

x87Init:
    fninit ; init x87
    ret

sseInit:    
    mov rax, cr0
    and rax, 0xFFFFFFFFFFFFFFFB ; CR0 &= 0xFFFFFFFFFFFFFFFB, clear CR0.EM
    or rax, 0x2 ; CR0 |= 0x2, set CR0.MP
    mov cr0, rax

    mov rax, cr4
    or rax, 0x200 ; CR4 |= 0x200 ; set CR4.OSFXSR
    or rax, 0x10000 ; CR4 |= 0x10000 ; set CR4.OSXMMEXCPT
    mov cr4, rax

    ret ; return