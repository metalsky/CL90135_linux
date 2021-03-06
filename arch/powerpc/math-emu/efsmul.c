/*
 * arch/powerpc/math-emu/efsmul.c
 *
 * Copyright (C) 2006 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Author: Ebony Zhu, ebony.zhu@freescale.com
 *
 * Description:
 * This file is the implementation of SPE instruction "efsmul"
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "spe.h"
#include "soft-fp.h"
#include "single.h"
int
efsmul(void *rD, void *rA, void *rB)
{
	FP_DECL_S(A);
	FP_DECL_S(B);
	FP_DECL_S(R);
	int ret = 0;
#ifdef DEBUG
	printk("%s: %p %p %p\n", __FUNCTION__, rD, rA, rB);
#endif

	__FP_UNPACK_S(A, rA);
	__FP_UNPACK_S(B, rB);

#ifdef DEBUG
        printk("A: %ld %lu %ld (%ld) [%08lx %lx]\n",
               A_s, A_f, A_e, A_c, A_f, A_e + 127);
        printk("B: %ld %lu %ld (%ld) [%08lx %lx]\n",
               B_s, B_f, B_e, B_c, B_f, B_e + 127);
#endif
	
	FP_MUL_S(R, A, B);

#ifdef DEBUG
	printk("D: %ld %lu %ld (%ld) [%08lx %lx]\n",
	       R_s, R_f, R_e, R_c, R_f, R_e + 127);
#endif

	return (ret | __FP_PACK_S(rD, R));
}

