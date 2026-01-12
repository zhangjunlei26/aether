	.file	"bench_scheduler.c"
	.text
	.p2align 4
	.globl	bench_actor_step
	.def	bench_actor_step;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_actor_step
bench_actor_step:
	.seh_endprologue
	movl	81944(%rcx), %edx
	testl	%edx, %edx
	je	.L3
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L2:
	movl	81936(%rcx), %eax
	subl	$1, %edx
	movl	%edx, 81944(%rcx)
	addl	$1, %eax
	andl	$2047, %eax
	movl	%eax, 81936(%rcx)
	lock addl	$1, 81960(%rcx)
	movl	81944(%rcx), %edx
	testl	%edx, %edx
	jne	.L2
.L3:
	movl	$0, 4(%rcx)
	ret
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC0:
	.ascii "\12=== Single Core Throughput ===\0"
.LC5:
	.ascii "Messages: %d / %d (%.1f%%)\12\0"
.LC6:
	.ascii "Time: %.3f seconds\12\0"
.LC7:
	.ascii "Throughput: %.0f msg/sec\12\0"
	.text
	.p2align 4
	.globl	bench_single_core_throughput
	.def	bench_single_core_throughput;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_single_core_throughput
bench_single_core_throughput:
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$184, %rsp
	.seh_stackalloc	184
	vmovaps	%xmm6, 160(%rsp)
	.seh_savexmm	%xmm6, 160
	.seh_endprologue
	leaq	.LC0(%rip), %rcx
	call	puts
	movl	$1, %ecx
	call	scheduler_init
	movl	$81976, %ecx
	call	malloc
	movq	$1, (%rax)
	movq	%rax, %rsi
	leaq	bench_actor_step(%rip), %rax
	movq	%rax, 81952(%rsi)
	leaq	81960(%rsi), %rbp
	xorl	%eax, %eax
	xchgl	0(%rbp), %eax
	xorl	%edx, %edx
	xorl	%ebx, %ebx
	movq	$0, 81936(%rsi)
	movq	%rsi, %rcx
	movl	$0, 81944(%rsi)
	call	scheduler_register_actor
	call	scheduler_start
	movq	__imp_GetTickCount64(%rip), %r12
	call	*%r12
	movq	%rax, %r13
	.p2align 4,,10
	.p2align 3
.L12:
	vpxor	%xmm2, %xmm2, %xmm2
	movl	$-1, %r8d
	leaq	48(%rsp), %rdx
	movq	%rsi, %rcx
	vmovdqu	%ymm2, 116(%rsp)
	movl	$0, 148(%rsp)
	movq	144(%rsp), %rax
	movl	%ebx, 120(%rsp)
	movl	$1, 112(%rsp)
	vmovdqu	112(%rsp), %ymm0
	movq	%rax, 80(%rsp)
	vmovdqa	%ymm0, 48(%rsp)
	vzeroupper
	call	scheduler_send_remote
	addl	$1, %ebx
	cmpl	$100000, %ebx
	jne	.L12
	movq	__imp_Sleep(%rip), %rdi
	movslq	%r13d, %rbx
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L14:
	call	*%r12
	subq	%rbx, %rax
	cmpq	$4999, %rax
	ja	.L16
	movl	$1, %ecx
	call	*%rdi
.L13:
	movl	0(%rbp), %eax
	cmpl	$99999, %eax
	jle	.L14
.L16:
	vxorps	%xmm6, %xmm6, %xmm6
	call	*%r12
	subl	%r13d, %eax
	vcvtsi2sdl	%eax, %xmm6, %xmm0
	movl	81960(%rsi), %ebx
	vdivsd	.LC2(%rip), %xmm0, %xmm1
	vmovsd	%xmm1, 40(%rsp)
	call	scheduler_stop
	call	scheduler_wait
	movl	$100000, %r8d
	movl	%ebx, %edx
	leaq	.LC5(%rip), %rcx
	vcvtsi2sdl	%ebx, %xmm6, %xmm6
	vmulsd	.LC3(%rip), %xmm6, %xmm3
	vdivsd	.LC4(%rip), %xmm3, %xmm4
	vmovq	%xmm4, %r9
	vmovapd	%xmm4, %xmm3
	call	printf
	vmovsd	40(%rsp), %xmm1
	leaq	.LC6(%rip), %rcx
	vmovq	%xmm1, %rdx
	call	printf
	leaq	.LC7(%rip), %rcx
	vdivsd	40(%rsp), %xmm6, %xmm1
	vmovq	%xmm1, %rdx
	call	printf
	movq	%rsi, %rcx
	call	free
	movq	.refptr.schedulers(%rip), %rax
	vmovaps	160(%rsp), %xmm6
	movq	16(%rax), %rcx
	addq	$184, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	jmp	free
	.seh_endproc
	.section .rdata,"dr"
.LC9:
	.ascii "\12=== %d-Core Throughput ===\12\0"
.LC10:
	.ascii "Per-core: %.0f msg/sec\12\0"
	.text
	.p2align 4
	.globl	bench_multi_core_throughput
	.def	bench_multi_core_throughput;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_multi_core_throughput
bench_multi_core_throughput:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$200, %rsp
	.seh_stackalloc	200
	vmovaps	%xmm6, 160(%rsp)
	.seh_savexmm	%xmm6, 160
	vmovaps	%xmm7, 176(%rsp)
	.seh_savexmm	%xmm7, 176
	.seh_endprologue
	movl	%ecx, %ebx
	movl	%ecx, %edx
	leaq	.LC9(%rip), %rcx
	call	printf
	movl	%ebx, %ecx
	imull	$50000, %ebx, %ebp
	call	scheduler_init
	movslq	%ebx, %r10
	leaq	0(,%r10,8), %r13
	movq	%r13, %rcx
	call	malloc
	testl	%ebx, %ebx
	movq	%rax, %rdi
	jle	.L19
	movq	%rax, %r15
	xorl	%r14d, %r14d
	leaq	bench_actor_step(%rip), %r12
	.p2align 4,,10
	.p2align 3
