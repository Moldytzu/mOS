bits 64
align 4096

global idleTask

idleTask:
    jmp $ ; don't do anything