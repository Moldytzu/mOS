bits 64

global atomicWrite, atomicClearLock, atomicLock, atomicRelease, atomicAquire, atomicAquireCli

; writes atomically to address; rdi=address, rsi=value
atomicWrite:
    mov QWORD [rdi], rsi
    ret

%macro	CF_RESULT	0
	mov		rcx, 1
	mov		rax, 0
	cmovnc	rax, rcx
%endmacro

atomicRelease:	; rdi = mutex location memory , 0x0 = location of the bit where we store the statu
	lock btr		QWORD [rdi], 0x0
	CF_RESULT
	ret

atomicAquire:		; rdi = mutex location memory , 0x0 = location of the bit where we store the statu
	.acquire:
		lock bts	QWORD [rdi], 0x0
		jnc			.exit				; CF = 0 to begin with
	.spin:
		pause
		bt			QWORD [rdi], 0x0
		jc			.spin				; CF = 1 still
		jmp			.acquire
	.exit:
		ret