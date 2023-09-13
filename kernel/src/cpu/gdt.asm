bits 64

section .text

global gdtLoad

; load the gdt
gdtLoad:
    lgdt [rdi] ; load gdt from the first argument
    
    mov ax, (8*2) ; 2nd segment, kernel data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax
    
    pop rdi        ; pop return address
    mov rax, (8*1) ; 1st segment, kernel code
    
    push rax ; push the new code segment
    push rdi ; push the return address
    retfq    ; do the far return