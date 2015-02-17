	.arch armv7-a
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"hacc_export.c"
	.text
.Ltext0:
	.align	2
	.type	sp_hacc_internal.isra.0, %function
sp_hacc_internal.isra.0:
.LFB187:
	.file 1 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/mach/hacc_export.c"
	.loc 1 91 0
	@ args = 8, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL0:
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {r4, r5, r6, r7, r8, fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
	sub	sp, sp, #12
.LVL1:
	.loc 1 91 0
	mov	r4, r0
	.loc 1 95 0
	mov	r0, #62
.LVL2:
	.loc 1 91 0
	mov	r7, r1
	mov	r6, r2
	mov	r5, r3
	ldrb	r8, [fp, #8]	@ zero_extendqisi2
	.loc 1 95 0
	bl	clock_is_on
.LVL3:
	cmp	r0, #0
	beq	.L26
	.loc 1 103 0
	cmp	r5, #1
	beq	.L3
.LVL4:
.L11:
	.loc 1 118 0
	cmp	r6, #3
	ldrls	pc, [pc, r6, asl #2]
	b	.L16
.L9:
	.word	.L5
	.word	.L6
	.word	.L7
	.word	.L8
.L8:
	.loc 1 168 0
	adds	r0, r8, #0
	ldr	r1, .L28
	movne	r0, #1
.L23:
	bl	HACC_V3_Init
.LVL5:
	.loc 1 170 0
	mov	r0, r4
	mov	r1, r7
	mov	r2, r4
	bl	HACC_V3_Run
.LVL6:
	.loc 1 172 0
	bl	HACC_V3_Terminate
.LVL7:
	.loc 1 183 0
	cmp	r5, #1
	beq	.L27
.L13:
	.loc 1 188 0
	mov	r0, #62
	bl	clock_is_on
.LVL8:
	cmp	r0, #0
	beq	.L20
	.loc 1 190 0
	ldr	r1, .L28+4
	mov	r0, #62
	bl	disable_clock
.LVL9:
.L20:
	.loc 1 206 0
	mov	r0, r4
	sub	sp, fp, #32
	ldmfd	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
.L7:
	.loc 1 146 0
	ldr	r3, .L28+8
	ldrb	r3, [r3, #0]	@ zero_extendqisi2
	cmp	r3, #0
	beq	.L18
	.loc 1 153 0
	mov	r0, #2
	mov	r1, #32
	bl	hacc_set_key
.LVL10:
	.loc 1 155 0
	subs	r2, r0, #0
	bne	.L4
	.loc 1 158 0
	mov	r2, r4
	ldr	r0, [fp, #4]
.LVL11:
	mov	r1, r4
	bic	r3, r7, #127
	bl	hacc_do_aes
.LVL12:
	.loc 1 160 0
	subs	r2, r0, #0
	bne	.L4
	.loc 1 183 0
	cmp	r5, #1
	bne	.L13
.LVL13:
.L27:
	.loc 1 185 0
	bl	osal_hacc_unlock
.LVL14:
	b	.L13
.L6:
	.loc 1 135 0
	adds	r0, r8, #0
	ldr	r1, .L28+12
	movne	r0, #1
	b	.L23
.L5:
	.loc 1 124 0
	adds	r0, r8, #0
	ldr	r1, .L28+16
	movne	r0, #1
	b	.L23
.L16:
	.loc 1 176 0
	movw	r2, #24588
.L4:
.LVL15:
	.loc 1 196 0
	cmp	r5, #1
	beq	.L10
.LVL16:
.L15:
	.loc 1 201 0
	ldr	r1, .L28+20
	ldr	r0, .L28+24
	bl	printk
.LVL17:
	.loc 1 203 0
#APP
@ 203 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/mach/hacc_export.c" 1
	.word 0xe7f001f2
@ 0 "" 2
.LVL18:
.L26:
	.loc 1 97 0
	mov	r0, #62
	ldr	r1, .L28+4
	bl	enable_clock
.LVL19:
	.loc 1 103 0
	cmp	r5, #1
	bne	.L11
.L3:
	.loc 1 106 0
	bl	osal_hacc_lock
.LVL20:
	.loc 1 108 0
	cmp	r0, #0
	beq	.L11
	.loc 1 110 0
	movw	r2, #24586
.LVL21:
.L10:
	.loc 1 198 0
	str	r2, [fp, #-40]
	bl	osal_hacc_unlock
.LVL22:
	ldr	r2, [fp, #-40]
	b	.L15
.L18:
	.loc 1 148 0
	movw	r2, #24583
	b	.L4
.L29:
	.align	2
.L28:
	.word	.LANCHOR0+64
	.word	.LC0
	.word	.LANCHOR1
	.word	.LANCHOR0+32
	.word	.LANCHOR0
	.word	.LC2
	.word	.LC1
.LFE187:
	.size	sp_hacc_internal.isra.0, .-sp_hacc_internal.isra.0
	.align	2
	.global	masp_hal_sp_hacc_enc
	.type	masp_hal_sp_hacc_enc, %function
masp_hal_sp_hacc_enc:
.LFB183:
	.loc 1 212 0
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL23:
	mov	ip, sp
.LCFI3:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI4:
	sub	fp, ip, #4
.LCFI5:
	sub	sp, sp, #8
	.loc 1 213 0
	mov	ip, #1
	mov	r2, r3
.LVL24:
	str	ip, [sp, #0]
	str	ip, [sp, #4]
	ldrb	r3, [fp, #4]	@ zero_extendqisi2
.LVL25:
	bl	sp_hacc_internal.isra.0
.LVL26:
	.loc 1 214 0
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.LFE183:
	.size	masp_hal_sp_hacc_enc, .-masp_hal_sp_hacc_enc
	.align	2
	.global	masp_hal_sp_hacc_dec
	.type	masp_hal_sp_hacc_dec, %function
masp_hal_sp_hacc_dec:
.LFB184:
	.loc 1 221 0
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL27:
	mov	ip, sp
.LCFI6:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI7:
	sub	fp, ip, #4
.LCFI8:
	sub	sp, sp, #8
	.loc 1 222 0
	mov	ip, #0
	mov	r2, r3
.LVL28:
	str	ip, [sp, #0]
	str	ip, [sp, #4]
	ldrb	r3, [fp, #4]	@ zero_extendqisi2
.LVL29:
	bl	sp_hacc_internal.isra.0
.LVL30:
	.loc 1 223 0
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.LFE184:
	.size	masp_hal_sp_hacc_dec, .-masp_hal_sp_hacc_dec
	.align	2
	.global	masp_hal_sp_hacc_blk_sz
	.type	masp_hal_sp_hacc_blk_sz, %function
masp_hal_sp_hacc_blk_sz:
.LFB185:
	.loc 1 229 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI9:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI10:
	sub	fp, ip, #4
.LCFI11:
	.loc 1 231 0
	mov	r0, #16
	ldmfd	sp, {fp, sp, pc}
.LFE185:
	.size	masp_hal_sp_hacc_blk_sz, .-masp_hal_sp_hacc_blk_sz
	.align	2
	.global	masp_hal_sp_hacc_init
	.type	masp_hal_sp_hacc_init, %function
masp_hal_sp_hacc_init:
.LFB186:
	.loc 1 237 0
	@ args = 0, pretend = 0, frame = 40
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL31:
	mov	ip, sp
.LCFI12:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI13:
	sub	fp, ip, #4
.LCFI14:
	sub	sp, sp, #48
.LVL32:
	.loc 1 241 0
	cmp	r1, #16
	.loc 1 243 0
	movwne	r0, #24580
.LVL33:
	.loc 1 241 0
	beq	.L38
.LVL34:
.L34:
	.loc 1 256 0
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.LVL35:
.L38:
	sub	r1, fp, #12
.LVL36:
	.loc 1 247 0
	mov	r3, #0
	.loc 1 246 0
	mov	r2, #32
	str	r2, [fp, #-48]
.LVL37:
.L35:
	.loc 1 236 0 discriminator 2
	sub	ip, fp, #48
	.loc 1 249 0 discriminator 2
	ldrb	r2, [r0, r3]	@ zero_extendqisi2
	.loc 1 236 0 discriminator 2
	add	lr, ip, r3
	.loc 1 247 0 discriminator 2
	add	r3, r3, #1
.LVL38:
	cmp	r3, #16
	.loc 1 250 0 discriminator 2
	add	ip, r2, #1
	.loc 1 249 0 discriminator 2
	strb	r2, [lr, #4]
	.loc 1 250 0 discriminator 2
	strb	ip, [r1, #-1]!
	.loc 1 247 0 discriminator 2
	bne	.L35
	.loc 1 253 0 discriminator 1
	ldrb	ip, [fp, #-41]	@ zero_extendqisi2
	ldrb	r1, [fp, #-44]	@ zero_extendqisi2
	ldrb	r2, [fp, #-43]	@ zero_extendqisi2
	ldrb	r3, [fp, #-42]	@ zero_extendqisi2
.LVL39:
	str	ip, [sp, #0]
	ldr	r0, .L39
	bl	printk
.LVL40:
	.loc 1 255 0 discriminator 1
	sub	r0, fp, #48
	bl	hacc_init
.LVL41:
	b	.L34
.L40:
	.align	2
.L39:
	.word	.LC3
.LFE186:
	.size	masp_hal_sp_hacc_init, .-masp_hal_sp_hacc_init
	.global	bHACC_SWKeyInit
	.global	bHACC_HWWrapKeyInit
	.section	.rodata
	.align	2
.LANCHOR0 = . + 0
	.type	g_HACC_CFG_1, %object
	.size	g_HACC_CFG_1, 32
g_HACC_CFG_1:
	.word	-1630272512
	.word	15238305
	.word	-470776899
	.word	793669002
	.word	-8155556
	.word	-381640477
	.word	-1924272954
	.word	1174145417
	.type	g_HACC_CFG_2, %object
	.size	g_HACC_CFG_2, 32
g_HACC_CFG_2:
	.word	-1437324070
	.word	1431445780
	.word	-470776899
	.word	1431445780
	.word	-1437324070
	.word	-1437324070
	.word	1431445780
	.word	-1437324070
	.type	g_HACC_CFG_3, %object
	.size	g_HACC_CFG_3, 32
g_HACC_CFG_3:
	.word	646231696
	.word	-345528130
	.word	-1592585140
	.word	393941525
	.word	378269286
	.word	310687364
	.word	-549728710
	.word	561999778
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"MASP\000"
	.space	3
.LC1:
	.ascii	"[%s] HACC Fail (0x%x)\012\000"
	.space	1
.LC2:
	.ascii	"ASF\000"
.LC3:
	.ascii	"0x%x,0x%x,0x%x,0x%x\012\000"
	.bss
.LANCHOR1 = . + 0
	.type	bHACC_HWWrapKeyInit, %object
	.size	bHACC_HWWrapKeyInit, 1
bHACC_HWWrapKeyInit:
	.space	1
	.type	bHACC_SWKeyInit, %object
	.size	bHACC_SWKeyInit, 1
bHACC_SWKeyInit:
	.space	1
	.section	.debug_frame,"",%progbits
.Lframe0:
	.4byte	.LECIE0-.LSCIE0
.LSCIE0:
	.4byte	0xffffffff
	.byte	0x1
	.ascii	"\000"
	.uleb128 0x1
	.sleb128 -4
	.byte	0xe
	.byte	0xc
	.uleb128 0xd
	.uleb128 0
	.align	2
.LECIE0:
.LSFDE0:
	.4byte	.LEFDE0-.LASFDE0
.LASFDE0:
	.4byte	.Lframe0
	.4byte	.LFB187
	.4byte	.LFE187-.LFB187
	.byte	0x4
	.4byte	.LCFI0-.LFB187
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI1-.LCFI0
	.byte	0x84
	.uleb128 0x9
	.byte	0x85
	.uleb128 0x8
	.byte	0x86
	.uleb128 0x7
	.byte	0x87
	.uleb128 0x6
	.byte	0x88
	.uleb128 0x5
	.byte	0x8b
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8e
	.uleb128 0x2
	.byte	0x4
	.4byte	.LCFI2-.LCFI1
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE0:
.LSFDE2:
	.4byte	.LEFDE2-.LASFDE2
.LASFDE2:
	.4byte	.Lframe0
	.4byte	.LFB183
	.4byte	.LFE183-.LFB183
	.byte	0x4
	.4byte	.LCFI3-.LFB183
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI4-.LCFI3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8e
	.uleb128 0x2
	.byte	0x4
	.4byte	.LCFI5-.LCFI4
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE2:
.LSFDE4:
	.4byte	.LEFDE4-.LASFDE4
.LASFDE4:
	.4byte	.Lframe0
	.4byte	.LFB184
	.4byte	.LFE184-.LFB184
	.byte	0x4
	.4byte	.LCFI6-.LFB184
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI7-.LCFI6
	.byte	0x8b
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8e
	.uleb128 0x2
	.byte	0x4
	.4byte	.LCFI8-.LCFI7
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE4:
.LSFDE6:
	.4byte	.LEFDE6-.LASFDE6
.LASFDE6:
	.4byte	.Lframe0
	.4byte	.LFB185
	.4byte	.LFE185-.LFB185
	.byte	0x4
	.4byte	.LCFI9-.LFB185
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI10-.LCFI9
	.byte	0x8b
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8e
	.uleb128 0x2
	.byte	0x4
	.4byte	.LCFI11-.LCFI10
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE6:
.LSFDE8:
	.4byte	.LEFDE8-.LASFDE8
.LASFDE8:
	.4byte	.Lframe0
	.4byte	.LFB186
	.4byte	.LFE186-.LFB186
	.byte	0x4
	.4byte	.LCFI12-.LFB186
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI13-.LCFI12
	.byte	0x8b
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8e
	.uleb128 0x2
	.byte	0x4
	.4byte	.LCFI14-.LCFI13
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE8:
	.text
.Letext0:
	.file 2 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/../mediatek/platform/mt6571/kernel/core/include/mach/mt_typedefs.h"
	.file 3 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/../mediatek/kernel/include/mach/mt_sec_hal.h"
	.file 4 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/../mediatek/platform/mt6571/kernel/core/include/mach/mt_clkmgr.h"
	.file 5 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/asf/asf_inc/sec_typedef.h"
	.file 6 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/mach/hacc_mach.h"
	.file 7 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/arch/arm/include/asm/hwcap.h"
	.file 8 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/include/linux/printk.h"
	.file 9 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/include/linux/kernel.h"
	.file 10 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/../mediatek/kernel/include/mach/sec_osal.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0xb19
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF205
	.byte	0x1
	.4byte	.LASF206
	.4byte	.LASF207
	.4byte	.Ltext0
	.4byte	.Letext0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF1
	.uleb128 0x3
	.byte	0x4
	.4byte	0x39
	.uleb128 0x4
	.4byte	0x3e
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF2
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF4
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF5
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF6
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF7
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF8
	.uleb128 0x5
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF9
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF10
	.uleb128 0x3
	.byte	0x4
	.4byte	0x53
	.uleb128 0x6
	.ascii	"U8\000"
	.byte	0x2
	.byte	0x2a
	.4byte	0x53
	.uleb128 0x6
	.ascii	"U32\000"
	.byte	0x2
	.byte	0x2e
	.4byte	0x45
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.4byte	.LASF11
	.uleb128 0x3
	.byte	0x4
	.4byte	0x3e
	.uleb128 0x7
	.4byte	.LASF12
	.byte	0x2
	.byte	0x68
	.4byte	0x53
	.uleb128 0x8
	.byte	0x4
	.byte	0x3
	.byte	0x12
	.4byte	0xd8
	.uleb128 0x9
	.4byte	.LASF13
	.sleb128 0
	.uleb128 0x9
	.4byte	.LASF14
	.sleb128 1
	.uleb128 0x9
	.4byte	.LASF15
	.sleb128 2
	.uleb128 0x9
	.4byte	.LASF16
	.sleb128 3
	.byte	0
	.uleb128 0x7
	.4byte	.LASF17
	.byte	0x3
	.byte	0x17
	.4byte	0xb7
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.4byte	.LASF18
	.uleb128 0xa
	.4byte	.LASF161
	.byte	0x4
	.byte	0x4
	.2byte	0x284
	.4byte	0x491
	.uleb128 0x9
	.4byte	.LASF19
	.sleb128 0
	.uleb128 0x9
	.4byte	.LASF20
	.sleb128 0
	.uleb128 0x9
	.4byte	.LASF21
	.sleb128 1
	.uleb128 0x9
	.4byte	.LASF22
	.sleb128 1
	.uleb128 0x9
	.4byte	.LASF23
	.sleb128 2
	.uleb128 0x9
	.4byte	.LASF24
	.sleb128 2
	.uleb128 0x9
	.4byte	.LASF25
	.sleb128 3
	.uleb128 0x9
	.4byte	.LASF26
	.sleb128 4
	.uleb128 0x9
	.4byte	.LASF27
	.sleb128 5
	.uleb128 0x9
	.4byte	.LASF28
	.sleb128 6
	.uleb128 0x9
	.4byte	.LASF29
	.sleb128 7
	.uleb128 0x9
	.4byte	.LASF30
	.sleb128 8
	.uleb128 0x9
	.4byte	.LASF31
	.sleb128 9
	.uleb128 0x9
	.4byte	.LASF32
	.sleb128 10
	.uleb128 0x9
	.4byte	.LASF33
	.sleb128 11
	.uleb128 0x9
	.4byte	.LASF34
	.sleb128 12
	.uleb128 0x9
	.4byte	.LASF35
	.sleb128 13
	.uleb128 0x9
	.4byte	.LASF36
	.sleb128 14
	.uleb128 0x9
	.4byte	.LASF37
	.sleb128 15
	.uleb128 0x9
	.4byte	.LASF38
	.sleb128 16
	.uleb128 0x9
	.4byte	.LASF39
	.sleb128 16
	.uleb128 0x9
	.4byte	.LASF40
	.sleb128 17
	.uleb128 0x9
	.4byte	.LASF41
	.sleb128 17
	.uleb128 0x9
	.4byte	.LASF42
	.sleb128 18
	.uleb128 0x9
	.4byte	.LASF43
	.sleb128 19
	.uleb128 0x9
	.4byte	.LASF44
	.sleb128 20
	.uleb128 0x9
	.4byte	.LASF45
	.sleb128 21
	.uleb128 0x9
	.4byte	.LASF46
	.sleb128 22
	.uleb128 0x9
	.4byte	.LASF47
	.sleb128 23
	.uleb128 0x9
	.4byte	.LASF48
	.sleb128 24
	.uleb128 0x9
	.4byte	.LASF49
	.sleb128 25
	.uleb128 0x9
	.4byte	.LASF50
	.sleb128 26
	.uleb128 0x9
	.4byte	.LASF51
	.sleb128 27
	.uleb128 0x9
	.4byte	.LASF52
	.sleb128 28
	.uleb128 0x9
	.4byte	.LASF53
	.sleb128 28
	.uleb128 0x9
	.4byte	.LASF54
	.sleb128 29
	.uleb128 0x9
	.4byte	.LASF55
	.sleb128 29
	.uleb128 0x9
	.4byte	.LASF56
	.sleb128 30
	.uleb128 0x9
	.4byte	.LASF57
	.sleb128 31
	.uleb128 0x9
	.4byte	.LASF58
	.sleb128 32
	.uleb128 0x9
	.4byte	.LASF59
	.sleb128 33
	.uleb128 0x9
	.4byte	.LASF60
	.sleb128 34
	.uleb128 0x9
	.4byte	.LASF61
	.sleb128 35
	.uleb128 0x9
	.4byte	.LASF62
	.sleb128 36
	.uleb128 0x9
	.4byte	.LASF63
	.sleb128 37
	.uleb128 0x9
	.4byte	.LASF64
	.sleb128 38
	.uleb128 0x9
	.4byte	.LASF65
	.sleb128 39
	.uleb128 0x9
	.4byte	.LASF66
	.sleb128 40
	.uleb128 0x9
	.4byte	.LASF67
	.sleb128 41
	.uleb128 0x9
	.4byte	.LASF68
	.sleb128 42
	.uleb128 0x9
	.4byte	.LASF69
	.sleb128 43
	.uleb128 0x9
	.4byte	.LASF70
	.sleb128 43
	.uleb128 0x9
	.4byte	.LASF71
	.sleb128 44
	.uleb128 0x9
	.4byte	.LASF72
	.sleb128 44
	.uleb128 0x9
	.4byte	.LASF73
	.sleb128 45
	.uleb128 0x9
	.4byte	.LASF74
	.sleb128 46
	.uleb128 0x9
	.4byte	.LASF75
	.sleb128 47
	.uleb128 0x9
	.4byte	.LASF76
	.sleb128 48
	.uleb128 0x9
	.4byte	.LASF77
	.sleb128 49
	.uleb128 0x9
	.4byte	.LASF78
	.sleb128 50
	.uleb128 0x9
	.4byte	.LASF79
	.sleb128 51
	.uleb128 0x9
	.4byte	.LASF80
	.sleb128 52
	.uleb128 0x9
	.4byte	.LASF81
	.sleb128 53
	.uleb128 0x9
	.4byte	.LASF82
	.sleb128 54
	.uleb128 0x9
	.4byte	.LASF83
	.sleb128 55
	.uleb128 0x9
	.4byte	.LASF84
	.sleb128 56
	.uleb128 0x9
	.4byte	.LASF85
	.sleb128 57
	.uleb128 0x9
	.4byte	.LASF86
	.sleb128 58
	.uleb128 0x9
	.4byte	.LASF87
	.sleb128 59
	.uleb128 0x9
	.4byte	.LASF88
	.sleb128 60
	.uleb128 0x9
	.4byte	.LASF89
	.sleb128 61
	.uleb128 0x9
	.4byte	.LASF90
	.sleb128 62
	.uleb128 0x9
	.4byte	.LASF91
	.sleb128 63
	.uleb128 0x9
	.4byte	.LASF92
	.sleb128 64
	.uleb128 0x9
	.4byte	.LASF93
	.sleb128 65
	.uleb128 0x9
	.4byte	.LASF94
	.sleb128 66
	.uleb128 0x9
	.4byte	.LASF95
	.sleb128 67
	.uleb128 0x9
	.4byte	.LASF96
	.sleb128 68
	.uleb128 0x9
	.4byte	.LASF97
	.sleb128 69
	.uleb128 0x9
	.4byte	.LASF98
	.sleb128 69
	.uleb128 0x9
	.4byte	.LASF99
	.sleb128 70
	.uleb128 0x9
	.4byte	.LASF100
	.sleb128 70
	.uleb128 0x9
	.4byte	.LASF101
	.sleb128 71
	.uleb128 0x9
	.4byte	.LASF102
	.sleb128 72
	.uleb128 0x9
	.4byte	.LASF103
	.sleb128 72
	.uleb128 0x9
	.4byte	.LASF104
	.sleb128 73
	.uleb128 0x9
	.4byte	.LASF105
	.sleb128 73
	.uleb128 0x9
	.4byte	.LASF106
	.sleb128 74
	.uleb128 0x9
	.4byte	.LASF107
	.sleb128 75
	.uleb128 0x9
	.4byte	.LASF108
	.sleb128 76
	.uleb128 0x9
	.4byte	.LASF109
	.sleb128 77
	.uleb128 0x9
	.4byte	.LASF110
	.sleb128 78
	.uleb128 0x9
	.4byte	.LASF111
	.sleb128 79
	.uleb128 0x9
	.4byte	.LASF112
	.sleb128 80
	.uleb128 0x9
	.4byte	.LASF113
	.sleb128 81
	.uleb128 0x9
	.4byte	.LASF114
	.sleb128 82
	.uleb128 0x9
	.4byte	.LASF115
	.sleb128 83
	.uleb128 0x9
	.4byte	.LASF116
	.sleb128 84
	.uleb128 0x9
	.4byte	.LASF117
	.sleb128 85
	.uleb128 0x9
	.4byte	.LASF118
	.sleb128 86
	.uleb128 0x9
	.4byte	.LASF119
	.sleb128 87
	.uleb128 0x9
	.4byte	.LASF120
	.sleb128 88
	.uleb128 0x9
	.4byte	.LASF121
	.sleb128 89
	.uleb128 0x9
	.4byte	.LASF122
	.sleb128 90
	.uleb128 0x9
	.4byte	.LASF123
	.sleb128 91
	.uleb128 0x9
	.4byte	.LASF124
	.sleb128 91
	.uleb128 0x9
	.4byte	.LASF125
	.sleb128 92
	.uleb128 0x9
	.4byte	.LASF126
	.sleb128 92
	.uleb128 0x9
	.4byte	.LASF127
	.sleb128 93
	.uleb128 0x9
	.4byte	.LASF128
	.sleb128 94
	.uleb128 0x9
	.4byte	.LASF129
	.sleb128 95
	.uleb128 0x9
	.4byte	.LASF130
	.sleb128 96
	.uleb128 0x9
	.4byte	.LASF131
	.sleb128 97
	.uleb128 0x9
	.4byte	.LASF132
	.sleb128 97
	.uleb128 0x9
	.4byte	.LASF133
	.sleb128 98
	.uleb128 0x9
	.4byte	.LASF134
	.sleb128 98
	.uleb128 0x9
	.4byte	.LASF135
	.sleb128 98
	.uleb128 0x9
	.4byte	.LASF136
	.sleb128 99
	.uleb128 0x9
	.4byte	.LASF137
	.sleb128 99
	.uleb128 0x9
	.4byte	.LASF138
	.sleb128 100
	.uleb128 0x9
	.4byte	.LASF139
	.sleb128 101
	.uleb128 0x9
	.4byte	.LASF140
	.sleb128 102
	.uleb128 0x9
	.4byte	.LASF141
	.sleb128 103
	.uleb128 0x9
	.4byte	.LASF142
	.sleb128 104
	.uleb128 0x9
	.4byte	.LASF143
	.sleb128 104
	.uleb128 0x9
	.4byte	.LASF144
	.sleb128 105
	.uleb128 0x9
	.4byte	.LASF145
	.sleb128 105
	.uleb128 0x9
	.4byte	.LASF146
	.sleb128 106
	.uleb128 0x9
	.4byte	.LASF147
	.sleb128 107
	.uleb128 0x9
	.4byte	.LASF148
	.sleb128 108
	.uleb128 0x9
	.4byte	.LASF149
	.sleb128 109
	.uleb128 0x9
	.4byte	.LASF150
	.sleb128 110
	.uleb128 0x9
	.4byte	.LASF151
	.sleb128 111
	.uleb128 0x9
	.4byte	.LASF152
	.sleb128 112
	.uleb128 0x9
	.4byte	.LASF153
	.sleb128 113
	.uleb128 0x9
	.4byte	.LASF154
	.sleb128 114
	.uleb128 0x9
	.4byte	.LASF155
	.sleb128 114
	.uleb128 0x9
	.4byte	.LASF156
	.sleb128 115
	.uleb128 0x9
	.4byte	.LASF157
	.sleb128 116
	.uleb128 0x9
	.4byte	.LASF158
	.sleb128 117
	.uleb128 0x9
	.4byte	.LASF159
	.sleb128 118
	.uleb128 0x9
	.4byte	.LASF160
	.sleb128 119
	.byte	0
	.uleb128 0xb
	.4byte	.LASF161
	.byte	0x4
	.2byte	0x336
	.4byte	0xea
	.uleb128 0x7
	.4byte	.LASF162
	.byte	0x5
	.byte	0xe
	.4byte	0x45
	.uleb128 0x7
	.4byte	.LASF163
	.byte	0x5
	.byte	0xf
	.4byte	0x53
	.uleb128 0x8
	.byte	0x4
	.byte	0x6
	.byte	0x9c
	.4byte	0x4c8
	.uleb128 0x9
	.4byte	.LASF164
	.sleb128 0
	.uleb128 0x9
	.4byte	.LASF165
	.sleb128 1
	.byte	0
	.uleb128 0x7
	.4byte	.LASF166
	.byte	0x6
	.byte	0x9f
	.4byte	0x4b3
	.uleb128 0x8
	.byte	0x4
	.byte	0x6
	.byte	0xa2
	.4byte	0x4ee
	.uleb128 0x9
	.4byte	.LASF167
	.sleb128 16
	.uleb128 0x9
	.4byte	.LASF168
	.sleb128 24
	.uleb128 0x9
	.4byte	.LASF169
	.sleb128 32
	.byte	0
	.uleb128 0x7
	.4byte	.LASF170
	.byte	0x6
	.byte	0xa6
	.4byte	0x4d3
	.uleb128 0x8
	.byte	0x4
	.byte	0x6
	.byte	0xa9
	.4byte	0x514
	.uleb128 0x9
	.4byte	.LASF171
	.sleb128 0
	.uleb128 0x9
	.4byte	.LASF172
	.sleb128 1
	.uleb128 0x9
	.4byte	.LASF173
	.sleb128 2
	.byte	0
	.uleb128 0x7
	.4byte	.LASF174
	.byte	0x6
	.byte	0xad
	.4byte	0x4f9
	.uleb128 0xc
	.byte	0x24
	.byte	0x6
	.byte	0xb3
	.4byte	0x544
	.uleb128 0xd
	.4byte	.LASF175
	.byte	0x6
	.byte	0xb4
	.4byte	0x94
	.byte	0x2
	.byte	0x23
	.uleb128 0
	.uleb128 0xd
	.4byte	.LASF176
	.byte	0x6
	.byte	0xb5
	.4byte	0x544
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0
	.uleb128 0xe
	.4byte	0x8a
	.4byte	0x554
	.uleb128 0xf
	.4byte	0x2c
	.byte	0x1f
	.byte	0
	.uleb128 0x7
	.4byte	.LASF177
	.byte	0x6
	.byte	0xb6
	.4byte	0x51f
	.uleb128 0x10
	.4byte	.LASF208
	.byte	0x1
	.byte	0x5b
	.byte	0x1
	.4byte	0x5d0
	.byte	0x1
	.4byte	0x5d0
	.uleb128 0x11
	.ascii	"buf\000"
	.byte	0x1
	.byte	0x5b
	.4byte	0x5d0
	.uleb128 0x12
	.4byte	.LASF175
	.byte	0x1
	.byte	0x5b
	.4byte	0x49d
	.uleb128 0x11
	.ascii	"bAC\000"
	.byte	0x1
	.byte	0x5b
	.4byte	0xac
	.uleb128 0x12
	.4byte	.LASF178
	.byte	0x1
	.byte	0x5b
	.4byte	0xd8
	.uleb128 0x12
	.4byte	.LASF179
	.byte	0x1
	.byte	0x5b
	.4byte	0xac
	.uleb128 0x12
	.4byte	.LASF180
	.byte	0x1
	.byte	0x5b
	.4byte	0x4c8
	.uleb128 0x11
	.ascii	"bEn\000"
	.byte	0x1
	.byte	0x5b
	.4byte	0xac
	.uleb128 0x13
	.ascii	"err\000"
	.byte	0x1
	.byte	0x5d
	.4byte	0x49d
	.uleb128 0x14
	.4byte	.LASF209
	.byte	0x1
	.byte	0xc3
	.byte	0
	.uleb128 0x3
	.byte	0x4
	.4byte	0x4a8
	.uleb128 0x15
	.4byte	0x55f
	.4byte	.LFB187
	.4byte	.LFE187
	.4byte	.LLST0
	.byte	0x1
	.4byte	0x751
	.uleb128 0x16
	.4byte	0x570
	.4byte	.LLST1
	.uleb128 0x16
	.4byte	0x57b
	.4byte	.LLST2
	.uleb128 0x16
	.4byte	0x591
	.4byte	.LLST3
	.uleb128 0x16
	.4byte	0x59c
	.4byte	.LLST4
	.uleb128 0x17
	.4byte	0x5a7
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x17
	.4byte	0x5b2
	.byte	0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x17
	.4byte	0x586
	.byte	0x6
	.byte	0xfa
	.4byte	0x586
	.byte	0x9f
	.uleb128 0x18
	.4byte	0x5bd
	.4byte	.LLST5
	.uleb128 0x19
	.4byte	0x5c8
	.4byte	.L4
	.uleb128 0x1a
	.4byte	.LVL3
	.4byte	0x9dd
	.4byte	0x652
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x8
	.byte	0x3e
	.byte	0
	.uleb128 0x1c
	.4byte	.LVL5
	.4byte	0x9f6
	.uleb128 0x1a
	.4byte	.LVL6
	.4byte	0xa1a
	.4byte	0x67b
	.uleb128 0x1b
	.byte	0x1
	.byte	0x52
	.byte	0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x1b
	.byte	0x1
	.byte	0x51
	.byte	0x2
	.byte	0x77
	.sleb128 0
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x74
	.sleb128 0
	.byte	0
	.uleb128 0x1c
	.4byte	.LVL7
	.4byte	0xa43
	.uleb128 0x1a
	.4byte	.LVL8
	.4byte	0x9dd
	.4byte	0x698
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x8
	.byte	0x3e
	.byte	0
	.uleb128 0x1a
	.4byte	.LVL9
	.4byte	0xa4d
	.4byte	0x6b5
	.uleb128 0x1b
	.byte	0x1
	.byte	0x51
	.byte	0x5
	.byte	0x3
	.4byte	.LC0
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x8
	.byte	0x3e
	.byte	0
	.uleb128 0x1a
	.4byte	.LVL10
	.4byte	0xa6b
	.4byte	0x6ce
	.uleb128 0x1b
	.byte	0x1
	.byte	0x51
	.byte	0x2
	.byte	0x8
	.byte	0x20
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x1
	.byte	0x32
	.byte	0
	.uleb128 0x1a
	.4byte	.LVL12
	.4byte	0xa88
	.4byte	0x6f8
	.uleb128 0x1b
	.byte	0x1
	.byte	0x53
	.byte	0x5
	.byte	0x77
	.sleb128 0
	.byte	0x9
	.byte	0x80
	.byte	0x1a
	.uleb128 0x1b
	.byte	0x1
	.byte	0x52
	.byte	0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x1b
	.byte	0x1
	.byte	0x51
	.byte	0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x3
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0
	.uleb128 0x1c
	.4byte	.LVL14
	.4byte	0xaaf
	.uleb128 0x1a
	.4byte	.LVL17
	.4byte	0xab9
	.4byte	0x721
	.uleb128 0x1b
	.byte	0x1
	.byte	0x51
	.byte	0x5
	.byte	0x3
	.4byte	.LC2
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x5
	.byte	0x3
	.4byte	.LC1
	.byte	0
	.uleb128 0x1a
	.4byte	.LVL19
	.4byte	0xad2
	.4byte	0x73e
	.uleb128 0x1b
	.byte	0x1
	.byte	0x51
	.byte	0x5
	.byte	0x3
	.4byte	.LC0
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x8
	.byte	0x3e
	.byte	0
	.uleb128 0x1c
	.4byte	.LVL20
	.4byte	0xaf0
	.uleb128 0x1c
	.4byte	.LVL22
	.4byte	0xaaf
	.byte	0
	.uleb128 0x1d
	.byte	0x1
	.4byte	.LASF181
	.byte	0x1
	.byte	0xd3
	.byte	0x1
	.4byte	0x84
	.4byte	.LFB183
	.4byte	.LFE183
	.4byte	.LLST6
	.byte	0x1
	.4byte	0x7ee
	.uleb128 0x1e
	.ascii	"buf\000"
	.byte	0x1
	.byte	0xd3
	.4byte	0x84
	.4byte	.LLST7
	.uleb128 0x1f
	.4byte	.LASF175
	.byte	0x1
	.byte	0xd3
	.4byte	0x45
	.4byte	.LLST8
	.uleb128 0x1e
	.ascii	"bAC\000"
	.byte	0x1
	.byte	0xd3
	.4byte	0x53
	.4byte	.LLST9
	.uleb128 0x1f
	.4byte	.LASF178
	.byte	0x1
	.byte	0xd3
	.4byte	0xd8
	.4byte	.LLST10
	.uleb128 0x20
	.4byte	.LASF179
	.byte	0x1
	.byte	0xd3
	.4byte	0x53
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x21
	.4byte	.LVL26
	.4byte	0x5d6
	.uleb128 0x1b
	.byte	0x1
	.byte	0x53
	.byte	0x4
	.byte	0x91
	.sleb128 0
	.byte	0x94
	.byte	0x1
	.uleb128 0x1b
	.byte	0x1
	.byte	0x52
	.byte	0x3
	.byte	0xf3
	.uleb128 0x1
	.byte	0x53
	.uleb128 0x1b
	.byte	0x1
	.byte	0x51
	.byte	0x3
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x3
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x1b
	.byte	0x2
	.byte	0x7d
	.sleb128 4
	.byte	0x1
	.byte	0x31
	.uleb128 0x22
	.4byte	0x586
	.byte	0x1
	.byte	0x31
	.byte	0
	.byte	0
	.uleb128 0x1d
	.byte	0x1
	.4byte	.LASF182
	.byte	0x1
	.byte	0xdc
	.byte	0x1
	.4byte	0x84
	.4byte	.LFB184
	.4byte	.LFE184
	.4byte	.LLST11
	.byte	0x1
	.4byte	0x88b
	.uleb128 0x1e
	.ascii	"buf\000"
	.byte	0x1
	.byte	0xdc
	.4byte	0x84
	.4byte	.LLST12
	.uleb128 0x1f
	.4byte	.LASF175
	.byte	0x1
	.byte	0xdc
	.4byte	0x45
	.4byte	.LLST13
	.uleb128 0x1e
	.ascii	"bAC\000"
	.byte	0x1
	.byte	0xdc
	.4byte	0x53
	.4byte	.LLST14
	.uleb128 0x1f
	.4byte	.LASF178
	.byte	0x1
	.byte	0xdc
	.4byte	0xd8
	.4byte	.LLST15
	.uleb128 0x20
	.4byte	.LASF179
	.byte	0x1
	.byte	0xdc
	.4byte	0x53
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x21
	.4byte	.LVL30
	.4byte	0x5d6
	.uleb128 0x1b
	.byte	0x1
	.byte	0x53
	.byte	0x4
	.byte	0x91
	.sleb128 0
	.byte	0x94
	.byte	0x1
	.uleb128 0x1b
	.byte	0x1
	.byte	0x52
	.byte	0x3
	.byte	0xf3
	.uleb128 0x1
	.byte	0x53
	.uleb128 0x1b
	.byte	0x1
	.byte	0x51
	.byte	0x3
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x3
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x1b
	.byte	0x2
	.byte	0x7d
	.sleb128 4
	.byte	0x1
	.byte	0x30
	.uleb128 0x22
	.4byte	0x586
	.byte	0x1
	.byte	0x31
	.byte	0
	.byte	0
	.uleb128 0x23
	.byte	0x1
	.4byte	.LASF210
	.byte	0x1
	.byte	0xe4
	.byte	0x1
	.4byte	0x45
	.4byte	.LFB185
	.4byte	.LFE185
	.4byte	.LLST16
	.byte	0x1
	.uleb128 0x1d
	.byte	0x1
	.4byte	.LASF183
	.byte	0x1
	.byte	0xec
	.byte	0x1
	.4byte	0x45
	.4byte	.LFB186
	.4byte	.LFE186
	.4byte	.LLST17
	.byte	0x1
	.4byte	0x924
	.uleb128 0x1f
	.4byte	.LASF184
	.byte	0x1
	.byte	0xec
	.4byte	0x84
	.4byte	.LLST18
	.uleb128 0x1f
	.4byte	.LASF175
	.byte	0x1
	.byte	0xec
	.4byte	0x45
	.4byte	.LLST19
	.uleb128 0x24
	.4byte	.LASF185
	.byte	0x1
	.byte	0xee
	.4byte	0x554
	.byte	0x2
	.byte	0x91
	.sleb128 -52
	.uleb128 0x25
	.ascii	"i\000"
	.byte	0x1
	.byte	0xef
	.4byte	0x49d
	.4byte	.LLST20
	.uleb128 0x1a
	.4byte	.LVL40
	.4byte	0xab9
	.4byte	0x913
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x5
	.byte	0x3
	.4byte	.LC3
	.byte	0
	.uleb128 0x21
	.4byte	.LVL41
	.4byte	0xafe
	.uleb128 0x1b
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x91
	.sleb128 -52
	.byte	0
	.byte	0
	.uleb128 0xe
	.4byte	0x49d
	.4byte	0x934
	.uleb128 0xf
	.4byte	0x2c
	.byte	0x7
	.byte	0
	.uleb128 0x24
	.4byte	.LASF186
	.byte	0x1
	.byte	0x48
	.4byte	0x945
	.byte	0x5
	.byte	0x3
	.4byte	g_HACC_CFG_1
	.uleb128 0x4
	.4byte	0x924
	.uleb128 0x24
	.4byte	.LASF187
	.byte	0x1
	.byte	0x4d
	.4byte	0x95b
	.byte	0x5
	.byte	0x3
	.4byte	g_HACC_CFG_2
	.uleb128 0x4
	.4byte	0x924
	.uleb128 0x24
	.4byte	.LASF188
	.byte	0x1
	.byte	0x52
	.4byte	0x971
	.byte	0x5
	.byte	0x3
	.4byte	g_HACC_CFG_3
	.uleb128 0x4
	.4byte	0x924
	.uleb128 0x26
	.4byte	.LASF189
	.byte	0x7
	.byte	0x24
	.4byte	0x45
	.byte	0x1
	.byte	0x1
	.uleb128 0xe
	.4byte	0x6f
	.4byte	0x98e
	.uleb128 0x27
	.byte	0
	.uleb128 0x26
	.4byte	.LASF190
	.byte	0x8
	.byte	0x1b
	.4byte	0x983
	.byte	0x1
	.byte	0x1
	.uleb128 0xe
	.4byte	0x3e
	.4byte	0x9a6
	.uleb128 0x27
	.byte	0
	.uleb128 0x28
	.4byte	.LASF191
	.byte	0x9
	.2byte	0x18a
	.4byte	0x9b4
	.byte	0x1
	.byte	0x1
	.uleb128 0x4
	.4byte	0x99b
	.uleb128 0x29
	.4byte	.LASF192
	.byte	0x1
	.byte	0x42
	.4byte	0xac
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.4byte	bHACC_HWWrapKeyInit
	.uleb128 0x29
	.4byte	.LASF193
	.byte	0x1
	.byte	0x43
	.4byte	0xac
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.4byte	bHACC_SWKeyInit
	.uleb128 0x2a
	.byte	0x1
	.4byte	.LASF196
	.byte	0x4
	.2byte	0x39b
	.byte	0x1
	.4byte	0x6f
	.byte	0x1
	.4byte	0x9f6
	.uleb128 0x2b
	.4byte	0x491
	.byte	0
	.uleb128 0x2c
	.byte	0x1
	.4byte	.LASF194
	.byte	0x1
	.byte	0x35
	.byte	0x1
	.byte	0x1
	.4byte	0xa0f
	.uleb128 0x2b
	.4byte	0xe3
	.uleb128 0x2b
	.4byte	0xa0f
	.byte	0
	.uleb128 0x3
	.byte	0x4
	.4byte	0xa15
	.uleb128 0x4
	.4byte	0x45
	.uleb128 0x2c
	.byte	0x1
	.4byte	.LASF195
	.byte	0x1
	.byte	0x36
	.byte	0x1
	.byte	0x1
	.4byte	0xa38
	.uleb128 0x2b
	.4byte	0xa38
	.uleb128 0x2b
	.4byte	0x94
	.uleb128 0x2b
	.4byte	0xa38
	.byte	0
	.uleb128 0x3
	.byte	0x4
	.4byte	0xa3e
	.uleb128 0x2d
	.4byte	0x94
	.uleb128 0x2e
	.byte	0x1
	.4byte	.LASF200
	.byte	0x1
	.byte	0x37
	.byte	0x1
	.byte	0x1
	.uleb128 0x2a
	.byte	0x1
	.4byte	.LASF197
	.byte	0x4
	.2byte	0x398
	.byte	0x1
	.4byte	0x6f
	.byte	0x1
	.4byte	0xa6b
	.uleb128 0x2b
	.4byte	0xea
	.uleb128 0x2b
	.4byte	0xa6
	.byte	0
	.uleb128 0x2f
	.byte	0x1
	.4byte	.LASF198
	.byte	0x1
	.byte	0x31
	.byte	0x1
	.4byte	0x45
	.byte	0x1
	.4byte	0xa88
	.uleb128 0x2b
	.4byte	0x514
	.uleb128 0x2b
	.4byte	0x4ee
	.byte	0
	.uleb128 0x2f
	.byte	0x1
	.4byte	.LASF199
	.byte	0x1
	.byte	0x32
	.byte	0x1
	.4byte	0x45
	.byte	0x1
	.4byte	0xaaf
	.uleb128 0x2b
	.4byte	0x4c8
	.uleb128 0x2b
	.4byte	0x84
	.uleb128 0x2b
	.4byte	0x84
	.uleb128 0x2b
	.4byte	0x45
	.byte	0
	.uleb128 0x2e
	.byte	0x1
	.4byte	.LASF201
	.byte	0xa
	.byte	0x17
	.byte	0x1
	.byte	0x1
	.uleb128 0x2f
	.byte	0x1
	.4byte	.LASF202
	.byte	0x8
	.byte	0x6a
	.byte	0x1
	.4byte	0x6f
	.byte	0x1
	.4byte	0xad2
	.uleb128 0x2b
	.4byte	0x33
	.uleb128 0x30
	.byte	0
	.uleb128 0x2a
	.byte	0x1
	.4byte	.LASF203
	.byte	0x4
	.2byte	0x397
	.byte	0x1
	.4byte	0x6f
	.byte	0x1
	.4byte	0xaf0
	.uleb128 0x2b
	.4byte	0xea
	.uleb128 0x2b
	.4byte	0xa6
	.byte	0
	.uleb128 0x31
	.byte	0x1
	.4byte	.LASF211
	.byte	0xa
	.byte	0x16
	.byte	0x1
	.4byte	0x6f
	.byte	0x1
	.uleb128 0x2f
	.byte	0x1
	.4byte	.LASF204
	.byte	0x1
	.byte	0x33
	.byte	0x1
	.4byte	0x45
	.byte	0x1
	.4byte	0xb16
	.uleb128 0x2b
	.4byte	0xb16
	.byte	0
	.uleb128 0x3
	.byte	0x4
	.4byte	0x554
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0xa
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x34
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0xa
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0xa
	.uleb128 0x2111
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1e
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x1f
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x20
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x21
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x22
	.uleb128 0x410a
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2111
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x23
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x2117
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x24
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x25
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x26
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x27
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x28
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x29
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x2a
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2b
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2c
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2d
	.uleb128 0x35
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2e
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x2f
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x30
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x31
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LLST0:
	.4byte	.LFB187-.Ltext0
	.4byte	.LCFI0-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI0-.Ltext0
	.4byte	.LCFI2-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI2-.Ltext0
	.4byte	.LFE187-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL2-.Ltext0
	.4byte	.LFE187-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL3-1-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL3-1-.Ltext0
	.4byte	.LFE187-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL3-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL3-1-.Ltext0
	.4byte	.LFE187-.Ltext0
	.2byte	0x1
	.byte	0x56
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL3-1-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL3-1-.Ltext0
	.4byte	.LFE187-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x53
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL11-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL12-.Ltext0
	.4byte	.LVL13-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL15-.Ltext0
	.4byte	.LVL16-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL18-.Ltext0
	.4byte	.LVL20-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL20-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LFB183-.Ltext0
	.4byte	.LCFI3-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI3-.Ltext0
	.4byte	.LCFI5-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI5-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST7:
	.4byte	.LVL23-.Ltext0
	.4byte	.LVL26-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL26-1-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST8:
	.4byte	.LVL23-.Ltext0
	.4byte	.LVL26-1-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL26-1-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST9:
	.4byte	.LVL23-.Ltext0
	.4byte	.LVL24-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL24-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x52
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST10:
	.4byte	.LVL23-.Ltext0
	.4byte	.LVL25-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL25-.Ltext0
	.4byte	.LVL26-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL26-1-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x53
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST11:
	.4byte	.LFB184-.Ltext0
	.4byte	.LCFI6-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI6-.Ltext0
	.4byte	.LCFI8-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI8-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST12:
	.4byte	.LVL27-.Ltext0
	.4byte	.LVL30-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL30-1-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST13:
	.4byte	.LVL27-.Ltext0
	.4byte	.LVL30-1-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL30-1-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST14:
	.4byte	.LVL27-.Ltext0
	.4byte	.LVL28-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL28-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x52
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST15:
	.4byte	.LVL27-.Ltext0
	.4byte	.LVL29-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL29-.Ltext0
	.4byte	.LVL30-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL30-1-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x53
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST16:
	.4byte	.LFB185-.Ltext0
	.4byte	.LCFI9-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI9-.Ltext0
	.4byte	.LCFI11-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI11-.Ltext0
	.4byte	.LFE185-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST17:
	.4byte	.LFB186-.Ltext0
	.4byte	.LCFI12-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI12-.Ltext0
	.4byte	.LCFI14-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI14-.Ltext0
	.4byte	.LFE186-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST18:
	.4byte	.LVL31-.Ltext0
	.4byte	.LVL33-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL33-.Ltext0
	.4byte	.LFE186-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST19:
	.4byte	.LVL31-.Ltext0
	.4byte	.LVL34-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL34-.Ltext0
	.4byte	.LVL35-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.byte	0x9f
	.4byte	.LVL35-.Ltext0
	.4byte	.LVL36-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL36-.Ltext0
	.4byte	.LFE186-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST20:
	.4byte	.LVL32-.Ltext0
	.4byte	.LVL34-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL35-.Ltext0
	.4byte	.LVL37-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL38-.Ltext0
	.4byte	.LVL39-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	0
	.4byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF130:
	.ascii	"MT_CG_DISP_DBI_ENGINE_SW_CG\000"
.LASF202:
	.ascii	"printk\000"
.LASF112:
	.ascii	"MT_CG_MDP_RSZ_SW_CG\000"
.LASF150:
	.ascii	"MT_VCG_AUX_ADC\000"
.LASF184:
	.ascii	"sec_seed\000"
.LASF93:
	.ascii	"MT_CG_APXGPT_SW_CG\000"
.LASF34:
	.ascii	"MT_CG_MPLL_D20\000"
.LASF40:
	.ascii	"MT_CG_UPLL_D2\000"
.LASF42:
	.ascii	"MT_CG_UPLL_D3\000"
.LASF45:
	.ascii	"MT_CG_UPLL_D4\000"
.LASF43:
	.ascii	"MT_CG_UPLL_D5\000"
.LASF46:
	.ascii	"MT_CG_UPLL_D6\000"
.LASF44:
	.ascii	"MT_CG_UPLL_D7\000"
.LASF48:
	.ascii	"MT_CG_UPLL_D8\000"
.LASF211:
	.ascii	"osal_hacc_lock\000"
.LASF118:
	.ascii	"MT_CG_MM_CAM_SMI_SW_CG\000"
.LASF139:
	.ascii	"MT_CG_AUD_PDN_ADC_EN\000"
.LASF190:
	.ascii	"console_printk\000"
.LASF105:
	.ascii	"CG_MMSYS0_FROM\000"
.LASF192:
	.ascii	"bHACC_HWWrapKeyInit\000"
.LASF10:
	.ascii	"long long unsigned int\000"
.LASF103:
	.ascii	"CG_CTRL7_TO\000"
.LASF47:
	.ascii	"MT_CG_UPLL_D10\000"
.LASF49:
	.ascii	"MT_CG_UPLL_D12\000"
.LASF136:
	.ascii	"MT_CG_AUD_PDN_AFE_EN\000"
.LASF51:
	.ascii	"MT_CG_UPLL_D16\000"
.LASF197:
	.ascii	"disable_clock\000"
.LASF208:
	.ascii	"sp_hacc_internal\000"
.LASF97:
	.ascii	"MT_CG_AUX_SW_CG_ADC\000"
.LASF77:
	.ascii	"MT_CG_NFI_SW_CG\000"
.LASF9:
	.ascii	"long long int\000"
.LASF8:
	.ascii	"signed char\000"
.LASF143:
	.ascii	"CG_AUDIO_TO\000"
.LASF164:
	.ascii	"AES_DEC\000"
.LASF92:
	.ascii	"MT_CG_SPI_SW_CG\000"
.LASF165:
	.ascii	"AES_ENC\000"
.LASF132:
	.ascii	"CG_MMSYS1_TO\000"
.LASF99:
	.ascii	"MT_CG_XIU2AHB0_SW_CG\000"
.LASF125:
	.ascii	"MT_CG_DSI_ENGINE_SW_CG\000"
.LASF157:
	.ascii	"MT_CG_SRC_DBI\000"
.LASF194:
	.ascii	"HACC_V3_Init\000"
.LASF17:
	.ascii	"HACC_USER\000"
.LASF6:
	.ascii	"long int\000"
.LASF104:
	.ascii	"MT_CG_DISP_PQ_SW_CG\000"
.LASF68:
	.ascii	"MT_CG_GPU_500P5M_EN\000"
.LASF73:
	.ascii	"MT_CG_THEM_SW_CG\000"
.LASF75:
	.ascii	"MT_CG_I2C0_SW_CG\000"
.LASF204:
	.ascii	"hacc_init\000"
.LASF80:
	.ascii	"MT_CG_UART0_SW_CG\000"
.LASF30:
	.ascii	"MT_CG_MPLL_D10\000"
.LASF33:
	.ascii	"MT_CG_MPLL_D12\000"
.LASF126:
	.ascii	"CG_MMSYS1_FROM\000"
.LASF31:
	.ascii	"MT_CG_MPLL_D14\000"
.LASF127:
	.ascii	"MT_CG_DSI_DIGITAL_SW_CG\000"
.LASF181:
	.ascii	"masp_hal_sp_hacc_enc\000"
.LASF50:
	.ascii	"MT_CG_UPLL_D20\000"
.LASF57:
	.ascii	"MT_CG_MFG_MM_SW_CG\000"
.LASF52:
	.ascii	"MT_CG_UPLL_D24\000"
.LASF168:
	.ascii	"AES_KEY_192\000"
.LASF162:
	.ascii	"uint32\000"
.LASF12:
	.ascii	"BOOL\000"
.LASF148:
	.ascii	"MT_VCG_APDMA_I2C\000"
.LASF124:
	.ascii	"CG_MMSYS0_TO\000"
.LASF193:
	.ascii	"bHACC_SWKeyInit\000"
.LASF21:
	.ascii	"MT_CG_USB_48M\000"
.LASF39:
	.ascii	"CG_MPLL_TO\000"
.LASF153:
	.ascii	"MT_VCG_MUTEX_SLOW_CLOCK_DISP\000"
.LASF109:
	.ascii	"MT_CG_DISP_OVL_SW_CG\000"
.LASF185:
	.ascii	"keyseed\000"
.LASF159:
	.ascii	"MT_CG_CLOCK_OFF\000"
.LASF186:
	.ascii	"g_HACC_CFG_1\000"
.LASF187:
	.ascii	"g_HACC_CFG_2\000"
.LASF3:
	.ascii	"unsigned int\000"
.LASF188:
	.ascii	"g_HACC_CFG_3\000"
.LASF154:
	.ascii	"MT_VCG_MUTEX_SLOW_CLOCK_M4U\000"
.LASF64:
	.ascii	"MT_CG_DBI_PAD1_SW_CG\000"
.LASF176:
	.ascii	"seed\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF78:
	.ascii	"MT_CG_DEBUGSYS_SW_CG\000"
.LASF147:
	.ascii	"MT_VCG_APDMA_BTIF\000"
.LASF37:
	.ascii	"MT_CG_MPLL_D24\000"
.LASF174:
	.ascii	"AES_KEY_ID\000"
.LASF36:
	.ascii	"MT_CG_MPLL_D16\000"
.LASF19:
	.ascii	"MT_CG_SYS_26M\000"
.LASF35:
	.ascii	"MT_CG_MPLL_D28\000"
.LASF56:
	.ascii	"MT_CG_CAM_MM_SW_CG\000"
.LASF198:
	.ascii	"hacc_set_key\000"
.LASF175:
	.ascii	"size\000"
.LASF4:
	.ascii	"short unsigned int\000"
.LASF59:
	.ascii	"MT_CG_MIPI_26M_DBG_EN\000"
.LASF53:
	.ascii	"CG_UPLL_TO\000"
.LASF131:
	.ascii	"MT_CG_DISP_DBI_IF_SW_CG\000"
.LASF142:
	.ascii	"MT_CG_AUD_PDN_TML_EN\000"
.LASF119:
	.ascii	"MT_CG_MM_SENINF_SW_CG\000"
.LASF94:
	.ascii	"MT_CG_AUDIO_SW_CG\000"
.LASF82:
	.ascii	"MT_CG_BTIF_SW_CG\000"
.LASF71:
	.ascii	"MT_CG_EFUSE_SW_CG\000"
.LASF88:
	.ascii	"MT_CG_NFI2X_SW_CG\000"
.LASF98:
	.ascii	"CG_CTRL1_TO\000"
.LASF205:
	.ascii	"GNU C 4.7\000"
.LASF167:
	.ascii	"AES_KEY_128\000"
.LASF116:
	.ascii	"MT_CG_MM_CAM_DP_SW_CG\000"
.LASF146:
	.ascii	"MT_VCG_APDMA_WLAN\000"
.LASF72:
	.ascii	"CG_CTRL1_FROM\000"
.LASF87:
	.ascii	"MT_CG_MSDC1_SW_CG\000"
.LASF210:
	.ascii	"masp_hal_sp_hacc_blk_sz\000"
.LASF196:
	.ascii	"clock_is_on\000"
.LASF133:
	.ascii	"MT_CG_MFG_PDN_BG3D_SW_CG\000"
.LASF24:
	.ascii	"CG_MPLL_FROM\000"
.LASF172:
	.ascii	"AES_HW_KEY\000"
.LASF84:
	.ascii	"MT_CG_FHCTL_SW_CG\000"
.LASF141:
	.ascii	"MT_CG_AUD_PDN_DAC_PREDIS_EN\000"
.LASF134:
	.ascii	"CG_MFG_FROM\000"
.LASF1:
	.ascii	"sizetype\000"
.LASF54:
	.ascii	"MT_CG_PWM_MM_SW_CG\000"
.LASF199:
	.ascii	"hacc_do_aes\000"
.LASF115:
	.ascii	"MT_CG_DISP_PWM_26M_SW_CG\000"
.LASF161:
	.ascii	"cg_clk_id\000"
.LASF70:
	.ascii	"CG_CTRL0_TO\000"
.LASF169:
	.ascii	"AES_KEY_256\000"
.LASF111:
	.ascii	"MT_CG_MDP_WROT_SW_CG\000"
.LASF163:
	.ascii	"uchar\000"
.LASF120:
	.ascii	"MT_CG_MM_CAMTG_SW_CG\000"
.LASF22:
	.ascii	"CG_MIXED_TO\000"
.LASF11:
	.ascii	"float\000"
.LASF173:
	.ascii	"AES_HW_WRAP_KEY\000"
.LASF171:
	.ascii	"AES_SW_KEY\000"
.LASF20:
	.ascii	"CG_MIXED_FROM\000"
.LASF178:
	.ascii	"user\000"
.LASF38:
	.ascii	"MT_CG_MPLL_D40\000"
.LASF23:
	.ascii	"MT_CG_MPLL_D2\000"
.LASF25:
	.ascii	"MT_CG_MPLL_D3\000"
.LASF28:
	.ascii	"MT_CG_MPLL_D4\000"
.LASF26:
	.ascii	"MT_CG_MPLL_D5\000"
.LASF29:
	.ascii	"MT_CG_MPLL_D6\000"
.LASF27:
	.ascii	"MT_CG_MPLL_D7\000"
.LASF32:
	.ascii	"MT_CG_MPLL_D8\000"
.LASF18:
	.ascii	"_Bool\000"
.LASF5:
	.ascii	"unsigned char\000"
.LASF117:
	.ascii	"MT_CG_MM_CAM_DP2_SW_CG\000"
.LASF166:
	.ascii	"AES_OPS\000"
.LASF183:
	.ascii	"masp_hal_sp_hacc_init\000"
.LASF7:
	.ascii	"short int\000"
.LASF114:
	.ascii	"MT_CG_DISP_PWM_SW_CG\000"
.LASF66:
	.ascii	"MT_CG_DBI_PAD3_SW_CG\000"
.LASF41:
	.ascii	"CG_UPLL_FROM\000"
.LASF101:
	.ascii	"MT_CG_RBIST_SW_CG\000"
.LASF55:
	.ascii	"CG_CTRL0_FROM\000"
.LASF58:
	.ascii	"MT_CG_SPM_52M_SW_CG\000"
.LASF62:
	.ascii	"MT_CG_SC_MEM_CK_OFF_EN\000"
.LASF106:
	.ascii	"MT_CG_DISP_BLS_SW_CG\000"
.LASF135:
	.ascii	"CG_MFG_TO\000"
.LASF86:
	.ascii	"MT_CG_MSDC0_SW_CG\000"
.LASF90:
	.ascii	"MT_CG_SEJ_SW_CG\000"
.LASF155:
	.ascii	"CG_VIRTUAL_TO\000"
.LASF137:
	.ascii	"CG_AUDIO_FROM\000"
.LASF206:
	.ascii	"ALPS_SW/M"
	.ascii	"P/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/"
	.ascii	"kernel/drivers/masp/mach/hacc_export.c\000"
.LASF96:
	.ascii	"MT_CG_PMIC_26M_SW_CG\000"
.LASF95:
	.ascii	"MT_CG_SPM_SW_CG\000"
.LASF201:
	.ascii	"osal_hacc_unlock\000"
.LASF110:
	.ascii	"MT_CG_MDP_SHP_SW_CG\000"
.LASF91:
	.ascii	"MT_CG_MEMSLP_DLYER_SW_CG\000"
.LASF85:
	.ascii	"MT_CG_SPINFI_SW_CG\000"
.LASF180:
	.ascii	"aes_type\000"
.LASF128:
	.ascii	"MT_CG_DISP_DPI_ENGINE_SW_CG\000"
.LASF60:
	.ascii	"MT_CG_DBI_BCLK_SW_CG\000"
.LASF2:
	.ascii	"char\000"
.LASF76:
	.ascii	"MT_CG_I2C1_SW_CG\000"
.LASF177:
	.ascii	"AES_KEY_SEED\000"
.LASF152:
	.ascii	"MT_VCG_AUX_THERM\000"
.LASF195:
	.ascii	"HACC_V3_Run\000"
.LASF145:
	.ascii	"CG_VIRTUAL_FROM\000"
.LASF63:
	.ascii	"MT_CG_DBI_PAD0_SW_CG\000"
.LASF13:
	.ascii	"HACC_USER1\000"
.LASF14:
	.ascii	"HACC_USER2\000"
.LASF15:
	.ascii	"HACC_USER3\000"
.LASF16:
	.ascii	"HACC_USER4\000"
.LASF81:
	.ascii	"MT_CG_UART1_SW_CG\000"
.LASF189:
	.ascii	"elf_hwcap\000"
.LASF203:
	.ascii	"enable_clock\000"
.LASF65:
	.ascii	"MT_CG_DBI_PAD2_SW_CG\000"
.LASF140:
	.ascii	"MT_CG_AUD_PDN_DAC_EN\000"
.LASF121:
	.ascii	"MT_CG_MM_CODEC_SW_CG\000"
.LASF179:
	.ascii	"bDoLock\000"
.LASF122:
	.ascii	"MT_CG_DISP_FAKE_ENG_SW_CG\000"
.LASF144:
	.ascii	"MT_VCG_BUS\000"
.LASF138:
	.ascii	"MT_CG_AUD_PDN_I2S_EN\000"
.LASF151:
	.ascii	"MT_VCG_AUX_TP\000"
.LASF160:
	.ascii	"MT_CG_INVALID\000"
.LASF89:
	.ascii	"MT_CG_PMIC_SW_CG_AP\000"
.LASF191:
	.ascii	"hex_asc\000"
.LASF207:
	.ascii	"ALPS_SW/M"
	.ascii	"P/KK1.MP7/alps/out/target/product/mt6571v1_phone/ob"
	.ascii	"j/KERNEL_OBJ\000"
.LASF182:
	.ascii	"masp_hal_sp_hacc_dec\000"
.LASF61:
	.ascii	"MT_CG_SC_26M_CK_SEL_EN\000"
.LASF158:
	.ascii	"MT_CG_SRC_SMI\000"
.LASF149:
	.ascii	"MT_VCG_APDMA_UART\000"
.LASF123:
	.ascii	"MT_CG_MUTEX_SLOW_CLOCK_SW_CG\000"
.LASF67:
	.ascii	"MT_CG_GPU_491P52M_EN\000"
.LASF100:
	.ascii	"CG_CTRL7_FROM\000"
.LASF102:
	.ascii	"MT_CG_NFI_HCLK_SW_CG\000"
.LASF170:
	.ascii	"AES_KEY\000"
.LASF108:
	.ascii	"MT_CG_DISP_RDMA_SW_CG\000"
.LASF156:
	.ascii	"NR_CLKS\000"
.LASF83:
	.ascii	"MT_CG_USB_SW_CG\000"
.LASF69:
	.ascii	"MT_CG_ARMDCM_CLKOFF_EN\000"
.LASF129:
	.ascii	"MT_CG_DISP_DPI_IF_SW_CG\000"
.LASF79:
	.ascii	"MT_CG_PWM_SW_CG\000"
.LASF107:
	.ascii	"MT_CG_DISP_WDMA_SW_CG\000"
.LASF209:
	.ascii	"_err\000"
.LASF113:
	.ascii	"MT_CG_MDP_RDMA_SW_CG\000"
.LASF74:
	.ascii	"MT_CG_APDMA_SW_CG\000"
.LASF200:
	.ascii	"HACC_V3_Terminate\000"
	.ident	"GCC: (GNU) 4.7"
	.section	.note.GNU-stack,"",%progbits
