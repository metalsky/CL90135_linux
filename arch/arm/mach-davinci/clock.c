/*
 * TI DaVinci clock config file
 *
 * Copyright (C) 2006 Texas Instruments.
 * Copyright (C) 2008 MontaVista Software, Inc. <source@mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include <asm/hardware.h>

#include <asm/arch/clock.h>

static unsigned int commonrate;
static unsigned int div_by_four;
static unsigned int div_by_six;
static unsigned int armrate;
static unsigned int fixedrate;

static struct clk davinci_dm644x_clks[] = {
	{
		.name = "ARMCLK",
		.rate = &armrate,
		.lpsc = -1,
		.flags = ALWAYS_ENABLED,
	},
	{
		.name = "UART0",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_UART0,
	},
	{
		.name = "EMACCLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_EMAC_WRAPPER,
	},
	{
		.name = "I2CCLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_I2C,
	},
	{
		.name = "IDECLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_ATA,
	},
	{
		.name = "McBSPCLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_McBSP0,
	},
	{
		.name = "MMCSDCLK0",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_MMC_SD0,
	},
	{
		.name = "SPICLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_SPI,
	},
	{
		.name = "gpio",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_GPIO,
	},
	{
		.name = "AEMIFCLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_AEMIF,
		.usecount = 1,
	},
	{
		.name = "PWM0_CLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_PWM0,
	},
	{
		.name = "PWM1_CLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_PWM1,
	},
	{
		.name = "PWM2_CLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_PWM2,
	},
	{
		.name = "USBCLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_USB,
	},
	{
		.name = "VLYNQ_CLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_VLYNQ,
	},
};
static struct clk davinci_dm6467_clks[] = {
	{
		.name = "ARMCLK",
		.rate = &armrate,
		.lpsc = -1,
		.flags = ALWAYS_ENABLED,
	},
	{
		.name = "UART0",
		.rate = &fixedrate,
		.lpsc = DAVINCI_DM646X_LPSC_UART0,
	},
	{
		.name = "UART1",
		.rate = &fixedrate,
		.lpsc = DAVINCI_DM646X_LPSC_UART1,
	},
	{
		.name = "UART2",
		.rate = &fixedrate,
		.lpsc = DAVINCI_DM646X_LPSC_UART2,
	},
	{
		.name = "EMACCLK",
		.rate = &div_by_four,
		.lpsc = DAVINCI_DM646X_LPSC_EMAC,
	},
	{
		.name = "I2CCLK",
		.rate = &div_by_four,
		.lpsc = DAVINCI_DM646X_LPSC_I2C,
	},
	{
		.name = "IDECLK",
		.rate = &div_by_six,
		.lpsc = DAVINCI_LPSC_ATA,
	},
	{
		.name = "McASPCLK0",
		.rate = &div_by_four,
		.lpsc = DAVINCI_DM646X_LPSC_McASP0,
	},
	{
		.name = "McASPCLK1",
		.rate = &div_by_four,
		.lpsc = DAVINCI_DM646X_LPSC_McASP1,
	},
	{
		.name = "SPICLK",
		.rate = &div_by_four,
		.lpsc = DAVINCI_DM646X_LPSC_SPI,
	},
	{
		.name = "gpio",
		.rate = &commonrate,
		.lpsc = DAVINCI_DM646X_LPSC_GPIO,
	},
	{
		.name = "AEMIFCLK",
		.rate = &div_by_four,
		.lpsc = DAVINCI_DM646X_LPSC_AEMIF,
		.usecount = 1,
	},
	{
		.name = "PWM0_CLK",
		.rate = &div_by_four,
		.lpsc = DAVINCI_DM646X_LPSC_PWM0,
	},
	{
		.name = "PWM1_CLK",
		.rate = &div_by_four,
		.lpsc = DAVINCI_DM646X_LPSC_PWM1,
	},
	{
		.name = "USBCLK",
		.rate = &div_by_four,
		.lpsc = DAVINCI_LPSC_USB,
	},
	{
		.name = "VLYNQ_CLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_VLYNQ,
	},
	{
		.name = "TSIF0_CLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_DM646X_LPSC_TSIF0,
	},
	{
		.name = "TSIF1_CLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_DM646X_LPSC_TSIF1,
	},
};
static struct clk davinci_dm355_clks[] = {
	{
		.name = "ARMCLK",
		.rate = &armrate,
		.lpsc = -1,
		.flags = ALWAYS_ENABLED,
	},
	{
		.name = "UART0",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_UART0,
	},
	{
		.name = "UART1",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_UART1,
	},
	{
		.name = "UART2",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_UART2,
	},
	{
		.name = "EMACCLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_EMAC_WRAPPER,
	},
	{
		.name = "I2CCLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_I2C,
	},
	{
		.name = "IDECLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_ATA,
	},
	{
		.name = "McBSPCLK0",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_McBSP0,
	},
	{
		.name = "McBSPCLK1",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_McBSP1,
	},
	{
		.name = "MMCSDCLK0",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_MMC_SD0,
	},
	{
		.name = "MMCSDCLK1",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_MMC_SD1,
	},
	{
		.name = "SPICLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_SPI,
	},
	{
		.name = "gpio",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_GPIO,
	},
	{
		.name = "AEMIFCLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_AEMIF,
		.usecount = 1,
	},
	{
		.name = "PWM0_CLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_PWM0,
	},
	{
		.name = "PWM1_CLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_PWM1,
	},
	{
		.name = "PWM2_CLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_PWM2,
	},
	{
		.name = "PWM3_CLK",
		.rate = &fixedrate,
		.lpsc = DAVINCI_LPSC_PWM3,
	},
	{
		.name = "USBCLK",
		.rate = &commonrate,
		.lpsc = DAVINCI_LPSC_USB,
	},
};

static void dm644x_module_setup(unsigned ctlr, unsigned id, int enable)
{
	if (ctlr == 0 && id == DAVINCI_LPSC_MMC_SD) {
		/*
		 * VDD power manipulations are done in U-Boot for CPMAC,
		 * so apply to MMC as well -- set up the pull register.
		 */
		davinci_writel(0, DAVINCI_VDD3P3V_PWDN);
	}
}

