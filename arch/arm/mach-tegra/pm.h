/*
 * arch/arm/mach-tegra/include/mach/pm.h
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * Author:
 *	Colin Cross <ccross@google.com>
 *
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _MACH_TEGRA_PM_H_
#define _MACH_TEGRA_PM_H_

#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/clkdev.h>

#include <mach/iomap.h>

#define PMC_SCRATCH0		0x50
#define PMC_SCRATCH1		0x54
#define PMC_SCRATCH4		0x60

#if defined(CONFIG_TEGRA_WDT_CLEAR_EARLY)
#define PINMUX_AUX_PWR_I2C_SCL_0		0x32b4
#define PINMUX_AUX_PWR_I2C_SCL_0_TRISTATE	0x10
#define PINMUX_AUX_PWR_I2C_SDA_0		0x32b8
#define PINMUX_AUX_PWR_I2C_SDA_0_TRISTATE	0x10
#define APB_MISC_GP_AOCFG1PADCTRL_0		0x868
#define SCHMT_ENABLE				0x8
#define CLK_RST_CONTROLLER_RST_DEV_H_SET_0	0x308
#define CLK_RST_CONTROLLER_RST_DEV_H_CLR_0	0x30c
#define CLK_RST_CONTROLLER_CLK_ENB_H_SET_0	0x328
#define CLK_RST_CONTROLLER_CLK_ENB_H_CLR_0	0x32c
#define I2C_CNFG				0x0
#define I2C_ADDR0				0x4
#define I2C_DATA1				0xc
#define I2C_DATA2				0x10
#define I2C_STATUS				0x1c
#endif

enum tegra_suspend_mode {
	TEGRA_SUSPEND_NONE = 0,
	TEGRA_SUSPEND_LP2,	/* CPU voltage off */
	TEGRA_SUSPEND_LP1,	/* CPU voltage off, DRAM self-refresh */
	TEGRA_SUSPEND_LP0,	/* CPU + core voltage off, DRAM self-refresh */
	TEGRA_MAX_SUSPEND_MODE,
};

enum suspend_stage {
	TEGRA_SUSPEND_BEFORE_PERIPHERAL,
	TEGRA_SUSPEND_BEFORE_CPU,
};

enum resume_stage {
	TEGRA_RESUME_AFTER_PERIPHERAL,
	TEGRA_RESUME_AFTER_CPU,
};

#if defined(CONFIG_TEGRA_LP1_LOW_COREVOLTAGE)
struct lpx_corev_reg_lookup {
	int core_voltage;
	int reg_value;
};
#endif

#if defined(CONFIG_TEGRA_WDT_CLEAR_EARLY)
#define MAX_WDT_INT_STATUS_REGS	3
struct pmic_reg_info {
	int addr;
	int mask;
};
#endif

struct tegra_suspend_platform_data {
	unsigned long cpu_timer;   /* CPU power good time in us,  LP2/LP1 */
	unsigned long cpu_off_timer;	/* CPU power off time us, LP2/LP1 */
	unsigned long core_timer;  /* core power good time in ticks,  LP0 */
	unsigned long core_off_timer;	/* core power off time ticks, LP0 */
	bool corereq_high;         /* Core power request active-high */
	bool sysclkreq_high;       /* System clock request is active-high */
	bool sysclkreq_gpio;       /* if System clock request is set to gpio */
	bool combined_req;         /* if core & CPU power requests are combined */
	enum tegra_suspend_mode suspend_mode;
	unsigned long cpu_lp2_min_residency; /* Min LP2 state residency in us */
	void (*board_suspend)(int lp_state, enum suspend_stage stg);
	/* lp_state = 0 for LP0 state, 1 for LP1 state, 2 for LP2 state */
	void (*board_resume)(int lp_state, enum resume_stage stg);
	unsigned int cpu_resume_boost;	/* CPU frequency resume boost in kHz */
#ifdef CONFIG_TEGRA_LP1_LOW_COREVOLTAGE
	bool lp1_lowvolt_support;
	unsigned int i2c_base_addr;
	unsigned int pmuslave_addr;
	unsigned int core_reg_addr;
	unsigned int lp1_core_volt_low_cold;
	unsigned int lp1_core_volt_low;
	unsigned int lp1_core_volt_high;
	int (*lp1_lookup_reg)(int voltage);
#endif
	unsigned int lp0bb_core_volt_min;
	unsigned int lp1bb_core_volt_min;
	unsigned long lp1bb_emc_rate_min;
	unsigned long lp1bb_emc_rate_max;
#ifdef CONFIG_ARCH_TEGRA_HAS_SYMMETRIC_CPU_PWR_GATE
	unsigned long min_residency_vmin_fmin;
	unsigned long min_residency_ncpu_slow;
	unsigned long min_residency_ncpu_fast;
	unsigned long min_residency_crail;
#endif
	unsigned long min_residency_mc_clk;
	bool usb_vbus_internal_wake; /* support for internal vbus wake */
	bool usb_id_internal_wake; /* support for internal id wake */
#ifdef CONFIG_TEGRA_WDT_CLEAR_EARLY
	bool wdt_clear_early_support;
	unsigned int i2c_clk_offset;
	struct pmic_reg_info wdt_clr_reg;
	struct pmic_reg_info wdt_int_status_regs[MAX_WDT_INT_STATUS_REGS];
#endif
};

