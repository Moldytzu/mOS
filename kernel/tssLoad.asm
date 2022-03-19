bits 64

global tssLoad

tssLoad:
    mov ax, 40
    ltr ax ; load tss from offset 64 in the gdt
    ret