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
	.file	"hacc_lib.c"
	.text
.Ltext0:
	.align	2
	.global	masp_hal_secure_algo_init
	.type	masp_hal_secure_algo_init, %function
masp_hal_secure_algo_init:
.LFB182:
	.file 1 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/mach/hacc_lib.c"
	.loc 1 60 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
.LVL0:
	.loc 1 64 0
	mov	r0, #1
	ldmfd	sp, {fp, sp, pc}
.LFE182:
	.size	masp_hal_secure_algo_init, .-masp_hal_secure_algo_init
	.align	2
	.global	masp_hal_secure_algo_deinit
	.type	masp_hal_secure_algo_deinit, %function
masp_hal_secure_algo_deinit:
.LFB183:
	.loc 1 70 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI3:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI4:
	sub	fp, ip, #4
.LCFI5:
.LVL1:
	.loc 1 74 0
	mov	r0, #1
	ldmfd	sp, {fp, sp, pc}
.LFE183:
	.size	masp_hal_secure_algo_deinit, .-masp_hal_secure_algo_deinit
	.align	2
	.global	masp_hal_secure_algo
	.type	masp_hal_secure_algo, %function
masp_hal_secure_algo:
.LFB184:
	.loc 1 87 0
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL2:
	mov	ip, sp
.LCFI6:
	stmfd	sp!, {r4, r5, r6, r7, r8, fp, ip, lr, pc}
.LCFI7:
	sub	fp, ip, #4