.L20:
	movl	$81976, %ecx
	call	malloc
	movl	%r14d, %edx
	addl	$1, %r14d
	xorl	%ecx, %ecx
	movq	%rax, (%r15)
	movl	%r14d, (%rax)
	movl	$0, 4(%rax)
	movq	%r12, 81952(%rax)
	xchgl	81960(%rax), %ecx
	movq	(%r15), %rcx
	movq	$0, 81936(%rcx)
	movl	$0, 81944(%rcx)
	addq	$8, %r15
	call	scheduler_register_actor
	cmpl	%ebx, %r14d
	jne	.L20
	call	scheduler_start
	movq	__imp_GetTickCount64(%rip), %r12
	xorl	%esi, %esi
	call	*%r12
	movl	%eax, 40(%rsp)
	.p2align 4,,10
	.p2align 3
.L23:
	movl	%esi, %eax
	vpxor	%xmm2, %xmm2, %xmm2
	movl	$-1, %r8d
	movl	$0, 148(%rsp)
	cltd
	vmovdqu	%ymm2, 116(%rsp)
	idivl	%ebx
	movl	%esi, 120(%rsp)
	movslq	%edx, %rdx
	movq	(%rdi,%rdx,8), %rcx
	leaq	48(%rsp), %rdx
	movl	(%rcx), %eax
	movl	%eax, 112(%rsp)
	movq	144(%rsp), %rax
	vmovdqu	112(%rsp), %ymm0
	movq	%rax, 80(%rsp)
	vmovdqa	%ymm0, 48(%rsp)
	vzeroupper
	call	scheduler_send_remote
	addl	$1, %esi
	cmpl	%ebp, %esi
	jl	.L23
.L22:
	testl	%ebx, %ebx
	leaq	(%rdi,%r13), %r15
	movq	__imp_Sleep(%rip), %r14
	movq	%rdi, %rsi
	movslq	40(%rsp), %r13
	jle	.L33
	.p2align 4,,10
	.p2align 3
.L41:
	movq	%rdi, %rax
	xorl	%edx, %edx
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L25:
	movq	(%rax), %rcx
	addq	$8, %rax
	movl	81960(%rcx), %ecx
	addl	%ecx, %edx
	cmpq	%r15, %rax
	jne	.L25
	cmpl	%edx, %ebp
	jle	.L26
.L42:
	call	*%r12
	subq	%r13, %rax
	cmpq	$5000, %rax
	ja	.L26
	movl	$1, %ecx
	call	*%r14
	testl	%ebx, %ebx
	jg	.L41
.L33:
	xorl	%edx, %edx
	cmpl	%edx, %ebp
	jg	.L42
	.p2align 4,,10
	.p2align 3
.L26:
	call	*%r12
	testl	%ebx, %ebx
	movq	%rax, %rcx
	jle	.L34
	movq	%rdi, %rax
	xorl	%r12d, %r12d
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L29:
	movq	(%rax), %rdx
	addq	$8, %rax
	movl	81960(%rdx), %edx
	addl	%edx, %r12d
	cmpq	%r15, %rax
	jne	.L29
