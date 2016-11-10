/*
 * TI DaVinci GPIO Support
 *
 * Copyright (c) 2006 David Brownell
 * Copyright (c) 2007, MontaVista Software, Inc. <source@mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>

#include <asm/arch/irqs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>


/* If a new chip is added with number of GPIO greater than 128, please
   update DAVINCI_MAX_N_GPIO in include/asm-arm/arch-davinci/irqs.h */
#define DM646x_N_GPIO	48
#define DM644x_N_GPIO	71
#define DM355_N_GPIO	104

#ifdef CONFIG_ARCH_DAVINCI644x
static DECLARE_BITMAP(dm644x_gpio_in_use, DM644x_N_GPIO);
struct gpio_bank gpio_bank_dm6446 = {
	.base		= DAVINCI_GPIO_BASE,
	.num_gpio	= DM644x_N_GPIO,
	.irq_num	= IRQ_DM644X_GPIOBNK0,
	.irq_mask	= 0x1f,
	.in_use		= dm644x_gpio_in_use,
};
#endif
#ifdef CONFIG_ARCH_DAVINCI_DM355
static DECLARE_BITMAP(dm355_gpio_in_use, DM355_N_GPIO);
struct gpio_bank gpio_bank_dm355 = {
	.base		= DAVINCI_GPIO_BASE,
	.num_gpio	= DM355_N_GPIO,
	.irq_num	= IRQ_DM355_GPIOBNK0,
	.irq_mask	= 0x3f,
	.in_use		= dm355_gpio_in_use,
};
#endif

#ifdef CONFIG_ARCH_DAVINCI_DM646x
static DECLARE_BITMAP(dm646x_gpio_in_use, DM646x_N_GPIO);
struct gpio_bank gpio_bank_dm646x = {
	.base		= DAVINCI_GPIO_BASE,
	.num_gpio	= DM646x_N_GPIO,
	.irq_num	= IRQ_DM646X_GPIOBNK0,
	.irq_mask	= 0x1f,
	.in_use		= dm646x_gpio_in_use,
};
#endif

void davinci_gpio_init(void)
{
	struct gpio_bank *gpio_bank;

#ifdef CONFIG_ARCH_DAVINCI644x
	if (cpu_is_davinci_dm644x())
		gpio_bank = &gpio_bank_dm6446;
#endif
#ifdef CONFIG_ARCH_DAVINCI_DM355
	if (cpu_is_davinci_dm355())
		gpio_bank = &gpio_bank_dm355;
#endif
#ifdef CONFIG_ARCH_DAVINCI_DM646x
	if (cpu_is_davinci_dm6467())
		gpio_bank = &gpio_bank_dm646x;
#endif
	davinci_gpio_irq_setup(gpio_bank);
}
