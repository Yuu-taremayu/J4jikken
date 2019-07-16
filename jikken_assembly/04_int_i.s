;	GCC For the Hitachi H8/300
;	By Hitachi America Ltd and Cygnus Support


	.file	"04_int_i.c"
	.section .text
	.align 1
	.global _main
_main:
	mov.w	r6,@-r7
	mov.w	r7,r6
	subs	#2,r7
	sub.w	r2,r2
	mov.w	r2,r0
	adds	#2,r7
	mov.w	@r7+,r6
	rts
	.end
	.ident	"GCC: (GNU) 3.3.6"
