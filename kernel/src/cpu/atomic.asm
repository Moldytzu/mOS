bits 64

global atomicWrite

; writes atomically to address; rdi=address, rsi=value
atomicWrite:
    lock mov [rdi], rsi
    ret