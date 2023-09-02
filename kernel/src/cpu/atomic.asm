bits 64

section .text

global atomicWrite, atomicRelease, atomicAquire

; writes atomically to address
; rdi=address, rsi=value
atomicWrite:
    mov QWORD [rdi], rsi
    ret

%macro	CF_RESULT	0
	mov		rcx, 1
	mov		rax, 0
	cmovnc	rax, rcx
%endmacro

; releases atomic spinlock
; rdi=address
atomicRelease:
	lock btr QWORD [rdi], 0x0
	CF_RESULT
	ret

; aquires atomic spinlock
; rdi=address
atomicAquire:
	.acquire:
		lock bts QWORD [rdi], 0x0
		jnc	.exit
	.spin:
		pause
		bt QWORD [rdi], 0x0
		jc .spin
		jmp	.acquire
	.exit:
		ret