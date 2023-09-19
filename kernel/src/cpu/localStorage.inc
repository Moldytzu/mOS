bits 64

section .text

%macro SWAPGS_TO_KERNEL_IF_NECESSARY 0
	mov rax, [gs:16] ; load kernel flag
	cmp rax, 1       ; check if it set
	je .kernelGS     ; if it is then we don't do anything
	swapgs           ; do the swapping

.kernelGS:
%endmacro