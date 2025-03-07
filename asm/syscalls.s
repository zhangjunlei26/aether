    .text
    .global sys_write
    .global sys_read_file
    .global sys_spawn_thread

// sys_write:
// For demonstration, this uses the Linux syscall interface (syscall instruction).
// (Note: On Windows the syscall mechanism is different, but this is our low-level stub.)
sys_write:
    mov $1, %rax           # syscall number for write
    # Parameters are expected in rdi, rsi, rdx
    syscall
    ret

// sys_read_file:
// Returns the address of an empty null-terminated string.
// Use 'movabs' to load a full 64-bit immediate address.
sys_read_file:
    movabs $empty_str, %rax
    ret

// sys_spawn_thread:
// A stub that calls the function pointer in rdi.
sys_spawn_thread:
    call *%rdi
    ret

    .data
empty_str:
    .asciz ""
