bits 64

section .text

%include "registers.inc"

extern exceptionHandler, intHandlers

; generate interrupt handlers for each and every possible vector and put their addresses in intHandlers
%macro GEN_HANDLER 1
global __baseHandler%1

__baseHandler%1:
%ifn %1 == 0x8 || %1 == 0xA || %1 == 0xB || %1 == 0xC || %1 == 0xD || %1 == 0xE || %1 == 0x11 || %1 == 0x15 || %1 == 0x1D || %1 == 0x1E
    sub rsp, 8 ; allocate an error code on the stack if the cpu doesn't do it for us 
%endif
    PUSH_REG              ; save old registers
    mov rdi, rsp          ; give the handler the stack frame
    mov rsi, %1           ; give the intrerrupt number
    call exceptionHandler ; call the handler
    POP_REG               ; restore registers
    add rsp, 8            ; clean up error code on the stack
    iretq                 ; return to previous context
%endmacro

%assign i 0
%rep 256
GEN_HANDLER i
%assign i i+1
%endrep

section .data
intHandlers:
%assign i 0
%rep 256
    dq __baseHandler%+i
%assign i i+1
%endrep