static void dm646x_module_setup(unsigned ctlr, unsigned id, int enable)
{
	u32 val;

	if (ctlr)
		return;

	switch (id) {
	case DAVINCI_LPSC_USB:
		val = davinci_readl(DAVINCI_VDD3P3V_PWDN);
		if (enable)
			val &= ~0x10000000;
		else
			val |=	0x10000000;
		davinci_writel(val, DAVINCI_VDD3P3V_PWDN);
		break;
	case DAVINCI_DM646X_LPSC_HDVICP0:
	case DAVINCI_DM646X_LPSC_HDVICP1:
		if (enable) {
			val = davinci_readl(DAVINCI_VSCLKDIS);
			davinci_writel(val & ~0x00000F00, DAVINCI_VSCLKDIS);
		}

		val = davinci_readl(DAVINCI_VDD3P3V_PWDN);
		if (enable)
			val &= ~0x0000000F;
		else
			val |=	0x0000000F;
		davinci_writel(val, DAVINCI_VDD3P3V_PWDN);
		break;
	}
}

static unsigned long davinci_psc_base[] = { DAVINCI_PWR_SLEEP_CNTRL_BASE };

int __init davinci_clk_init(void)
{
	struct clk *clk_list;
	int num_clks;
	u32 pll_mult;

	davinci_psc_register(davinci_psc_base, 1);

	pll_mult = davinci_readl(DAVINCI_PLL_CNTRL0_BASE + PLLM);
	commonrate = ((pll_mult + 1) * 27000000) / 6;
	armrate = ((pll_mult + 1) * 27000000) / 2;

	if (cpu_is_davinci_dm355()) {
		/*
		 * FIXME
		 * We're assuming a 24MHz reference, but the DM355 also
		 * supports a 36MHz reference.
		 */
		unsigned long postdiv;

		/*
		 * Read the PLL1 POSTDIV register to determine if the post
		 * divider is /1 or /2
		 */
		postdiv = (davinci_readl(DAVINCI_PLL_CNTRL0_BASE + 0x128)
			& 0x1f) + 1;

		fixedrate = 24000000;
		armrate = (pll_mult + 1) * (fixedrate / (16 * postdiv));
		commonrate = armrate / 2;

		clk_list = davinci_dm355_clks;
		num_clks = ARRAY_SIZE(davinci_dm355_clks);
	} else if (cpu_is_davinci_dm6467()) {
		fixedrate = 24000000;
		div_by_four = ((pll_mult + 1) * 27000000) / 4;
		div_by_six = ((pll_mult + 1) * 27000000) / 6;
		armrate = ((pll_mult + 1) * 27000000) / 2;

		clk_list = davinci_dm6467_clks;
		num_clks = ARRAY_SIZE(davinci_dm6467_clks);

		davinci_module_setup = dm646x_module_setup;
	} else {

		fixedrate = 27000000;
		armrate = (pll_mult + 1) * (fixedrate / 2);
		commonrate = armrate / 3;

		clk_list = davinci_dm644x_clks;
		num_clks = ARRAY_SIZE(davinci_dm644x_clks);

		davinci_module_setup = dm644x_module_setup;
	}

	return davinci_enable_clks(clk_list, num_clks);
}
