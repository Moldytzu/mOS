bits 64

global tssLoad

tssLoad:
    mov ax, (8*5) ; 5th segment, kernel code
    ltr ax ; load tss from offset 40 in the gdt
    ret