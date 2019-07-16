;	GCC For the Hitachi H8/300
;	By Hitachi America Ltd and Cygnus Support


	.file	"06_int_i_j_k.c"
	.section .text
	.align 1
	.global _main
_main:
	mov.w	r6,@-r7
	mov.w	r7,r6
	mov.w	#-6,r3
	add.w	r3,r7
	sub.w	r2,r2
	mov.w	r2,r0
	mov.w	#6,r3
	add.w	r3,r7
	mov.w	@r7+,r6
	rts
	.end
	.ident	"GCC: (GNU) 3.3.6"
