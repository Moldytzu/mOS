bits 64

global tssLoad

tssLoad:
    mov ax, 40
    ltr ax ; load tss from offset 40 in the gdt
    ret