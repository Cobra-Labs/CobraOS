// crt0.S

.global _start
.extern main
.extern _exit
_start:
    xor %rbp, %rbp
    call main
    mov %eax, %edi
    call _exit