/* clears io dpd settings before kernel code */
void tegra_bl_io_dpd_cleanup(void);

unsigned long tegra_cpu_power_good_time(void);
unsigned long tegra_cpu_power_off_time(void);
unsigned long tegra_cpu_lp2_min_residency(void);
unsigned long tegra_mc_clk_stop_min_residency(void);
#ifdef CONFIG_ARCH_TEGRA_HAS_SYMMETRIC_CPU_PWR_GATE
unsigned long tegra_min_residency_vmin_fmin(void);
unsigned long tegra_min_residency_ncpu(void);
unsigned long tegra_min_residency_crail(void);
#endif
void tegra_clear_cpu_in_pd(int cpu);
bool tegra_set_cpu_in_pd(int cpu);

void tegra_mc_clk_prepare(void);
void tegra_mc_clk_finish(void);
int tegra_suspend_dram(enum tegra_suspend_mode mode, unsigned int flags);
#ifdef CONFIG_TEGRA_LP1_LOW_COREVOLTAGE
int tegra_is_lp1_suspend_mode(void);
#endif
void tegra_lp1bb_suspend_emc_rate(unsigned long emc_min, unsigned long emc_max);
void tegra_lp1bb_suspend_mv_set(int mv);
unsigned long tegra_lp1bb_emc_min_rate_get(void);

void tegra_lp0bb_suspend_mv_set(int mv);
long set_lp0_pmc_registers(unsigned long rate);

#ifdef CONFIG_ARCH_TEGRA_14x_SOC
#define FLOW_CTRL_CLUSTER_CONTROL \
	(IO_ADDRESS(TEGRA_FLOW_CTRL_BASE) + 0x2c)
#endif

#define FLOW_CTRL_CPU_PWR_CSR \
	(IO_ADDRESS(TEGRA_FLOW_CTRL_BASE) + 0x38)
#define FLOW_CTRL_CPU_PWR_CSR_RAIL_ENABLE	1

#define FLOW_CTRL_MPID \
	(IO_ADDRESS(TEGRA_FLOW_CTRL_BASE) + 0x3c)

#define FLOW_CTRL_RAM_REPAIR \
	(IO_ADDRESS(TEGRA_FLOW_CTRL_BASE) + 0x40)
#define FLOW_CTRL_RAM_REPAIR_BYPASS_EN	(1<<2)

#define FUSE_SKU_DIRECT_CONFIG \
	(IO_ADDRESS(TEGRA_FUSE_BASE) + 0x1F4)
#define FUSE_SKU_DISABLE_ALL_CPUS	(1<<5)
#define FUSE_SKU_NUM_DISABLED_CPUS(x)	(((x) >> 3) & 3)

void __init tegra_init_suspend(struct tegra_suspend_platform_data *plat);

u64 tegra_rtc_read_ms(void);

/*
 * Callbacks for platform drivers to implement.
 */
extern void (*tegra_deep_sleep)(int);

unsigned int tegra_idle_power_down_last(unsigned int us, unsigned int flags);

#if defined(CONFIG_PM_SLEEP) && !defined(CONFIG_ARCH_TEGRA_2x_SOC)
void tegra_lp0_suspend_mc(void);
void tegra_lp0_resume_mc(void);
void tegra_lp0_cpu_mode(bool enter);
#else
static inline void tegra_lp0_suspend_mc(void) {}
static inline void tegra_lp0_resume_mc(void) {}
static inline void tegra_lp0_cpu_mode(bool enter) {}
#endif

#ifdef CONFIG_TEGRA_CLUSTER_CONTROL
#define INSTRUMENT_CLUSTER_SWITCH 0	/* Should be zero for shipping code */
#define DEBUG_CLUSTER_SWITCH 0		/* Should be zero for shipping code */
#define PARAMETERIZE_CLUSTER_SWITCH 1	/* Should be zero for shipping code */

static inline bool is_g_cluster_present(void)
{
	u32 fuse_sku = readl(FUSE_SKU_DIRECT_CONFIG);
	if (fuse_sku & FUSE_SKU_DISABLE_ALL_CPUS)
		return false;
	return true;
}
static inline unsigned int is_lp_cluster(void)
{
	unsigned int reg;
#ifdef CONFIG_ARCH_TEGRA_14x_SOC
	reg = readl(FLOW_CTRL_CLUSTER_CONTROL);
	return reg & 1; /* 0 == G, 1 == LP*/
#else
	asm("mrc	p15, 0, %0, c0, c0, 5\n"
	    "ubfx	%0, %0, #8, #4"
	    : "=r" (reg)
	    :
	    : "cc","memory");
	return reg ; /* 0 == G, 1 == LP*/
#endif
}
int tegra_cluster_control(unsigned int us, unsigned int flags);
void tegra_cluster_switch_prolog(unsigned int flags);
void tegra_cluster_switch_epilog(unsigned int flags);
int tegra_switch_to_g_cluster(void);
int tegra_switch_to_lp_cluster(void);
int tegra_cluster_switch(struct clk *cpu_clk, struct clk *new_cluster_clk);
#else
#define INSTRUMENT_CLUSTER_SWITCH 0	/* Must be zero for ARCH_TEGRA_2x_SOC */
#define DEBUG_CLUSTER_SWITCH 0		/* Must be zero for ARCH_TEGRA_2x_SOC */
#define PARAMETERIZE_CLUSTER_SWITCH 0	/* Must be zero for ARCH_TEGRA_2x_SOC */

