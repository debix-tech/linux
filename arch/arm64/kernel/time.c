// SPDX-License-Identifier: GPL-2.0-only
/*
 * Based on arch/arm/kernel/time.c
 *
 * Copyright (C) 1991, 1992, 1995  Linus Torvalds
 * Modifications for ARM (C) 1994-2001 Russell King
 * Copyright (C) 2012 ARM Ltd.
 */

#include <linux/clockchips.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/timex.h>
#include <linux/errno.h>
#include <linux/profile.h>
#include <linux/stacktrace.h>
#include <linux/syscore_ops.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/clocksource.h>
#include <linux/of_clk.h>
#include <linux/acpi.h>
#include <linux/gpio.h>

#include <clocksource/arm_arch_timer.h>

#include <asm/thread_info.h>
#include <asm/paravirt.h>

static bool profile_pc_cb(void *arg, unsigned long pc)
{
	unsigned long *prof_pc = arg;

	if (in_lock_functions(pc))
		return true;
	*prof_pc = pc;
	return false;
}

unsigned long profile_pc(struct pt_regs *regs)
{
	unsigned long prof_pc = 0;

	arch_stack_walk(profile_pc_cb, &prof_pc, current, regs);

	return prof_pc;
}
EXPORT_SYMBOL(profile_pc);

void __init time_init(void)
{
	u32 arch_timer_rate;
//John_gao add for uefi
#if 0
	void __iomem *regs = ioremap(0x4ac10000, 4);
	writeb(0, regs);
	iounmap(regs);

	regs = ioremap(0x4ac10001, 4);
	writeb(0, (regs));
	iounmap(regs);

	regs = ioremap(0x4ac10004, 4);
	writeb(0xFF, (regs));
	iounmap(regs);

	regs = ioremap(0x4ac10005, 4);
	writeb(0x1F, (regs));
	iounmap(regs);

	regs = ioremap(0x4ae30013, 4);
	writeb(0, (regs));
	iounmap(regs);

	mdelay(10);

	regs = ioremap(0x4ae30217, 4);
	writeb(0x9, (regs));
	iounmap(regs);

#endif
#if 0
	int ret = 0;
	void __iomem *regs;
	//John_gao set pwm down
	regs = ioremap(0x443C01F4, 4);
	printk("GLS 0x443C01F4 = 0x%x \n", readl((regs)));
	writel(0x51E , (regs));
	iounmap(regs);
	printk("GLS 0x443C01F4 = 0x%x \n", readl((regs)));

	regs = ioremap(0x443C01F0, 4);
	printk("GLS 0x443C01F0 = 0x%x \n", readl((regs)));
	writel(0x51E , (regs));
	iounmap(regs);
	printk("GLS 0x443C01F0 = 0x%x \n", readl((regs)));

	regs = ioremap(0x43810080, 4);
	printk("GLS 01 0x43810080 = 0x%x \n", readl((regs)));

	ret = gpio_request(45, "pwm-gpio-01");
	printk("GLS 01 45 gpio request ret = %d\n", ret);
	ret = gpio_direction_output(45, 1);
	gpio_set_value(45,1);
	printk("GLS 02 0x43810080 = 0x%x ret = %d\n", readl((regs)), ret);
	ret = gpio_get_value(45);
	printk("GLS 03 0x43810080 = 0x%x ret = %d\n", readl((regs)), ret);
	ret = gpio_request(44, "power-gpio-02");
	printk("GLS 02 44 gpio request ret = %d\n", ret);
	ret = gpio_direction_output(44, 0);
	iounmap(regs);
#endif
//end John_gao add for uefi
	of_clk_init(NULL);
	timer_probe();

	tick_setup_hrtimer_broadcast();

	arch_timer_rate = arch_timer_get_rate();
	if (!arch_timer_rate)
		panic("Unable to initialise architected timer.\n");

	/* Calibrate the delay loop directly */
	lpj_fine = arch_timer_rate / HZ;

	pv_time_init();
}