.L28:
	vxorps	%xmm7, %xmm7, %xmm7
	movl	%ecx, %eax
	subl	40(%rsp), %eax
	vcvtsi2sdl	%eax, %xmm7, %xmm0
	vdivsd	.LC2(%rip), %xmm0, %xmm1
	vmovsd	%xmm1, 40(%rsp)
	call	scheduler_stop
	call	scheduler_wait
	movl	%ebp, %r8d
	movl	%r12d, %edx
	leaq	.LC5(%rip), %rcx
	vcvtsi2sdl	%r12d, %xmm7, %xmm6
	vmulsd	.LC3(%rip), %xmm6, %xmm3
	vcvtsi2sdl	%ebp, %xmm7, %xmm0
	vdivsd	%xmm0, %xmm3, %xmm4
	vmovq	%xmm4, %r9
	vmovapd	%xmm4, %xmm3
	call	printf
	vmovsd	40(%rsp), %xmm1
	leaq	.LC6(%rip), %rcx
	vmovq	%xmm1, %rdx
	call	printf
	leaq	.LC7(%rip), %rcx
	vdivsd	40(%rsp), %xmm6, %xmm1
	vmovq	%xmm1, %rdx
	vmovsd	%xmm1, 40(%rsp)
	call	printf
	vmovsd	40(%rsp), %xmm1
	leaq	.LC10(%rip), %rcx
	vcvtsi2sdl	%ebx, %xmm7, %xmm0
	vdivsd	%xmm0, %xmm1, %xmm1
	vmovq	%xmm1, %rdx
	call	printf
	testl	%ebx, %ebx
	jle	.L32
	movq	.refptr.schedulers(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	16(%rax), %rbx
	.p2align 4,,10
	.p2align 3
.L31:
	movq	(%rsi), %rcx
	addq	$8, %rsi
	addq	$3170624, %rbx
	call	free
	movq	-3170624(%rbx), %rcx
	call	free
	cmpq	%r15, %rsi
	jne	.L31
.L32:
	vmovaps	160(%rsp), %xmm6
	movq	%rdi, %rcx
	vmovaps	176(%rsp), %xmm7
	addq	$200, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	jmp	free
.L19:
	call	scheduler_start
	movq	__imp_GetTickCount64(%rip), %r12
	call	*%r12
	movl	%eax, 40(%rsp)
	jmp	.L22
.L34:
	xorl	%r12d, %r12d
	jmp	.L28
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC12:
	.ascii "\12=== Cross-Core Messaging Overhead ===\0"
	.align 8
.LC15:
	.ascii "Cross-core messages: %d / %d (%.1f%%)\12\0"
	.text
	.p2align 4
	.globl	bench_cross_core_overhead
	.def	bench_cross_core_overhead;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_cross_core_overhead
bench_cross_core_overhead:
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$176, %rsp
	.seh_stackalloc	176
	vmovaps	%xmm6, 160(%rsp)
	.seh_savexmm	%xmm6, 160
	.seh_endprologue
	leaq	.LC12(%rip), %rcx
	call	puts
	movl	$4, %ecx
	call	scheduler_init
	movl	$81976, %ecx
	call	malloc
	movl	$81976, %ecx
	movq	%rax, %r13
	call	malloc
	leaq	bench_actor_step(%rip), %rdx
	movq	$1, 0(%r13)
	movq	%rdx, 81952(%r13)
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	%eax, %edi
	xchgl	81960(%r13), %edi
	movq	$2, (%rsi)
	leaq	81960(%rsi), %rbp
	movq	$0, 81936(%r13)
	movl	$0, 81944(%r13)
	movq	%rdx, 81952(%rsi)
	xchgl	0(%rbp), %eax
	xorl	%edx, %edx
	xorl	%ebx, %ebx
	movq	$0, 81936(%rsi)
	movq	%r13, %rcx
	movl	$0, 81944(%rsi)
	call	scheduler_register_actor
	movl	$3, %edx
	movq	%rsi, %rcx
	call	scheduler_register_actor
	call	scheduler_start
	movq	__imp_GetTickCount64(%rip), %r12
	call	*%r12
	movq	%rax, %r14
	.p2align 4,,10
	.p2align 3
.L44:
	vpxor	%xmm2, %xmm2, %xmm2
	xorl	%r8d, %r8d
	leaq	48(%rsp), %rdx
	movq	%rsi, %rcx
	vmovdqu	%ymm2, 116(%rsp)
	movl	$0, 148(%rsp)
	movq	144(%rsp), %rax
	movl	%ebx, 120(%rsp)
	movl	$2, 112(%rsp)
	vmovdqu	112(%rsp), %ymm0
	movq	%rax, 80(%rsp)
	vmovdqa	%ymm0, 48(%rsp)
	vzeroupper
	call	scheduler_send_remote
	addl	$1, %ebx
	cmpl	$10000, %ebx
	jne	.L44
	movq	__imp_Sleep(%rip), %rdi
	movslq	%r14d, %rbx
	jmp	.L45
	.p2align 4,,10
	.p2align 3
.L46:
	call	*%r12
	subq	%rbx, %rax
	cmpq	$4999, %rax
	ja	.L49
	movl	$1, %ecx
	call	*%rdi
.L45:
	movl	0(%rbp), %eax
	cmpl	$9999, %eax
	jle	.L46
.L49:
	vxorps	%xmm6, %xmm6, %xmm6
	call	*%r12
	subl	%r14d, %eax
	vcvtsi2sdl	%eax, %xmm6, %xmm0
	movl	81960(%rsi), %ebx
	vdivsd	.LC2(%rip), %xmm0, %xmm1
	vmovsd	%xmm1, 40(%rsp)
	call	scheduler_stop
	call	scheduler_wait
	movl	$10000, %r8d
	movl	%ebx, %edx
	leaq	.LC15(%rip), %rcx
	vcvtsi2sdl	%ebx, %xmm6, %xmm6
	vmulsd	.LC3(%rip), %xmm6, %xmm3
	vdivsd	.LC14(%rip), %xmm3, %xmm4
	vmovq	%xmm4, %r9
	vmovapd	%xmm4, %xmm3
	call	printf
	vmovsd	40(%rsp), %xmm1
	leaq	.LC6(%rip), %rcx
	vmovq	%xmm1, %rdx
	call	printf
	leaq	.LC7(%rip), %rcx
	vdivsd	40(%rsp), %xmm6, %xmm1
	vmovq	%xmm1, %rdx
	call	printf
	movq	%r13, %rcx
	call	free
	movq	%rsi, %rcx
	call	free
	movq	.refptr.schedulers(%rip), %rbx
	leaq	12682496(%rbx), %rsi
.L47:
	movq	16(%rbx), %rcx
	addq	$3170624, %rbx
	call	free
	cmpq	%rbx, %rsi
	jne	.L47
	vmovaps	160(%rsp), %xmm6
	addq	$176, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC18:
	.ascii "\12=== Scalability Analysis ===\0"
	.align 8
.LC19:
	.ascii "Cores | Throughput (msg/sec) | Efficiency\0"
	.align 8
.LC20:
	.ascii "------|----------------------|-----------\0"
	.align 8
.LC21:
	.ascii "  %d   | %15.0f        | %6.1f%%\12\0"
	.text
	.p2align 4
	.globl	bench_scalability
	.def	bench_scalability;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_scalability
bench_scalability:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$216, %rsp
	.seh_stackalloc	216
	vmovaps	%xmm6, 160(%rsp)
	.seh_savexmm	%xmm6, 160
	vmovaps	%xmm7, 176(%rsp)
	.seh_savexmm	%xmm7, 176
	vmovaps	%xmm8, 192(%rsp)
	.seh_savexmm	%xmm8, 192
	.seh_endprologue
	leaq	.LC18(%rip), %rcx
	vxorps	%xmm6, %xmm6, %xmm6
	movl	$1, %esi
	call	puts
	leaq	.LC19(%rip), %rcx
	call	puts
	leaq	.LC20(%rip), %rcx
	call	puts
	movq	__imp_GetTickCount64(%rip), %r13
	vxorpd	%xmm7, %xmm7, %xmm7
	movl	$4, 44(%rsp)
	vmovsd	.LC2(%rip), %xmm8
.L72:
	movslq	%esi, %rbx
	imull	$25000, %esi, %ebp
	movl	%esi, %ecx
	call	scheduler_init
	salq	$3, %rbx
	movq	%rbx, %rcx
	call	malloc
	testl	%esi, %esi
	movq	%rax, %rdi
	jle	.L53
	movq	%rax, %r15
	xorl	%r14d, %r14d
	.p2align 4,,10
	.p2align 3
.L54:
	movl	$81976, %ecx
	call	malloc
	movl	%r14d, %edx
	leaq	bench_actor_step(%rip), %rcx
	addl	$1, %r14d
	movq	%rcx, 81952(%rax)
	xorl	%ecx, %ecx
	movq	%rax, (%r15)
	movl	%r14d, (%rax)
	movl	$0, 4(%rax)
	xchgl	81960(%rax), %ecx
	movq	(%r15), %rcx
	movq	$0, 81936(%rcx)
	movl	$0, 81944(%rcx)
	addq	$8, %r15
	call	scheduler_register_actor
	cmpl	%esi, %r14d
	jne	.L54
	call	scheduler_start
	xorl	%r15d, %r15d
	call	*%r13
	movl	%eax, 40(%rsp)
	.p2align 4,,10
	.p2align 3
.L57:
	movl	%r15d, %eax
	vpxor	%xmm4, %xmm4, %xmm4
	movl	$-1, %r8d
	movl	$0, 148(%rsp)
	cltd
	vmovdqu	%ymm4, 116(%rsp)
	idivl	%esi
	movl	%r15d, 120(%rsp)
	movslq	%edx, %rdx
	movq	(%rdi,%rdx,8), %rcx
	leaq	48(%rsp), %rdx
	movl	(%rcx), %eax
	movl	%eax, 112(%rsp)
	movq	144(%rsp), %rax
	vmovdqu	112(%rsp), %ymm0
	movq	%rax, 80(%rsp)
	vmovdqa	%ymm0, 48(%rsp)
	vzeroupper
	call	scheduler_send_remote
	addl	$1, %r15d
	cmpl	%ebp, %r15d
	jl	.L57
.L56:
	addq	%rdi, %rbx
	testl	%esi, %esi
	movslq	40(%rsp), %r14
	movq	%rdi, %r15
	jle	.L75
	.p2align 4,,10
	.p2align 3
.L89:
	movq	%rdi, %rax
	xorl	%edx, %edx
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L59:
	movq	(%rax), %rcx
	addq	$8, %rax
	movl	81960(%rcx), %ecx
	addl	%ecx, %edx
	cmpq	%rbx, %rax
	jne	.L59
	cmpl	%ebp, %edx
	jge	.L60
.L90:
	call	*%r13
	subq	%r14, %rax
	cmpq	$5000, %rax
	ja	.L60
	movl	$1, %ecx
	call	*__imp_Sleep(%rip)
	testl	%esi, %esi
	jg	.L89
.L75:
	xorl	%edx, %edx
	cmpl	%ebp, %edx
	jl	.L90
	.p2align 4,,10
	.p2align 3
.L60:
	call	*%r13
	testl	%esi, %esi
	movq	%rax, %r8
	jle	.L62
	movq	%rdi, %rax
	xorl	%edx, %edx
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L63:
	movq	(%rax), %rcx
	addq	$8, %rax
	movl	81960(%rcx), %ecx
	addl	%ecx, %edx
	cmpq	%rbx, %rax
	jne	.L63
	subl	40(%rsp), %r8d
	vcvtsi2sdl	%edx, %xmm6, %xmm0
	cmpl	$1, %esi
	vcvtsi2sdl	%r8d, %xmm6, %xmm1
	vdivsd	%xmm8, %xmm1, %xmm1
	vdivsd	%xmm1, %xmm0, %xmm5
	vmovq	%xmm5, %r8
	jne	.L67
	vmovapd	%xmm5, %xmm7
.L67:
	vxorpd	%xmm0, %xmm0, %xmm0
	vcomisd	%xmm0, %xmm7
	jbe	.L86
.L65:
	vcvtsi2sdl	%esi, %xmm6, %xmm0
	vmulsd	%xmm7, %xmm0, %xmm0
	movl	%esi, %edx
	vmovq	%r8, %xmm2
	leaq	.LC21(%rip), %rcx
	vdivsd	%xmm0, %xmm2, %xmm0
	vmulsd	.LC3(%rip), %xmm0, %xmm3
	vmovq	%xmm3, %r9
	call	printf
	call	scheduler_stop
	call	scheduler_wait
	testl	%esi, %esi
	jle	.L73
.L74:
	movq	.refptr.schedulers(%rip), %rax
	leaq	16(%rax), %rbp
	.p2align 4,,10
	.p2align 3
.L71:
	movq	(%r15), %rcx
	addq	$8, %r15
	addq	$3170624, %rbp
	call	free
	movq	-3170624(%rbp), %rcx
	call	free
	cmpq	%rbx, %r15
	jne	.L71
.L73:
	movq	%rdi, %rcx
	addl	%esi, %esi
	call	free
	subl	$1, 44(%rsp)
	jne	.L72
	vmovaps	160(%rsp), %xmm6
	vmovaps	176(%rsp), %xmm7
	vmovaps	192(%rsp), %xmm8
	addq	$216, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.L86:
	vxorpd	%xmm3, %xmm3, %xmm3
	vmovq	%r8, %xmm2
	movl	%esi, %edx
	vmovq	%xmm3, %r9
	leaq	.LC21(%rip), %rcx
	call	printf
	call	scheduler_stop
	call	scheduler_wait
	jmp	.L74
.L53:
	call	scheduler_start
	call	*%r13
	movl	%eax, 40(%rsp)
	jmp	.L56
.L62:
	subl	40(%rsp), %r8d
	vxorpd	%xmm1, %xmm1, %xmm1
	vcvtsi2sdl	%r8d, %xmm6, %xmm0
	vdivsd	%xmm8, %xmm0, %xmm0
	vcomisd	%xmm1, %xmm7
	vdivsd	%xmm0, %xmm1, %xmm5
	vmovq	%xmm5, %r8
	ja	.L65
	vxorpd	%xmm3, %xmm3, %xmm3
	vmovapd	%xmm5, %xmm2
	movl	%esi, %edx
	vmovq	%xmm3, %r9
	leaq	.LC21(%rip), %rcx
	call	printf
	call	scheduler_stop
	call	scheduler_wait
	jmp	.L73
	.seh_endproc
	.section .rdata,"dr"
.LC22:
	.ascii "\12=== Latency Test ===\0"
.LC23:
	.ascii "Samples: %d / %d (%.1f%%)\12\0"
.LC24:
	.ascii "Avg latency: %.2f ms\12\0"
	.align 8
.LC25:
	.ascii "Min latency: ~%.2f ms (theoretical)\12\0"
	.text
	.p2align 4
	.globl	bench_latency
	.def	bench_latency;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_latency
bench_latency:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$200, %rsp
	.seh_stackalloc	200
	vmovaps	%xmm6, 160(%rsp)
	.seh_savexmm	%xmm6, 160
	vmovaps	%xmm7, 176(%rsp)
	.seh_savexmm	%xmm7, 176
	.seh_endprologue
	leaq	.LC22(%rip), %rcx
	call	printf
	movl	$2, %ecx
	call	scheduler_init
	movl	$81976, %ecx
	call	malloc
	movq	$1, (%rax)
	movq	%rax, %rcx
	movq	%rax, 32(%rsp)
	leaq	bench_actor_step(%rip), %rax
	leaq	81960(%rcx), %rdi
	movq	%rax, 81952(%rcx)
	xorl	%eax, %eax
	xchgl	(%rdi), %eax
	xorl	%edx, %edx
	xorl	%r12d, %r12d
	xorl	%r13d, %r13d
	xorl	%r14d, %r14d
	movq	$0, 81936(%rcx)
	movl	$0, 81944(%rcx)
	call	scheduler_register_actor
	call	scheduler_start
	leaq	48(%rsp), %rax
	movq	__imp_GetTickCount64(%rip), %rbp
	movq	%rax, 40(%rsp)
	.p2align 4,,10
	.p2align 3
.L95:
	movl	(%rdi), %ebx
	call	*%rbp
	vpxor	%xmm2, %xmm2, %xmm2
	movq	40(%rsp), %rdx
	movq	32(%rsp), %rcx
	vmovdqu	%ymm2, 116(%rsp)
	movq	%rax, %r15
	movl	$-1, %r8d
	movl	$0, 148(%rsp)
	movq	144(%rsp), %rax
	leal	100(%r15), %esi
	movl	$1, 112(%rsp)
	movslq	%esi, %rsi
	movl	%r12d, 120(%rsp)
	vmovdqu	112(%rsp), %ymm0
	movq	%rax, 80(%rsp)
	vmovdqa	%ymm0, 48(%rsp)
	vzeroupper
	call	scheduler_send_remote
	jmp	.L93
	.p2align 4,,10
	.p2align 3
.L101:
	call	*%rbp
	cmpq	%rsi, %rax
	jnb	.L92
.L93:
	movl	(%rdi), %eax
	cmpl	%eax, %ebx
	jge	.L101
.L92:
	call	*%rbp
	subl	%r15d, %eax
	addl	%r14d, %eax
	movl	(%rdi), %edx
	cmpl	%edx, %ebx
	cmovl	%eax, %r14d
	leal	1(%r13), %eax
	cmovl	%eax, %r13d
	addl	$1, %r12d
	cmpl	$1000, %r12d
	jne	.L95
	vxorps	%xmm6, %xmm6, %xmm6
	call	scheduler_stop
	call	scheduler_wait
	movl	$1000, %r8d
	movl	%r13d, %edx
	leaq	.LC23(%rip), %rcx
	vcvtsi2sdl	%r13d, %xmm6, %xmm7
	vmulsd	.LC3(%rip), %xmm7, %xmm0
	vdivsd	.LC2(%rip), %xmm0, %xmm4
	vmovq	%xmm4, %r9
	vmovapd	%xmm4, %xmm3
	call	printf
	testl	%r13d, %r13d
	vxorpd	%xmm1, %xmm1, %xmm1
	je	.L96
	vcvtsi2sdl	%r14d, %xmm6, %xmm6
	vdivsd	%xmm7, %xmm6, %xmm1
.L96:
	vmovq	%xmm1, %rdx
	leaq	.LC24(%rip), %rcx
	call	printf
	vmovsd	.LC26(%rip), %xmm1
	movabsq	$0x3f50624dd2f1a9fc, %rdx
	leaq	.LC25(%rip), %rcx
	call	printf
	movq	32(%rsp), %rcx
	call	free
	movq	.refptr.schedulers(%rip), %rbx
	movq	16(%rbx), %rcx
	call	free
	movq	3170640(%rbx), %rcx
	vmovaps	160(%rsp), %xmm6
	vmovaps	176(%rsp), %xmm7
	addq	$200, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	jmp	free
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC27:
	.ascii "\12=== Contention Test (Many-to-One) ===\0"
	.align 8
.LC29:
	.ascii "Senders: %d cores \342\206\222 1 target\12\0"
.LC31:
	.ascii "Dropped: %d (%.1f%%)\12\0"
	.text
	.p2align 4
	.globl	bench_contention
	.def	bench_contention;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_contention
bench_contention:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$200, %rsp
	.seh_stackalloc	200
	vmovaps	%xmm6, 160(%rsp)
	.seh_savexmm	%xmm6, 160
	vmovaps	%xmm7, 176(%rsp)
	.seh_savexmm	%xmm7, 176
	.seh_endprologue
	leaq	.LC27(%rip), %rcx
	call	printf
	movl	$4, %ecx
	call	scheduler_init
	movl	$81976, %ecx
	call	malloc
	movq	$100, (%rax)
	movq	%rax, %rsi
	leaq	bench_actor_step(%rip), %rax
	movq	%rax, 81952(%rsi)
	leaq	81960(%rsi), %r13
	xorl	%eax, %eax
	xchgl	0(%r13), %eax
	xorl	%edx, %edx
	movl	$10000, %ebp
	xorl	%edi, %edi
	movq	$0, 81936(%rsi)
	movq	%rsi, %rcx
	movl	$0, 81944(%rsi)
	call	scheduler_register_actor
	call	scheduler_start
	movq	__imp_GetTickCount64(%rip), %r14
	call	*%r14
	movq	%rax, %r15
.L103:
	addl	$1, %edi
	leal	-10000(%rbp), %ebx
	.p2align 4,,10
	.p2align 3
.L104:
	vpxor	%xmm4, %xmm4, %xmm4
	movl	%edi, %r8d
	leaq	48(%rsp), %rdx
	movq	%rsi, %rcx
	vmovdqu	%ymm4, 116(%rsp)
	movl	$0, 148(%rsp)
	movq	144(%rsp), %rax
	movl	%ebx, 120(%rsp)
	movl	$100, 112(%rsp)
	vmovdqu	112(%rsp), %ymm0
	movq	%rax, 80(%rsp)
	vmovdqa	%ymm0, 48(%rsp)
	vzeroupper
	call	scheduler_send_remote
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L104
	cmpl	$3, %edi
	leal	10000(%rbx), %ebp
	jne	.L103
	movq	__imp_Sleep(%rip), %rdi
	movslq	%r15d, %rbx
	jmp	.L105
	.p2align 4,,10
	.p2align 3
.L106:
	call	*%r14
	subq	%rbx, %rax
	cmpq	$9999, %rax
	ja	.L109
	movl	$1, %ecx
	call	*%rdi
.L105:
	movl	0(%r13), %eax
	cmpl	$29999, %eax
	jle	.L106
.L109:
	vxorps	%xmm6, %xmm6, %xmm6
	call	*%r14
	subl	%r15d, %eax
	vcvtsi2sdl	%eax, %xmm6, %xmm0
	movl	81960(%rsi), %ebx
	vdivsd	.LC2(%rip), %xmm0, %xmm1
	vmovsd	%xmm1, 40(%rsp)
	call	scheduler_stop
	call	scheduler_wait
	movl	$3, %edx
	leaq	.LC29(%rip), %rcx
	call	printf
	movl	$30000, %r8d
	movl	%ebx, %edx
	leaq	.LC5(%rip), %rcx
	vcvtsi2sdl	%ebx, %xmm6, %xmm7
	vmulsd	.LC3(%rip), %xmm7, %xmm3
	vdivsd	.LC30(%rip), %xmm3, %xmm5
	vmovq	%xmm5, %r9
	vmovapd	%xmm5, %xmm3
	call	printf
	vmovsd	40(%rsp), %xmm1
	leaq	.LC6(%rip), %rcx
	vmovq	%xmm1, %rdx
	call	printf
	leaq	.LC7(%rip), %rcx
	vdivsd	40(%rsp), %xmm7, %xmm1
	vmovq	%xmm1, %rdx
	call	printf
	movl	$30000, %edx
	leaq	.LC31(%rip), %rcx
	subl	%ebx, %edx
	vcvtsi2sdl	%edx, %xmm6, %xmm0
	vmulsd	.LC3(%rip), %xmm0, %xmm0
	vdivsd	.LC30(%rip), %xmm0, %xmm2
	vmovq	%xmm2, %r8
	call	printf
	movq	%rsi, %rcx
	call	free
	movq	.refptr.schedulers(%rip), %rbx
	leaq	12682496(%rbx), %rsi
.L107:
	movq	16(%rbx), %rcx
	addq	$3170624, %rbx
	call	free
	cmpq	%rsi, %rbx
	jne	.L107
	vmovaps	160(%rsp), %xmm6
	vmovaps	176(%rsp), %xmm7
	addq	$200, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC32:
	.ascii "Perfect\0"
.LC33:
	.ascii "Partial\0"
.LC34:
	.ascii "\12=== Burst Pattern Test ===\0"
.LC35:
	.ascii "Bursts: %d \303\227 %d messages\12\0"
.LC37:
	.ascii "Recovery: %s\12\0"
	.text
	.p2align 4
	.globl	bench_burst_patterns
	.def	bench_burst_patterns;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_burst_patterns
bench_burst_patterns:
	pushq	%r15
	.seh_pushreg	%r15
	pushq	%r14
	.seh_pushreg	%r14
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$184, %rsp
	.seh_stackalloc	184
	vmovaps	%xmm6, 160(%rsp)
	.seh_savexmm	%xmm6, 160
	.seh_endprologue
	leaq	.LC34(%rip), %rcx
	call	printf
	movl	$2, %ecx
	call	scheduler_init
	movl	$81976, %ecx
	call	malloc
	movq	$1, (%rax)
	movq	%rax, %rdi
	leaq	bench_actor_step(%rip), %rax
	movq	%rax, 81952(%rdi)
	leaq	81960(%rdi), %r13
	xorl	%eax, %eax
	xchgl	0(%r13), %eax
	xorl	%edx, %edx
	movl	$500, %esi
	movq	$0, 81936(%rdi)
	movq	%rdi, %rcx
	leaq	48(%rsp), %rbp
	movl	$0, 81944(%rdi)
	call	scheduler_register_actor
	call	scheduler_start
	movq	__imp_GetTickCount64(%rip), %r14
	call	*%r14
	movq	__imp_Sleep(%rip), %r12
	movq	%rax, %r15
.L114:
	leal	-500(%rsi), %ebx
	.p2align 4,,10
	.p2align 3
.L115:
	vpxor	%xmm2, %xmm2, %xmm2
	movl	$-1, %r8d
	movq	%rbp, %rdx
	movq	%rdi, %rcx
	vmovdqu	%ymm2, 116(%rsp)
	movl	$0, 148(%rsp)
	movq	144(%rsp), %rax
	movl	%ebx, 120(%rsp)
	addl	$1, %ebx
	movl	$1, 112(%rsp)
	vmovdqu	112(%rsp), %ymm0
	movq	%rax, 80(%rsp)
	vmovdqa	%ymm0, 48(%rsp)
	vzeroupper
	call	scheduler_send_remote
	cmpl	%ebx, %esi
	jne	.L115
	addl	$500, %esi
	movl	$50, %ecx
	call	*%r12
	cmpl	$5500, %esi
	jne	.L114
	movslq	%r15d, %rbx
	jmp	.L116
.L117:
	call	*%r14
	subq	%rbx, %rax
	cmpq	$9999, %rax
	ja	.L121
	movl	$10, %ecx
	call	*%r12
.L116:
	movl	0(%r13), %eax
	cmpl	$4999, %eax
	jle	.L117
.L121:
	vxorps	%xmm6, %xmm6, %xmm6
	call	*%r14
	subl	%r15d, %eax
	vcvtsi2sdl	%eax, %xmm6, %xmm1
	movl	81960(%rdi), %ebx
	vdivsd	.LC2(%rip), %xmm1, %xmm1
	vmovsd	%xmm1, 40(%rsp)
	call	scheduler_stop
	call	scheduler_wait
	movl	$500, %r8d
	movl	$10, %edx
	leaq	.LC35(%rip), %rcx
	call	printf
	movl	$5000, %r8d
	movl	%ebx, %edx
	leaq	.LC5(%rip), %rcx
	vcvtsi2sdl	%ebx, %xmm6, %xmm6
	vmulsd	.LC3(%rip), %xmm6, %xmm0
	vdivsd	.LC36(%rip), %xmm0, %xmm4
	vmovq	%xmm4, %r9
	vmovapd	%xmm4, %xmm3
	call	printf
	vmovsd	40(%rsp), %xmm1
	leaq	.LC6(%rip), %rcx
	vmovq	%xmm1, %rdx
	call	printf
	leaq	.LC7(%rip), %rcx
	vdivsd	40(%rsp), %xmm6, %xmm1
	vmovq	%xmm1, %rdx
	call	printf
	cmpl	$5000, %ebx
	leaq	.LC33(%rip), %rax
	leaq	.LC32(%rip), %rdx
	cmovne	%rax, %rdx
	leaq	.LC37(%rip), %rcx
	call	printf
	movq	%rdi, %rcx
	call	free
	movq	.refptr.schedulers(%rip), %rbx
	movq	16(%rbx), %rcx
	call	free
	movq	3170640(%rbx), %rcx
	vmovaps	160(%rsp), %xmm6
	addq	$184, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	jmp	free
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC38:
	.ascii "\12=== Mailbox Saturation Test ===\0"
.LC39:
	.ascii "Messages sent: %d\12\0"
	.align 8
.LC40:
	.ascii "Messages processed: %d (%.1f%%)\12\0"
	.align 8
.LC41:
	.ascii "Messages dropped: %d (%.1f%%)\12\0"
	.text
	.p2align 4
	.globl	bench_mailbox_saturation
	.def	bench_mailbox_saturation;	.scl	2;	.type	32;	.endef
	.seh_proc	bench_mailbox_saturation
bench_mailbox_saturation:
	pushq	%r13
	.seh_pushreg	%r13
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$200, %rsp
	.seh_stackalloc	200
	vmovaps	%xmm6, 160(%rsp)
	.seh_savexmm	%xmm6, 160
	vmovaps	%xmm7, 176(%rsp)
	.seh_savexmm	%xmm7, 176
	.seh_endprologue
	leaq	.LC38(%rip), %rcx
	call	puts
	movl	$2, %ecx
	call	scheduler_init
	movl	$81976, %ecx
	call	malloc
	movq	$1, (%rax)
	movq	%rax, %rsi
	leaq	bench_actor_step(%rip), %rax
	movq	%rax, 81952(%rsi)
	leaq	81960(%rsi), %rbp
	xorl	%eax, %eax
	xchgl	0(%rbp), %eax
	xorl	%edx, %edx
	xorl	%ebx, %ebx
	movq	$0, 81936(%rsi)
	movq	%rsi, %rcx
	movl	$0, 81944(%rsi)
	call	scheduler_register_actor
	call	scheduler_start
	movq	__imp_GetTickCount64(%rip), %r12
	call	*%r12
	movq	%rax, %r13
	.p2align 4,,10
	.p2align 3
.L125:
	vpxor	%xmm3, %xmm3, %xmm3
	movl	$-1, %r8d
	leaq	48(%rsp), %rdx
	movq	%rsi, %rcx
	vmovdqu	%ymm3, 116(%rsp)
	movl	$0, 148(%rsp)
	movq	144(%rsp), %rax
	movl	%ebx, 120(%rsp)
	movl	$1, 112(%rsp)
	vmovdqu	112(%rsp), %ymm0
	movq	%rax, 80(%rsp)
	vmovdqa	%ymm0, 48(%rsp)
	vzeroupper
	call	scheduler_send_remote
	addl	$1, %ebx
	cmpl	$5000, %ebx
	jne	.L125
	movq	__imp_Sleep(%rip), %rdi
	movslq	%r13d, %rbx
	jmp	.L126
	.p2align 4,,10
	.p2align 3
.L127:
	call	*%r12
	subq	%rbx, %rax
	cmpq	$9999, %rax
	ja	.L129
	movl	$10, %ecx
	call	*%rdi
.L126:
	movl	0(%rbp), %eax
	cmpl	$4999, %eax
	jle	.L127
.L129:
	vxorps	%xmm6, %xmm6, %xmm6
	movl	$5000, %ebx
	call	*%r12
	subl	%r13d, %eax
	vcvtsi2sdl	%eax, %xmm6, %xmm0
	movl	81960(%rsi), %edi
	subl	%edi, %ebx
	vdivsd	.LC2(%rip), %xmm0, %xmm1
	vmovsd	%xmm1, 40(%rsp)
	call	scheduler_stop
	call	scheduler_wait
	movl	$5000, %edx
	leaq	.LC39(%rip), %rcx
	call	printf
	movl	%edi, %edx
	leaq	.LC40(%rip), %rcx
	vcvtsi2sdl	%edi, %xmm6, %xmm7
	vmulsd	.LC3(%rip), %xmm7, %xmm2
	vdivsd	.LC36(%rip), %xmm2, %xmm4
	vmovq	%xmm4, %r8
	vmovapd	%xmm4, %xmm2
	call	printf
	movl	%ebx, %edx
	leaq	.LC41(%rip), %rcx
	vcvtsi2sdl	%ebx, %xmm6, %xmm0
	vmulsd	.LC3(%rip), %xmm0, %xmm0
	vdivsd	.LC36(%rip), %xmm0, %xmm5
	vmovq	%xmm5, %r8
	vmovapd	%xmm5, %xmm2
	call	printf
	vmovsd	40(%rsp), %xmm1
	leaq	.LC6(%rip), %rcx
	vmovq	%xmm1, %rdx
	call	printf
	leaq	.LC7(%rip), %rcx
	vdivsd	40(%rsp), %xmm7, %xmm1
	vmovq	%xmm1, %rdx
	call	printf
	movq	%rsi, %rcx
	call	free
	movq	.refptr.schedulers(%rip), %rbx
	movq	16(%rbx), %rcx
	call	free
	movq	3170640(%rbx), %rcx
	vmovaps	160(%rsp), %xmm6
	vmovaps	176(%rsp), %xmm7
	addq	$200, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	popq	%r13
	jmp	free
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC42:
	.ascii "===============================================================\0"
	.align 8
.LC43:
	.ascii "        Aether Scheduler Performance Benchmarks               \0"
	.align 8
.LC44:
	.ascii "\12===============================================================\0"
	.align 8
.LC45:
	.ascii "                    Benchmarks Complete                        \0"
	.section	.text.startup,"x"
	.p2align 4
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
	subq	$40, %rsp
	.seh_stackalloc	40
	.seh_endprologue
	call	__main
	leaq	.LC42(%rip), %rcx
	call	puts
	leaq	.LC43(%rip), %rcx
	call	puts
	leaq	.LC42(%rip), %rcx
	call	puts
	call	bench_single_core_throughput
	movl	$2, %ecx
	call	bench_multi_core_throughput
	movl	$4, %ecx
	call	bench_multi_core_throughput
	call	bench_latency
	call	bench_cross_core_overhead
	call	bench_contention
	call	bench_burst_patterns
	call	bench_mailbox_saturation
	call	bench_scalability
	leaq	.LC44(%rip), %rcx
	call	puts
	leaq	.LC45(%rip), %rcx
	call	puts
	leaq	.LC42(%rip), %rcx
	call	puts
	xorl	%eax, %eax
	addq	$40, %rsp
	ret
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC2:
	.long	0
	.long	1083129856
	.align 8
.LC3:
	.long	0
	.long	1079574528
	.align 8
.LC4:
	.long	0
	.long	1090021888
	.align 8
.LC14:
	.long	0
	.long	1086556160
	.align 8
.LC26:
	.long	-755914244
	.long	1062232653
	.align 8
.LC30:
	.long	0
	.long	1088244736
	.align 8
.LC36:
	.long	0
	.long	1085507584
	.def	__main;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (MinGW-W64 x86_64-ucrt-posix-seh, built by Brecht Sanders, r4) 15.2.0"
	.def	puts;	.scl	2;	.type	32;	.endef
	.def	scheduler_init;	.scl	2;	.type	32;	.endef
	.def	malloc;	.scl	2;	.type	32;	.endef
	.def	scheduler_register_actor;	.scl	2;	.type	32;	.endef
	.def	scheduler_start;	.scl	2;	.type	32;	.endef
	.def	scheduler_send_remote;	.scl	2;	.type	32;	.endef
	.def	scheduler_stop;	.scl	2;	.type	32;	.endef
	.def	scheduler_wait;	.scl	2;	.type	32;	.endef
	.def	printf;	.scl	2;	.type	32;	.endef
	.def	free;	.scl	2;	.type	32;	.endef
	.section	.rdata$.refptr.schedulers, "dr"
	.p2align	3, 0
	.globl	.refptr.schedulers
	.linkonce	discard
.refptr.schedulers:
	.quad	schedulers
