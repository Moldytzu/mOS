bits 64

global gdtLoad

gdtLoad:
    lgdt [rdi]
    mov ax, 16
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop rdi
    mov rax, 8
    push rax
    push rdi
    retfq