.LCFI8:
	sub	sp, sp, #12
	ldr	r5, [fp, #4]
.LVL3:
	mov	r6, r0
	mov	r8, r1
	mov	r4, r2
	mov	r7, r3
.LVL4:
.L4:
	.loc 1 96 0 discriminator 1
	bl	osal_hacc_lock
.LVL5:
	.loc 1 97 0 discriminator 1
	cmp	r0, #0
	bne	.L4
.LVL6:
	.loc 1 101 0
	mov	r0, r7
.LVL7:
	mov	r1, #16
	bl	masp_hal_sp_hacc_init
.LVL8:
	.loc 1 103 0
	subs	r7, r0, #0
.LVL9:
	bne	.L5
.LVL10:
	.loc 1 113 0
	cmp	r6, #0
	bne	.L18
	.loc 1 126 0
	str	r6, [sp, #0]
	mov	r0, r8
.LVL11:
	mov	r1, r4
	mov	r2, #1
	mov	r3, #2
	bl	masp_hal_sp_hacc_dec
.LVL12:
.L8:
	.loc 1 136 0
	cmp	r4, #0
	movne	r3, #0
	beq	.L10
.LVL13:
.L9:
	.loc 1 138 0 discriminator 2
	ldrb	ip, [r0, r3]	@ zero_extendqisi2
	strb	ip, [r5, r3]
	.loc 1 136 0 discriminator 2
	add	r3, r3, #1
.LVL14:
	cmp	r3, r4
	bne	.L9
.LVL15:
.L10:
	.loc 1 142 0
	bl	osal_hacc_unlock
.LVL16:
	.loc 1 153 0
	sub	sp, fp, #32
	ldmfd	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
.LVL17:
.L18:
	.loc 1 113 0
	cmp	r6, #1
	.loc 1 130 0
	movne	r7, #24576
	.loc 1 113 0
	beq	.L19
.LVL18:
.L5:
	.loc 1 148 0
	bl	osal_hacc_unlock
.LVL19:
	.loc 1 150 0
	ldr	r1, .L20
	mov	r2, r7
	ldr	r0, .L20+4
	bl	printk
.LVL20:
	.loc 1 151 0
#APP
@ 151 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/mach/hacc_lib.c" 1
	.word 0xe7f001f2
@ 0 "" 2
.LVL21:
.L19:
	.loc 1 119 0
	str	r7, [sp, #0]
	mov	r0, r8
.LVL22:
	mov	r1, r4
	mov	r2, r6
	mov	r3, #2
	bl	masp_hal_sp_hacc_enc
.LVL23:
	.loc 1 120 0
	b	.L8
.L21:
	.align	2
.L20:
	.word	.LC1
	.word	.LC0
.LFE184:
	.size	masp_hal_secure_algo, .-masp_hal_secure_algo
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"[%s] masp_hal_secure_algo error (0x%x)\012\000"
.LC1:
	.ascii	"CE\000"
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
	.4byte	.LCFI8-.LCFI7
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE4:
	.text
.Letext0:
	.file 2 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/../mediatek/kernel/include/mach/mt_sec_hal.h"
	.file 3 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/kernel/drivers/masp/asf/asf_inc/sec_typedef.h"
	.file 4 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/arch/arm/include/asm/hwcap.h"
	.file 5 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/include/linux/printk.h"
	.file 6 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/include/linux/kernel.h"
	.file 7 "ALPS_SW/MP/KK1.MP7/alps/kernel-3.4/../mediatek/kernel/include/mach/sec_osal.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x37e
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF34
	.byte	0x1
	.4byte	.LASF35
	.4byte	.LASF36
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
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.4byte	.LASF11
	.uleb128 0x6
	.byte	0x4
	.byte	0x2
	.byte	0x12
	.4byte	0xb2
	.uleb128 0x7
	.4byte	.LASF12
	.sleb128 0
	.uleb128 0x7
	.4byte	.LASF13
	.sleb128 1
	.uleb128 0x7
	.4byte	.LASF14
	.sleb128 2
	.uleb128 0x7
	.4byte	.LASF15
	.sleb128 3
	.byte	0
	.uleb128 0x8
	.4byte	.LASF17
	.byte	0x2
	.byte	0x17
	.4byte	0x91
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.4byte	.LASF16
	.uleb128 0x8
	.4byte	.LASF18
	.byte	0x3
	.byte	0xe
	.4byte	0x45
	.uleb128 0x8
	.4byte	.LASF19
	.byte	0x3
	.byte	0xf
	.4byte	0x53
	.uleb128 0x9
	.byte	0x1
	.4byte	.LASF20
	.byte	0x1
	.byte	0x3b
	.byte	0x1
	.4byte	0x53
	.4byte	.LFB182
	.4byte	.LFE182
	.4byte	.LLST0
	.byte	0x1
	.4byte	0x105
	.uleb128 0xa
	.ascii	"ret\000"
	.byte	0x1
	.byte	0x3d
	.4byte	0xbd
	.byte	0x1
	.byte	0
	.uleb128 0x9
	.byte	0x1
	.4byte	.LASF21
	.byte	0x1
	.byte	0x45
	.byte	0x1
	.4byte	0x53
	.4byte	.LFB183
	.4byte	.LFE183
	.4byte	.LLST1
	.byte	0x1
	.4byte	0x130
	.uleb128 0xa
	.ascii	"ret\000"
	.byte	0x1
	.byte	0x47
	.4byte	0xbd
	.byte	0x1
	.byte	0
	.uleb128 0xb
	.byte	0x1
	.4byte	.LASF37
	.byte	0x1
	.byte	0x56
	.byte	0x1
	.4byte	.LFB184
	.4byte	.LFE184
	.4byte	.LLST2
	.byte	0x1
	.4byte	0x296
	.uleb128 0xc
	.4byte	.LASF22
	.byte	0x1
	.byte	0x56
	.4byte	0x53
	.4byte	.LLST3
	.uleb128 0xc
	.4byte	.LASF23
	.byte	0x1
	.byte	0x56
	.4byte	0x45
	.4byte	.LLST4
	.uleb128 0xc
	.4byte	.LASF24
	.byte	0x1
	.byte	0x56
	.4byte	0x45
	.4byte	.LLST5
	.uleb128 0xc
	.4byte	.LASF25
	.byte	0x1
	.byte	0x56
	.4byte	0x84
	.4byte	.LLST6
	.uleb128 0xd
	.4byte	.LASF26
	.byte	0x1
	.byte	0x56
	.4byte	0x84
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0xe
	.ascii	"err\000"
	.byte	0x1
	.byte	0x58
	.4byte	0xc4
	.4byte	.LLST7
	.uleb128 0xe
	.ascii	"src\000"
	.byte	0x1
	.byte	0x59
	.4byte	0x296
	.4byte	.LLST8
	.uleb128 0xe
	.ascii	"dst\000"
	.byte	0x1
	.byte	0x59
	.4byte	0x296
	.4byte	.LLST9
	.uleb128 0xf
	.4byte	.LASF27
	.byte	0x1
	.byte	0x59
	.4byte	0x296
	.4byte	.LLST10
	.uleb128 0xe
	.ascii	"i\000"
	.byte	0x1
	.byte	0x5a
	.4byte	0xc4
	.4byte	.LLST11
	.uleb128 0x10
	.4byte	.LASF38
	.byte	0x1
	.byte	0x92
	.4byte	.L5
	.uleb128 0x11
	.4byte	.LVL5
	.4byte	0x2df
	.uleb128 0x12
	.4byte	.LVL8
	.4byte	0x2ed
	.4byte	0x20a
	.uleb128 0x13
	.byte	0x1
	.byte	0x51
	.byte	0x1
	.byte	0x40
	.uleb128 0x13
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x77
	.sleb128 0
	.byte	0
	.uleb128 0x12
	.4byte	.LVL12
	.4byte	0x30a
	.4byte	0x235
	.uleb128 0x13
	.byte	0x1
	.byte	0x53
	.byte	0x1
	.byte	0x32
	.uleb128 0x13
	.byte	0x1
	.byte	0x52
	.byte	0x1
	.byte	0x31
	.uleb128 0x13
	.byte	0x1
	.byte	0x51
	.byte	0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x13
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x78
	.sleb128 0
	.uleb128 0x13
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.byte	0x2
	.byte	0x76
	.sleb128 0
	.byte	0
	.uleb128 0x11
	.4byte	.LVL16
	.4byte	0x336
	.uleb128 0x11
	.4byte	.LVL19
	.4byte	0x336
	.uleb128 0x12
	.4byte	.LVL20
	.4byte	0x340
	.4byte	0x26d
	.uleb128 0x13
	.byte	0x1
	.byte	0x52
	.byte	0x2
	.byte	0x77
	.sleb128 0
	.uleb128 0x13
	.byte	0x1
	.byte	0x51
	.byte	0x5
	.byte	0x3
	.4byte	.LC1
	.uleb128 0x13
	.byte	0x1
	.byte	0x50
	.byte	0x5
	.byte	0x3
	.4byte	.LC0
	.byte	0
	.uleb128 0x14
	.4byte	.LVL23
	.4byte	0x359
	.uleb128 0x13
	.byte	0x1
	.byte	0x53
	.byte	0x1
	.byte	0x32
	.uleb128 0x13
	.byte	0x1
	.byte	0x52
	.byte	0x2
	.byte	0x76
	.sleb128 0
	.uleb128 0x13
	.byte	0x1
	.byte	0x51
	.byte	0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x13
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x78
	.sleb128 0
	.uleb128 0x13
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.byte	0x2
	.byte	0x77
	.sleb128 0
	.byte	0
	.byte	0
	.uleb128 0x3
	.byte	0x4
	.4byte	0xcf
	.uleb128 0x15
	.4byte	.LASF28
	.byte	0x4
	.byte	0x24
	.4byte	0x45
	.byte	0x1
	.byte	0x1
	.uleb128 0x16
	.4byte	0x6f
	.4byte	0x2b4
	.uleb128 0x17
	.byte	0
	.uleb128 0x15
	.4byte	.LASF29
	.byte	0x5
	.byte	0x1b
	.4byte	0x2a9
	.byte	0x1
	.byte	0x1
	.uleb128 0x16
	.4byte	0x3e
	.4byte	0x2cc
	.uleb128 0x17
	.byte	0
	.uleb128 0x18
	.4byte	.LASF30
	.byte	0x6
	.2byte	0x18a
	.4byte	0x2da
	.byte	0x1
	.byte	0x1
	.uleb128 0x4
	.4byte	0x2c1
	.uleb128 0x19
	.byte	0x1
	.4byte	.LASF39
	.byte	0x7
	.byte	0x16
	.byte	0x1
	.4byte	0x6f
	.byte	0x1
	.uleb128 0x1a
	.byte	0x1
	.4byte	.LASF31
	.byte	0x2
	.byte	0x1f
	.byte	0x1
	.4byte	0x45
	.byte	0x1
	.4byte	0x30a
	.uleb128 0x1b
	.4byte	0x84
	.uleb128 0x1b
	.4byte	0x45
	.byte	0
	.uleb128 0x1a
	.byte	0x1
	.4byte	.LASF32
	.byte	0x2
	.byte	0x22
	.byte	0x1
	.4byte	0x84
	.byte	0x1
	.4byte	0x336
	.uleb128 0x1b
	.4byte	0x84
	.uleb128 0x1b
	.4byte	0x45
	.uleb128 0x1b
	.4byte	0x53
	.uleb128 0x1b
	.4byte	0xb2
	.uleb128 0x1b
	.4byte	0x53
	.byte	0
	.uleb128 0x1c
	.byte	0x1
	.4byte	.LASF40
	.byte	0x7
	.byte	0x17
	.byte	0x1
	.byte	0x1
	.uleb128 0x1a
	.byte	0x1
	.4byte	.LASF33
	.byte	0x5
	.byte	0x6a
	.byte	0x1
	.4byte	0x6f
	.byte	0x1
	.4byte	0x359
	.uleb128 0x1b
	.4byte	0x33
	.uleb128 0x1d
	.byte	0
	.uleb128 0x1e
	.byte	0x1
	.4byte	.LASF41
	.byte	0x2
	.byte	0x21
	.byte	0x1
	.4byte	0x84
	.byte	0x1
	.uleb128 0x1b
	.4byte	0x84
	.uleb128 0x1b
	.4byte	0x45
	.uleb128 0x1b
	.4byte	0x53
	.uleb128 0x1b
	.4byte	0xb2
	.uleb128 0x1b
	.4byte	0x53
	.byte	0
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
	.uleb128 0x7
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0
	.byte	0
	.uleb128 0x8
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
	.uleb128 0x9
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
	.uleb128 0xa
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
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xb
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
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0xd
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
	.uleb128 0xe
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
	.uleb128 0xf
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
	.uleb128 0x10
	.uleb128 0xa
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x12
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
	.uleb128 0x13
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0xa
	.uleb128 0x2111
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
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
	.uleb128 0x16
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x18
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
	.uleb128 0x19
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
	.uleb128 0x1a
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
	.uleb128 0x1b
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1c
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
	.uleb128 0x1d
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x1e
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
.LLST2:
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
.LLST3:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL4-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL4-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x1
	.byte	0x58
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL4-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL4-.Ltext0
	.4byte	.LVL9-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	.LVL9-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x53
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST7:
	.4byte	.LVL5-.Ltext0
	.4byte	.LVL7-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL8-.Ltext0
	.4byte	.LVL11-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL11-.Ltext0
	.4byte	.LVL12-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	.LVL17-.Ltext0
	.4byte	.LVL18-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL18-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	.LVL21-.Ltext0
	.4byte	.LVL22-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST8:
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL18-.Ltext0
	.2byte	0x1
	.byte	0x58
	.4byte	.LVL21-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x1
	.byte	0x58
	.4byte	0
	.4byte	0
.LLST9:
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL12-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	.LVL12-.Ltext0
	.4byte	.LVL16-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL17-.Ltext0
	.4byte	.LVL18-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	.LVL21-.Ltext0
	.4byte	.LVL23-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	.LVL23-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST10:
	.4byte	.LVL6-.Ltext0
	.4byte	.LVL9-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	.LVL9-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x53
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST11:
	.4byte	.LVL3-.Ltext0
	.4byte	.LVL13-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL14-.Ltext0
	.4byte	.LVL15-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL17-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x2
	.byte	0x30
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
.LASF29:
	.ascii	"console_printk\000"
.LASF16:
	.ascii	"_Bool\000"
.LASF34:
	.ascii	"GNU C 4.7\000"
.LASF32:
	.ascii	"masp_hal_sp_hacc_dec\000"
.LASF11:
	.ascii	"float\000"
.LASF36:
	.ascii	"ALPS_SW/M"
	.ascii	"P/KK1.MP7/alps/out/target/product/mt6571v1_phone/ob"
	.ascii	"j/KERNEL_OBJ\000"
.LASF5:
	.ascii	"unsigned char\000"
.LASF25:
	.ascii	"CustomSeed\000"
.LASF41:
	.ascii	"masp_hal_sp_hacc_enc\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF37:
	.ascii	"masp_hal_secure_algo\000"
.LASF4:
	.ascii	"short unsigned int\000"
.LASF19:
	.ascii	"uchar\000"
.LASF26:
	.ascii	"ResText\000"
.LASF28:
	.ascii	"elf_hwcap\000"
.LASF35:
	.ascii	"ALPS_SW/M"
	.ascii	"P/KK1.MP7/alps/kernel-3.4/mediatek/platform/mt6571/"
	.ascii	"kernel/drivers/masp/mach/hacc_lib.c\000"
.LASF30:
	.ascii	"hex_asc\000"
.LASF24:
	.ascii	"ContentLen\000"
.LASF18:
	.ascii	"uint32\000"
.LASF31:
	.ascii	"masp_hal_sp_hacc_init\000"
.LASF20:
	.ascii	"masp_hal_secure_algo_init\000"
.LASF3:
	.ascii	"unsigned int\000"
.LASF40:
	.ascii	"osal_hacc_unlock\000"
.LASF39:
	.ascii	"osal_hacc_lock\000"
.LASF10:
	.ascii	"long long unsigned int\000"
.LASF17:
	.ascii	"HACC_USER\000"
.LASF1:
	.ascii	"sizetype\000"
.LASF9:
	.ascii	"long long int\000"
.LASF2:
	.ascii	"char\000"
.LASF21:
	.ascii	"masp_hal_secure_algo_deinit\000"
.LASF7:
	.ascii	"short int\000"
.LASF33:
	.ascii	"printk\000"
.LASF27:
	.ascii	"seed\000"
.LASF22:
	.ascii	"Direction\000"
.LASF6:
	.ascii	"long int\000"
.LASF23:
	.ascii	"ContentAddr\000"
.LASF8:
	.ascii	"signed char\000"
.LASF12:
	.ascii	"HACC_USER1\000"
.LASF13:
	.ascii	"HACC_USER2\000"
.LASF14:
	.ascii	"HACC_USER3\000"
.LASF15:
	.ascii	"HACC_USER4\000"
.LASF38:
	.ascii	"_error\000"
	.ident	"GCC: (GNU) 4.7"
	.section	.note.GNU-stack,"",%progbits
