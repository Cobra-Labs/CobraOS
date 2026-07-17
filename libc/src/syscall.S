// syscall.S

.global __syscall0
__syscall0:
    mov %rdi, %rax
    syscall
    ret

.global __syscall1
__syscall1:
    mov %rdi, %rax
    mov %rsi, %rdi
    syscall
    ret

.global __syscall2
__syscall2:
    mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    syscall 
    ret

.global __syscall3
__syscall3:
    mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    syscall
    ret
