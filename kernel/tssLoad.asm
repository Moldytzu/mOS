bits 64

global tssLoad

; load the tss
tssLoad:
    mov ax, (8*5) ; 5th segment, tss system segment
    ltr ax ; load tss from offset 40 in the gdt
    ret