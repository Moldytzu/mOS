bits 64

global gdtLoad

gdtLoad:
    lgdt [rdi] ; load gdt from the first argument
    mov ax, (8*2) ; 2nd segment, kernel data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop rdi
    mov rax, (8*1) ; 1st segment, kernel code
    push rax
    push rdi
    retfq