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
	.file	"hacc_hk.c"
	.text
.Ltext0:
	.align	2
	.global	HACC_V3_Init
	.type	HACC_V3_Init, %function
HACC_V3_Init:
.LFB182:
	.file 1 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/mach/hacc_hk.c"
	.loc 1 63 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL0:
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {r4, r5, r6, r7, fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
	.loc 1 63 0
	mov	r5, r1
	mov	r4, r0
	.loc 1 73 0
	bl	masp_hal_secure_algo_init
.LVL1:
	.loc 1 105 0
	mov	r3, #40960
	movt	r3, 61440
	mov	r2, #0
	.loc 1 111 0
	mov	r7, #2
	.loc 1 91 0
	cmp	r4, r2
	.loc 1 105 0
	str	r2, [r3, #32]
	.loc 1 126 0
	mov	r4, r2
	.loc 1 105 0
	str	r2, [r3, #36]
	.loc 1 91 0
	moveq	r0, r7
	.loc 1 105 0
	str	r2, [r3, #40]
	.loc 1 91 0
	movne	r0, #3
.LVL2:
	.loc 1 105 0
	str	r2, [r3, #44]
.LVL3:
	.loc 1 136 0
	mov	ip, r3
	.loc 1 105 0
	str	r2, [r3, #48]
.LVL4:
	.loc 1 139 0
	mov	r6, #1
	.loc 1 105 0
	str	r2, [r3, #52]
.LVL5:
	str	r2, [r3, #56]
.LVL6:
	str	r2, [r3, #60]
.LVL7:
	.loc 1 115 0
	mov	r2, #16
	.loc 1 111 0
	str	r7, [r3, #4]
	.loc 1 115 0
	str	r2, [r3, #12]
	.loc 1 117 0
	ldr	r2, [r3, #12]
	ldr	r1, .L12
	orr	r2, r2, #256
	str	r2, [r3, #12]
	.loc 1 120 0
	str	r7, [r3, #8]
.LVL8:
	.loc 1 126 0
	ldr	r2, [r5, #0]
	str	r2, [r3, #64]
.LVL9:
	ldr	r2, [r5, #4]
	str	r2, [r3, #68]
.LVL10:
	ldr	r2, [r5, #8]
	str	r2, [r3, #72]
.LVL11:
	ldr	r2, [r5, #12]
	str	r2, [r3, #76]
.LVL12:
.L4:
	.loc 1 136 0
	ldr	r2, [r1, r4, asl #4]
	.loc 1 133 0
	add	r3, r1, r4, asl #4
.LVL13:
	.loc 1 136 0
	str	r2, [ip, #16]
.LVL14:
	ldr	r2, [r3, #4]
	str	r2, [ip, #20]
.LVL15:
	ldr	r2, [r3, #8]
	str	r2, [ip, #24]
.LVL16:
	ldr	r3, [r3, #12]
.LVL17:
	str	r3, [ip, #28]
.LVL18:
	.loc 1 139 0
	str	r6, [ip, #8]
.L3:
	.loc 1 141 0 discriminator 1
	ldr	r2, [ip, #8]
	mov	r3, #40960
	movt	r3, 61440
	tst	r2, #32768
	beq	.L3
	.loc 1 130 0
	add	r4, r4, #1
.LVL19:
	cmp	r4, #3
	bne	.L4
	.loc 1 150 0
	mov	r2, #2
	str	r2, [r3, #8]
.LVL20:
	.loc 1 156 0
	ldr	r2, [r5, #0]
	str	r2, [r3, #64]
.LVL21:
	ldr	r2, [r5, #4]
	str	r2, [r3, #68]
.LVL22:
	ldr	r2, [r5, #8]
	str	r2, [r3, #72]
.LVL23:
	ldr	r2, [r5, #12]
	str	r2, [r3, #76]
.LVL24:
	.loc 1 161 0
	mov	r2, #0
	.loc 1 160 0
	str	r0, [r3, #4]
	.loc 1 161 0
	str	r2, [r3, #12]
	ldmfd	sp, {r4, r5, r6, r7, fp, sp, pc}
.L13:
	.align	2
.L12:
	.word	.LANCHOR0
.LFE182:
	.size	HACC_V3_Init, .-HACC_V3_Init
	.align	2
	.global	HACC_V3_Run
	.type	HACC_V3_Run, %function
HACC_V3_Run:
.LFB183:
	.loc 1 165 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL25:
	mov	ip, sp
.LCFI3:
	stmfd	sp!, {r4, r5, r6, r7, fp, ip, lr, pc}
.LCFI4:
	sub	fp, ip, #4
.LCFI5:
.LVL26:
	.loc 1 172 0
	cmp	r1, #0
	ldmeqfd	sp, {r4, r5, r6, r7, fp, sp, pc}
	.loc 1 177 0 discriminator 1
	mov	r4, #40960
	movt	r4, 61440
	.loc 1 172 0 discriminator 1
	mov	r5, r0
	.loc 1 180 0 discriminator 1
	mov	r6, #1
.LVL27:
.L18:
	.loc 1 177 0 discriminator 1
	ldr	r3, [r5, #0]
	str	r3, [r4, #16]
.LVL28:
	ldr	r3, [r5, #4]
	str	r3, [r4, #20]
.LVL29:
	ldr	r3, [r5, #8]
	str	r3, [r4, #24]
.LVL30:
	ldr	r3, [r5, #12]
	str	r3, [r4, #28]
.LVL31:
	.loc 1 180 0 discriminator 1
	str	r6, [r4, #8]
.L17:
	.loc 1 182 0 discriminator 1
	ldr	ip, [r4, #8]
	mov	r3, #40960
	movt	r3, 61440
	tst	ip, #32768
	beq	.L17
.LVL32:
	.loc 1 186 0 discriminator 1
	ldr	ip, [r3, #80]
	.loc 1 172 0 discriminator 1
	add	r5, r5, #16
.LVL33:
	rsb	r7, r0, r5
	cmp	r7, r1
	.loc 1 186 0 discriminator 1
	str	ip, [r2, #0]
.LVL34:
	ldr	ip, [r3, #84]
	str	ip, [r2, #4]
.LVL35:
	ldr	ip, [r3, #88]
	str	ip, [r2, #8]
.LVL36:
	ldr	r3, [r3, #92]
	str	r3, [r2, #12]
.LVL37:
	.loc 1 172 0 discriminator 1
	add	r2, r2, #16
.LVL38:
	bcc	.L18
	ldmfd	sp, {r4, r5, r6, r7, fp, sp, pc}
.LFE183:
	.size	HACC_V3_Run, .-HACC_V3_Run
	.align	2
	.global	HACC_V3_Terminate
	.type	HACC_V3_Terminate, %function
HACC_V3_Terminate:
.LFB184:
	.loc 1 192 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI6:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI7:
	sub	fp, ip, #4
.LCFI8:
	.loc 1 196 0
	mov	r3, #40960
	movt	r3, 61440
	.loc 1 204 0
	mov	r2, #0
	.loc 1 196 0
	mov	r1, #2
	str	r1, [r3, #8]
.LVL39:
	.loc 1 204 0
	str	r2, [r3, #32]
.LVL40:
	str	r2, [r3, #36]
.LVL41:
	str	r2, [r3, #40]
.LVL42:
	str	r2, [r3, #44]
.LVL43:
	str	r2, [r3, #48]
.LVL44:
	str	r2, [r3, #52]
.LVL45:
	str	r2, [r3, #56]
.LVL46:
	str	r2, [r3, #60]
.LVL47:
	.loc 1 207 0
	bl	masp_hal_secure_algo_deinit
.LVL48:
	ldmfd	sp, {fp, sp, pc}
.LFE184:
	.size	HACC_V3_Terminate, .-HACC_V3_Terminate
	.section	.rodata
	.align	2
.LANCHOR0 = . + 0
	.type	g_CFG_RANDOM_PATTERN, %object
	.size	g_CFG_RANDOM_PATTERN, 48
g_CFG_RANDOM_PATTERN:
	.word	759479152
	.word	-1488661977
	.word	-794196405
	.word	-2080390588
	.word	2127046251
	.word	1139281842
	.word	27669642
	.word	543685024
	.word	1414751237
	.word	-716790095
	.word	482602042
	.word	-566789501
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
	.4byte	.LFB182
	.4byte	.LFE182-.LFB182
	.byte	0x4
	.4byte	.LCFI0-.LFB182
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI1-.LCFI0
	.byte	0x84
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x87
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
	.byte	0x84
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x87
	.uleb128 0x5
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
	.text
.Letext0:
	.file 2 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/arch/arm/include/asm/hwcap.h"
	.file 3 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/include/linux/printk.h"
	.file 4 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/include/linux/kernel.h"
	.file 5 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/../mediatek/kernel/include/mach/mt_sec_hal.h"
	.file 6 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/asf/asf_inc/sec_typedef.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x247
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF28
	.byte	0x1
	.4byte	.LASF29
	.4byte	.LASF30
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
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF2
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF4
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF5
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF6
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF7
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF8
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF9
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF10
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.4byte	.LASF11
	.uleb128 0x4
	.4byte	.LASF31
	.byte	0x6
	.byte	0xe
	.4byte	0x3a
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.4byte	.LASF12
	.uleb128 0x5
	.byte	0x1
	.4byte	.LASF16
	.byte	0x1
	.byte	0x3e
	.byte	0x1
	.4byte	.LFB182
	.4byte	.LFE182
	.4byte	.LLST0
	.byte	0x1
	.4byte	0x10b
	.uleb128 0x6
	.4byte	.LASF13
	.byte	0x1
	.byte	0x3e
	.4byte	0x79
	.4byte	.LLST1
	.uleb128 0x6
	.4byte	.LASF14
	.byte	0x1
	.byte	0x3e
	.4byte	0x10b
	.4byte	.LLST2
	.uleb128 0x7
	.ascii	"i\000"
	.byte	0x1
	.byte	0x41
	.4byte	0x80
	.4byte	.LLST3
	.uleb128 0x7
	.ascii	"j\000"
	.byte	0x1
	.byte	0x41
	.4byte	0x80
	.4byte	.LLST4
	.uleb128 0x7
	.ascii	"p1\000"
	.byte	0x1
	.byte	0x42
	.4byte	0x10b
	.4byte	.LLST5
	.uleb128 0x8
	.4byte	.LASF15
	.byte	0x1
	.byte	0x44
	.4byte	0x80
	.4byte	.LLST6
	.uleb128 0x9
	.4byte	.LVL1
	.4byte	0x22e
	.byte	0
	.uleb128 0xa
	.byte	0x4
	.4byte	0x111
	.uleb128 0xb
	.4byte	0x80
	.uleb128 0x5
	.byte	0x1
	.4byte	.LASF17
	.byte	0x1
	.byte	0xa4
	.byte	0x1
	.4byte	.LFB183
	.4byte	.LFE183
	.4byte	.LLST7
	.byte	0x1
	.4byte	0x183
	.uleb128 0x6
	.4byte	.LASF18
	.byte	0x1
	.byte	0xa4
	.4byte	0x183
	.4byte	.LLST8
	.uleb128 0xc
	.4byte	.LASF19
	.byte	0x1
	.byte	0xa4
	.4byte	0x80
	.byte	0x1
	.byte	0x51
	.uleb128 0x6
	.4byte	.LASF20
	.byte	0x1
	.byte	0xa4
	.4byte	0x183
	.4byte	.LLST9
	.uleb128 0x7
	.ascii	"i\000"
	.byte	0x1
	.byte	0xa6
	.4byte	0x80
	.4byte	.LLST10
	.uleb128 0x7
	.ascii	"j\000"
	.byte	0x1
	.byte	0xa6
	.4byte	0x80
	.4byte	.LLST11
	.uleb128 0xd
	.ascii	"len\000"
	.byte	0x1
	.byte	0xa9
	.4byte	0x80
	.byte	0x1
	.byte	0x51
	.byte	0
	.uleb128 0xa
	.byte	0x4
	.4byte	0x189
	.uleb128 0xe
	.4byte	0x80
	.uleb128 0x5
	.byte	0x1
	.4byte	.LASF21
	.byte	0x1
	.byte	0xbf
	.byte	0x1
	.4byte	.LFB184
	.4byte	.LFE184
	.4byte	.LLST12
	.byte	0x1
	.4byte	0x1bf
	.uleb128 0x7
	.ascii	"i\000"
	.byte	0x1
	.byte	0xc1
	.4byte	0x80
	.4byte	.LLST13
	.uleb128 0x9
	.4byte	.LVL48
	.4byte	0x23c
	.byte	0
	.uleb128 0xf
	.4byte	0x80
	.4byte	0x1d5
	.uleb128 0x10
	.4byte	0x2c
	.byte	0x2
	.uleb128 0x10
	.4byte	0x2c
	.byte	0x3
	.byte	0
	.uleb128 0x11
	.4byte	.LASF22
	.byte	0x1
	.byte	0x34
	.4byte	0x1e6
	.byte	0x5
	.byte	0x3
	.4byte	g_CFG_RANDOM_PATTERN
	.uleb128 0xb
	.4byte	0x1bf
	.uleb128 0x12
	.4byte	.LASF23
	.byte	0x2
	.byte	0x24
	.4byte	0x3a
	.byte	0x1
	.byte	0x1
	.uleb128 0xf
	.4byte	0x5d
	.4byte	0x203
	.uleb128 0x13
	.byte	0
	.uleb128 0x12
	.4byte	.LASF24
	.byte	0x3
	.byte	0x1b
	.4byte	0x1f8
	.byte	0x1
	.byte	0x1
	.uleb128 0xf
	.4byte	0x33
	.4byte	0x21b
	.uleb128 0x13
	.byte	0
	.uleb128 0x14
	.4byte	.LASF25
	.byte	0x4
	.2byte	0x18a
	.4byte	0x229
	.byte	0x1
	.byte	0x1
	.uleb128 0xb
	.4byte	0x210
	.uleb128 0x15
	.byte	0x1
	.4byte	.LASF26
	.byte	0x5
	.byte	0x1c
	.byte	0x1
	.4byte	0x48
	.byte	0x1
	.uleb128 0x15
	.byte	0x1
	.4byte	.LASF27
	.byte	0x5
	.byte	0x1d
	.byte	0x1
	.4byte	0x48
	.byte	0x1
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
	.uleb128 0x4
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
	.uleb128 0x5
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
	.uleb128 0x6
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
	.uleb128 0x7
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
	.uleb128 0x8
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
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
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
	.uleb128 0xd
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
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x35
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x11
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
	.uleb128 0x12
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
	.uleb128 0x13
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x14
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
	.uleb128 0x15
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
	.4byte	.LFB182-.Ltext0
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
	.4byte	.LFE182-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL1-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL1-1-.Ltext0
	.4byte	.LFE182-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL1-1-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL1-1-.Ltext0
	.4byte	.LFE182-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL3-.Ltext0
	.2byte	0x2
	.byte	0x33
	.byte	0x9f
	.4byte	.LVL3-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x2
	.byte	0x34
	.byte	0x9f
	.4byte	.LVL4-.Ltext0
	.4byte	.LVL5-.Ltext0
	.2byte	0x2
	.byte	0x35
	.byte	0x9f
	.4byte	.LVL5-.Ltext0
	.4byte	.LVL6-.Ltext0
	.2byte	0x2
	.byte	0x36
	.byte	0x9f
	.4byte	.LVL6-.Ltext0
	.4byte	.LVL7-.Ltext0
	.2byte	0x2
	.byte	0x37
	.byte	0x9f
	.4byte	.LVL7-.Ltext0
	.4byte	.LVL8-.Ltext0
	.2byte	0x2
	.byte	0x38
	.byte	0x9f
	.4byte	.LVL8-.Ltext0
	.4byte	.LVL9-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL9-.Ltext0
	.4byte	.LVL10-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL11-.Ltext0
	.2byte	0x2
	.byte	0x32
	.byte	0x9f
	.4byte	.LVL11-.Ltext0
	.4byte	.LVL12-.Ltext0
	.2byte	0x2
	.byte	0x33
	.byte	0x9f
	.4byte	.LVL19-.Ltext0
	.4byte	.LVL20-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	.LVL20-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL21-.Ltext0
	.4byte	.LVL22-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL22-.Ltext0
	.4byte	.LVL23-.Ltext0
	.2byte	0x2
	.byte	0x32
	.byte	0x9f
	.4byte	.LVL23-.Ltext0
	.4byte	.LVL24-.Ltext0
	.2byte	0x2
	.byte	0x33
	.byte	0x9f
	.4byte	.LVL24-.Ltext0
	.4byte	.LFE182-.Ltext0
	.2byte	0x2
	.byte	0x34
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL13-.Ltext0
	.4byte	.LVL14-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL14-.Ltext0
	.4byte	.LVL15-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL15-.Ltext0
	.4byte	.LVL16-.Ltext0
	.2byte	0x2
	.byte	0x32
	.byte	0x9f
	.4byte	.LVL16-.Ltext0
	.4byte	.LVL18-.Ltext0
	.2byte	0x2
	.byte	0x33
	.byte	0x9f
	.4byte	.LVL18-.Ltext0
	.4byte	.LFE182-.Ltext0
	.2byte	0x2
	.byte	0x34
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL8-.Ltext0
	.4byte	.LVL12-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	.LVL13-.Ltext0
	.4byte	.LVL17-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL17-.Ltext0
	.4byte	.LVL19-.Ltext0
	.2byte	0x8
	.byte	0x74
	.sleb128 0
	.byte	0x34
	.byte	0x24
	.byte	0x71
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.4byte	.LVL19-.Ltext0
	.4byte	.LVL20-.Ltext0
	.2byte	0x8
	.byte	0x74
	.sleb128 -1
	.byte	0x34
	.byte	0x24
	.byte	0x71
	.sleb128 0
	.byte	0x22
	.byte	0x9f
	.4byte	.LVL20-.Ltext0
	.4byte	.LFE182-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x2
	.byte	0x32
	.byte	0x9f
	.4byte	.LVL2-.Ltext0
	.4byte	.LFE182-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST7:
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
.LLST8:
	.4byte	.LVL25-.Ltext0
	.4byte	.LVL27-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL27-.Ltext0
	.4byte	.LVL33-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	.LVL33-.Ltext0
	.4byte	.LVL37-.Ltext0
	.2byte	0x3
	.byte	0x75
	.sleb128 -16
	.byte	0x9f
	.4byte	.LVL37-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	0
	.4byte	0
.LLST9:
	.4byte	.LVL25-.Ltext0
	.4byte	.LVL38-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL38-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	0
	.4byte	0
.LLST10:
	.4byte	.LVL26-.Ltext0
	.4byte	.LVL27-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST11:
	.4byte	.LVL27-.Ltext0
	.4byte	.LVL28-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL28-.Ltext0
	.4byte	.LVL29-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL29-.Ltext0
	.4byte	.LVL30-.Ltext0
	.2byte	0x2
	.byte	0x32
	.byte	0x9f
	.4byte	.LVL30-.Ltext0
	.4byte	.LVL31-.Ltext0
	.2byte	0x2
	.byte	0x33
	.byte	0x9f
	.4byte	.LVL31-.Ltext0
	.4byte	.LVL32-.Ltext0
	.2byte	0x2
	.byte	0x34
	.byte	0x9f
	.4byte	.LVL32-.Ltext0
	.4byte	.LVL34-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL34-.Ltext0
	.4byte	.LVL35-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL35-.Ltext0
	.4byte	.LVL36-.Ltext0
	.2byte	0x2
	.byte	0x32
	.byte	0x9f
	.4byte	.LVL36-.Ltext0
	.4byte	.LVL37-.Ltext0
	.2byte	0x2
	.byte	0x33
	.byte	0x9f
	.4byte	.LVL37-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x2
	.byte	0x34
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST12:
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
.LLST13:
	.4byte	.LVL39-.Ltext0
	.4byte	.LVL40-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL40-.Ltext0
	.4byte	.LVL41-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL41-.Ltext0
	.4byte	.LVL42-.Ltext0
	.2byte	0x2
	.byte	0x32
	.byte	0x9f
	.4byte	.LVL42-.Ltext0
	.4byte	.LVL43-.Ltext0
	.2byte	0x2
	.byte	0x33
	.byte	0x9f
	.4byte	.LVL43-.Ltext0
	.4byte	.LVL44-.Ltext0
	.2byte	0x2
	.byte	0x34
	.byte	0x9f
	.4byte	.LVL44-.Ltext0
	.4byte	.LVL45-.Ltext0
	.2byte	0x2
	.byte	0x35
	.byte	0x9f
	.4byte	.LVL45-.Ltext0
	.4byte	.LVL46-.Ltext0
	.2byte	0x2
	.byte	0x36
	.byte	0x9f
	.4byte	.LVL46-.Ltext0
	.4byte	.LVL47-.Ltext0
	.2byte	0x2
	.byte	0x37
	.byte	0x9f
	.4byte	.LVL47-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x2
	.byte	0x38
	.byte	0x9f
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
.LASF24:
	.ascii	"console_printk\000"
.LASF15:
	.ascii	"acon_setting\000"
.LASF27:
	.ascii	"masp_hal_secure_algo_deinit\000"
.LASF28:
	.ascii	"GNU C 4.7\000"
.LASF18:
	.ascii	"p_src\000"
.LASF19:
	.ascii	"src_len\000"
.LASF12:
	.ascii	"float\000"
.LASF14:
	.ascii	"g_AC_CFG\000"
.LASF30:
	.ascii	"ALPS_SW/M"
	.ascii	"P/KK1.MP7/alps/out/target/product/mt6571v1_phone/ob"
	.ascii	"j/KERNEL_OBJ\000"
.LASF5:
	.ascii	"unsigned char\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF7:
	.ascii	"short unsigned int\000"
.LASF21:
	.ascii	"HACC_V3_Terminate\000"
.LASF29:
	.ascii	"ALPS_SW/M"
	.ascii	"P/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/"
	.ascii	"kernel/drivers/masp/mach/hacc_hk.c\000"
.LASF23:
	.ascii	"elf_hwcap\000"
.LASF17:
	.ascii	"HACC_V3_Run\000"
.LASF31:
	.ascii	"uint32\000"
.LASF26:
	.ascii	"masp_hal_secure_algo_init\000"
.LASF3:
	.ascii	"unsigned int\000"
.LASF9:
	.ascii	"long long unsigned int\000"
.LASF16:
	.ascii	"HACC_V3_Init\000"
.LASF20:
	.ascii	"p_dst\000"
.LASF1:
	.ascii	"sizetype\000"
.LASF8:
	.ascii	"long long int\000"
.LASF2:
	.ascii	"char\000"
.LASF22:
	.ascii	"g_CFG_RANDOM_PATTERN\000"
.LASF6:
	.ascii	"short int\000"
.LASF25:
	.ascii	"hex_asc\000"
.LASF10:
	.ascii	"long int\000"
.LASF13:
	.ascii	"encode\000"
.LASF4:
	.ascii	"signed char\000"
.LASF11:
	.ascii	"_Bool\000"
	.ident	"GCC: (GNU) 4.7"
	.section	.note.GNU-stack,"",%progbits
