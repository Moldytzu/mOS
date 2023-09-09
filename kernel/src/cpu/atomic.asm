bits 64

section .text

global atomicWrite, atomicRelease, atomicAquire
extern schedScheduleIfPossible

; writes atomically to address
; rdi=address, rsi=value
atomicWrite:
    mov QWORD [rdi], rsi
    ret

; releases atomic spinlock
; rdi=address
atomicRelease:
	lock btr QWORD [rdi], 0x0
	ret

; aquires atomic spinlock
; rdi=address
atomicAquire:
	.acquire: ; aquire lock
		lock bts QWORD [rdi], 0x0
		jnc	.exit ; if carry bit is set then it's locked
	.spin:
		call schedScheduleIfPossible
		bt QWORD [rdi], 0x0
		jc .spin
		jmp	.acquire
	.exit:
		ret