static inline bool is_g_cluster_present(void)   { return true; }
static inline unsigned int is_lp_cluster(void)  { return 0; }
static inline int tegra_cluster_control(unsigned int us, unsigned int flags)
{
	return -EPERM;
}
static inline void tegra_cluster_switch_prolog(unsigned int flags) {}
static inline void tegra_cluster_switch_epilog(unsigned int flags) {}
static inline int tegra_switch_to_g_cluster(void)
{
	return -EPERM;
}
static inline int tegra_switch_to_lp_cluster(void)
{
	return -EPERM;
}
static inline int tegra_cluster_switch(struct clk *cpu_clk,
				       struct clk *new_cluster_clk)
{
	return -EPERM;
}
#endif

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
void tegra2_lp0_suspend_init(void);
void tegra2_lp2_set_trigger(unsigned long cycles);
unsigned long tegra2_lp2_timer_remain(void);
#else
void tegra3_lp2_set_trigger(unsigned long cycles);
unsigned long tegra3_lp2_timer_remain(void);
int tegra3_is_cpu_wake_timer_ready(unsigned int cpu);
void tegra3_lp2_timer_cancel_secondary(void);
#endif

static inline void tegra_lp0_suspend_init(void)
{
#ifdef CONFIG_ARCH_TEGRA_2x_SOC
	tegra2_lp0_suspend_init();
#endif
}

static inline void tegra_pd_set_trigger(unsigned long cycles)
{
#ifdef CONFIG_ARCH_TEGRA_2x_SOC
	tegra2_lp2_set_trigger(cycles);
#else
	tegra3_lp2_set_trigger(cycles);
#endif
}

static inline unsigned long tegra_pd_timer_remain(void)
{
#ifdef CONFIG_ARCH_TEGRA_2x_SOC
	return tegra2_lp2_timer_remain();
#else
	return tegra3_lp2_timer_remain();
#endif
}

static inline int tegra_is_cpu_wake_timer_ready(unsigned int cpu)
{
#if defined(CONFIG_TEGRA_LP2_CPU_TIMER) || defined(CONFIG_ARCH_TEGRA_2x_SOC)
	return 1;
#else
	return tegra3_is_cpu_wake_timer_ready(cpu);
#endif
}

static inline void tegra_pd_timer_cancel_secondary(void)
{
#ifndef CONFIG_ARCH_TEGRA_2x_SOC
	tegra3_lp2_timer_cancel_secondary();
#endif
}

#if DEBUG_CLUSTER_SWITCH && 0 /* !!!FIXME!!! THIS IS BROKEN */
extern unsigned int tegra_cluster_debug;
#define DEBUG_CLUSTER(x) do { if (tegra_cluster_debug) printk x; } while (0)
#else
#define DEBUG_CLUSTER(x) do { } while (0)
#endif
#if PARAMETERIZE_CLUSTER_SWITCH
void tegra_cluster_switch_set_parameters(unsigned int us, unsigned int flags);
#else
static inline void tegra_cluster_switch_set_parameters(
	unsigned int us, unsigned int flags)
{ }
#endif

#ifdef CONFIG_SMP
extern bool tegra_all_cpus_booted __read_mostly;
#else
#define tegra_all_cpus_booted (true)
#endif

#if !defined(CONFIG_ARCH_TEGRA_2x_SOC) && !defined(CONFIG_ARCH_TEGRA_3x_SOC) \
	&& defined(CONFIG_SMP)
void tegra_smp_clear_power_mask(void);
#else
static inline void tegra_smp_clear_power_mask(void){}
#endif

#if defined(CONFIG_ARCH_TEGRA_14x_SOC)
void tegra_smp_save_power_mask(void);
void tegra_smp_restore_power_mask(void);
#endif
#if defined(CONFIG_TEGRA_WDT_CLEAR_EARLY)
void pm_i2c_init(void);
void pm_i2c_deinit(void);
void pm_i2c_wait(void);
u8 pm_i2c_read_byte(u8 slave_addr, u8 offset);
void pm_i2c_write_byte(u8 slave_addr, u8 offset, u8 value);
bool is_wdt_int(void);
#endif

#ifdef CONFIG_TEGRA_USE_SECURE_KERNEL
void tegra_generic_smc(u32 type, u32 subtype, u32 arg);
#endif

/* The debug channel uart base physical address */
extern unsigned long  debug_uart_port_base;

extern struct clk *debug_uart_clk;
void tegra_console_uart_suspend(void);
void tegra_console_uart_resume(void);

extern bool tegra_is_dpd_mode;

#endif /* _MACH_TEGRA_PM_H_ */