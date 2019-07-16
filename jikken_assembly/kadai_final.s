;	GCC For the Hitachi H8/300
;	By Hitachi America Ltd and Cygnus Support


	.file	"kadai_final.c"
	.section .text
	.align 1
	.global _sub_1
_sub_1:
	mov.w	r6,@-r7
	mov.w	r7,r6
	subs	#2,r7
	sub.w	r2,r2
	mov.w	r2,@(-2,r6)
	adds	#2,r7
	mov.w	@r7+,r6
	rts
	.align 1
	.global _sub_2
_sub_2:
	mov.w	r6,@-r7
	mov.w	r7,r6
	sub.w	r2,r2
	mov.w	r2,r0
	mov.w	@r7+,r6
	rts
	.align 1
	.global _sub_3
_sub_3:
	mov.w	r6,@-r7
	mov.w	r7,r6
	subs	#2,r7
	mov.w	#1,r2
	mov.w	r2,@(-2,r6)
	mov.w	@(-2,r6),r2
	mov.w	r2,r0
	adds	#2,r7
	mov.w	@r7+,r6
	rts
	.align 1
	.global _sub_4
_sub_4:
	mov.w	r6,@-r7
	mov.w	r7,r6
	subs	#2,r7
	subs	#2,r7
	mov.w	r0,@(-2,r6)
	mov.w	@(-2,r6),r2
	mov.w	r2,r2
	bne	.L5
	mov.w	#1,r2
	mov.w	r2,@(-4,r6)
	bra	.L6
.L5:
	mov.w	@(-2,r6),r3
	mov.w	#1,r2
	cmp.w	r2,r3
	bne	.L7
	mov.w	#10,r2
	mov.w	r2,@(-4,r6)
	bra	.L6
.L7:
	mov.w	#100,r2
	mov.w	r2,@(-4,r6)
.L6:
	mov.w	@(-4,r6),r2
	mov.w	r2,r0
	adds	#2,r7
	adds	#2,r7
	mov.w	@r7+,r6
	rts
	.align 1
	.global _sub_5
_sub_5:
	mov.w	r6,@-r7
	mov.w	r7,r6
	mov.w	#-6,r3
	add.w	r3,r7
	mov.w	r0,@(-2,r6)
	sub.w	r2,r2
	mov.w	r2,@(-6,r6)
	sub.w	r2,r2
	mov.w	r2,@(-4,r6)
.L10:
	mov.w	@(-4,r6),r3
	mov.w	@(-2,r6),r2
	cmp.w	r2,r3
	blt	.L13
	bra	.L11
.L13:
	mov.w	@(-6,r6),r3
	mov.w	@(-4,r6),r2
	add.w	r3,r2
	mov.w	r2,@(-6,r6)
	mov.w	@(-4,r6),r2
	adds	#1,r2
	mov.w	r2,@(-4,r6)
	bra	.L10
.L11:
	mov.w	@(-6,r6),r2
	mov.w	r2,r0
	mov.w	#6,r3
	add.w	r3,r7
	mov.w	@r7+,r6
	rts
	.align 1
	.global _main
_main:
	mov.w	r6,@-r7
	mov.w	r7,r6
	subs	#2,r7
	jsr	@_sub_1
	jsr	@_sub_2
	jsr	@_sub_3
	mov.w	r0,r2
	mov.w	r2,@(-2,r6)
	mov.w	@(-2,r6),r0
	jsr	@_sub_4
	mov.w	r0,r2
	mov.w	r2,@(-2,r6)
	mov.w	@(-2,r6),r0
	jsr	@_sub_5
	mov.w	r0,r2
	mov.w	r2,@(-2,r6)
	mov.w	@(-2,r6),r2
	mov.w	r2,r0
	adds	#2,r7
	mov.w	@r7+,r6
	rts
	.end
	.ident	"GCC: (GNU) 3.3.6"
