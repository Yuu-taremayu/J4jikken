;	GCC For the Hitachi H8/300
;	By Hitachi America Ltd and Cygnus Support


	.file	"13_for.c"
	.section .text
	.align 1
	.global _main
_main:
	mov.w	r6,@-r7
	mov.w	r7,r6
	subs	#2,r7
	sub.w	r2,r2
	mov.w	r2,@(-2,r6)
.L2:
	mov.w	@(-2,r6),r3
	mov.w	#9,r2
	cmp.w	r2,r3
	ble	.L4
	bra	.L3
.L4:
	mov.w	@(-2,r6),r2
	adds	#1,r2
	mov.w	r2,@(-2,r6)
	bra	.L2
.L3:
	sub.w	r2,r2
	mov.w	r2,r0
	adds	#2,r7
	mov.w	@r7+,r6
	rts
	.end
	.ident	"GCC: (GNU) 3.3.6"
