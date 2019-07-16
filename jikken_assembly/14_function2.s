;	GCC For the Hitachi H8/300
;	By Hitachi America Ltd and Cygnus Support


	.file	"14_function2.c"
	.section .text
	.align 1
	.global _func1
_func1:
	mov.w	r6,@-r7
	mov.w	r7,r6
	mov.w	#-6,r3
	add.w	r3,r7
	jsr	@_func2
	mov.w	r0,r2
	mov.w	r2,@(-4,r6)
	mov.w	@(-4,r6),r2
	mov.w	r2,r0
	mov.w	#6,r3
	add.w	r3,r7
	mov.w	@r7+,r6
	rts
	.align 1
	.global _func2
_func2:
	mov.w	r6,@-r7
	mov.w	r7,r6
	mov.w	#5,r2
	mov.w	r2,r0
	mov.w	@r7+,r6
	rts
	.align 1
	.global _main
_main:
	mov.w	r6,@-r7
	mov.w	r7,r6
	subs	#2,r7
	jsr	@_func1
	mov.w	r0,r2
	mov.w	r2,@(-2,r6)
	sub.w	r2,r2
	mov.w	r2,r0
	adds	#2,r7
	mov.w	@r7+,r6
	rts
	.end
	.ident	"GCC: (GNU) 3.3.6"
