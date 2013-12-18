/*
 * Copyright (C) 2013 High Technology Devices LLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/fsl_devices.h>
#include <linux/platform_device.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/anatop-regulator.h>
#include <linux/phy.h>
#include <linux/fec.h>
#include <linux/gpio.h>

#include <mach/ipu-v3.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/iomux-mx6q.h>
#include <mach/iomux-v3.h>
#include <mach/mxc_dvfs.h>

#include "crm_regs.h"
#include "devices-imx6q.h"
#include "usb.h"
#include "board-mx6q_jupiter.h"

#define MX6Q_JUPITER_SD1_WP 	IMX_GPIO_NR(4, 9)
#define MX6Q_JUPITER_SD1_CD 	IMX_GPIO_NR(4, 10)
#define MX6Q_JUPITER_PMIC_INT 	IMX_GPIO_NR(7, 13)

extern char *gp_reg_id;
extern char *soc_reg_id;
extern int __init mx6q_jupiter_init_pfuze100(u32 int_gpio);

static const struct esdhc_platform_data mx6q_jupiter_sd1_data __initconst = {
   .cd_gpio = MX6Q_JUPITER_SD1_CD,
   .wp_gpio = MX6Q_JUPITER_SD1_WP,
   .keep_power_at_suspend = 1,
   .support_18v = 1,
   .support_8bit = 0,
   .delay_line = 0,
   .runtime_pm = 1,
   .cd_type = ESDHC_CD_CONTROLLER,
};

static int mx6q_jupiter_fec_phy_init(struct phy_device *phydev) {
   /* todo review if needed to renable phy settings */
   return 0;
}

static struct fec_platform_data mx6q_jupiter_fec_data __initdata = {
   .init = mx6q_jupiter_fec_phy_init,
   .phy = PHY_INTERFACE_MODE_RMII,
};

static struct viv_gpu_platform_data imx6q_gpu_pdata __initdata = {
	.reserved_mem_size = SZ_128M + SZ_64M - SZ_16M,
};

static struct imx_ipuv3_platform_data ipu_data[] = {
   {
       .rev = 4,
       .csi_clk[0] = "clko_clk",
       .bypass_reset = false,
   },
};

static struct ipuv3_fb_platform_data mx6q_jupiter_fb_data[] = {
   {
       .disp_dev = "ldb",
       .interface_pix_fmt = IPU_PIX_FMT_RGB666,
       .mode_str = "LDB-XGA",
       .default_bpp = 16,
       .int_clk = false,
   },
};

static struct fsl_mxc_ldb_platform_data mx6q_jupiter_ldb_data = {
   .ipu_id = 0,
   .disp_id = 0,
   .ext_ref = 1,
   .mode = LDB_SEP0,
};

static struct mxc_dvfs_platform_data mx6q_jupiter_dvfscore_data = {
    .reg_id = "VDDCORE",
    .soc_id = "VDDSOC",
    .clk1_id = "cpu_clk",
    .clk2_id = "gpc_dvfs_clk",
    .gpc_cntr_offset = MXC_GPC_CNTR_OFFSET,
    .ccm_cdcr_offset = MXC_CCM_CDCR_OFFSET,
    .ccm_cacrr_offset = MXC_CCM_CACRR_OFFSET,
    .ccm_cdhipr_offset = MXC_CCM_CDHIPR_OFFSET,
    .prediv_mask = 0x1F800,
    .prediv_offset = 11,
    .prediv_val = 3,
    .div3ck_mask = 0xE0000000,
    .div3ck_offset = 29,
    .div3ck_val = 2,
    .emac_val = 0x08,
    .upthr_val = 25,
    .dnthr_val = 9,
    .pncthr_val = 33,
    .upcnt_val = 10,
    .dncnt_val = 10,
    .delay_time = 80,
};

static inline void mx6q_jupiter_init_uart(void)
{
	imx6q_add_imx_uart(3, NULL);
}

static void __init mx6_board_init(void)
{
	mxc_iomux_v3_setup_multiple_pads(mx6q_jupiter_pads,
			ARRAY_SIZE(mx6q_jupiter_pads));

	gp_reg_id =  mx6q_jupiter_dvfscore_data.reg_id;
	soc_reg_id = mx6q_jupiter_dvfscore_data.soc_id;

	mx6q_jupiter_init_uart();

	imx6q_add_ipuv3(0, &ipu_data[0]);
	imx6q_add_ipuv3fb(0, &mx6q_jupiter_fb_data[0]);
    imx6q_add_vdoa();
    imx6q_add_ldb(&mx6q_jupiter_ldb_data);
    imx6q_add_v4l2_output(0);
    /* reuest pmic interrupt gpio */
    gpio_request(MX6Q_JUPITER_PMIC_INT, "pfuze-int");
    gpio_direction_input(MX6Q_JUPITER_PMIC_INT);
    /* enable pfuze regulators */
    mx6q_jupiter_init_pfuze100(MX6Q_JUPITER_PMIC_INT);

    imx6q_add_dvfs_core(&mx6q_jupiter_dvfscore_data);
	/* SD1 */
	imx6q_add_sdhci_usdhc_imx(0, &mx6q_jupiter_sd1_data);
	/* ethernet phy */
	imx6_init_fec(mx6q_jupiter_fec_data);
}

extern void __iomem *twd_base;
static void __init mx6_timer_init(void)
{
	struct clk *uart_clk;
#ifdef CONFIG_LOCAL_TIMERS
	twd_base = ioremap(LOCAL_TWD_ADDR, SZ_256);
	BUG_ON(!twd_base);
#endif
	mx6_clocks_init(32768, 24000000, 0, 0);

	uart_clk = clk_get_sys("imx-uart.0", NULL);
	early_console_setup(UART4_BASE_ADDR, uart_clk);
}

static struct sys_timer mx6_timer = {
	.init = mx6_timer_init,
};

static void __init mx6q_reserve(void)
{
#if defined(CONFIG_MXC_GPU_VIV) || defined(CONFIG_MXC_GPU_VIV_MODULE)
	phys_addr_t phys;

	if (imx6q_gpu_pdata.reserved_mem_size) {
		phys = memblock_alloc_base(imx6q_gpu_pdata.reserved_mem_size,
				SZ_4K, SZ_1G);
		memblock_remove(phys, imx6q_gpu_pdata.reserved_mem_size);
		imx6q_gpu_pdata.reserved_mem_base = phys;
	}
#endif
}

MACHINE_START(MX6Q_JUPITER, "Freescale i.MX6Q Jupiter Board")
	.boot_params = MX6_PHYS_OFFSET + 0x100,
	.map_io = mx6_map_io,
	.init_irq = mx6_init_irq,
	.init_machine = mx6_board_init,
	.timer = &mx6_timer,
	.reserve = mx6q_reserve,
MACHINE_END
