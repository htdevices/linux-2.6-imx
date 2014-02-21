/*
 * Copyright (C) 2013-2014 High Technology Devices LLC.
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

#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include <linux/types.h>
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
#include <linux/pwm_backlight.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/anatop-regulator.h>
#include <linux/phy.h>
#include <linux/fec.h>
#if defined(CONFIG_ION)
#include <linux/ion.h>
#endif
#include <linux/gpio.h>

#include <mach/ipu-v3.h>
#include <mach/viv_gpu.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/iomux-mx6q.h>
#include <mach/iomux-v3.h>
#include <mach/mxc_dvfs.h>

#include "crm_regs.h"
#include "devices-imx6q.h"
#include "usb.h"
#include "board-mx6q_jupiter.h"

#define MX6Q_JUPITER_USB_OTG_ON IMX_GPIO_NR(3, 16)
#define MX6Q_JUPITER_USB_OTG_OC IMX_GPIO_NR(3, 17)
#define MX6Q_JUPITER_GPS_ON 	IMX_GPIO_NR(3, 29)
#define MX6Q_JUPITER_SD1_WP 	IMX_GPIO_NR(4, 9)
#define MX6Q_JUPITER_SD1_CD 	IMX_GPIO_NR(4, 11)
#define MX6Q_JUPITER_DISP_EN	IMX_GPIO_NR(4, 14)
#define MX6Q_JUPITER_BKLT_EN	IMX_GPIO_NR(4, 15)
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
   .cd_type = ESDHC_CD_CONTROLLER,
};

static const struct esdhc_platform_data mx6q_jupiter_sd4_data __initconst = {
	.always_present = 1,
	.keep_power_at_suspend = 1,
	.support_18v = 1,
	.support_8bit = 1,
	.delay_line = 0,
	.cd_type = ESDHC_CD_PERMANENT,
};

static struct regulator_consumer_supply mx6q_jupiter_vmmc_consumers[] = {
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.0"),
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.3"),
};

static struct regulator_init_data mx6q_jupiter_vmmc_init = {
	.num_consumer_supplies = ARRAY_SIZE(mx6q_jupiter_vmmc_consumers),
	.consumer_supplies = mx6q_jupiter_vmmc_consumers,
};

static struct fixed_voltage_config mx6q_jupiter_vmmc_reg_config = {
	.supply_name = "vmmc",
	.microvolts = 3300000,
	.gpio = -1,
	.init_data = &mx6q_jupiter_vmmc_init,
};

static struct platform_device mx6q_jupiter_vmmc_reg_devices = {
	.name = "reg-fixed-voltage",
	.id = 3,
	.dev = {
		.platform_data = &mx6q_jupiter_vmmc_reg_config,
	},
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
	.reserved_mem_size = SZ_128M + SZ_64M,
};

#if defined(CONFIG_ION)
static struct ion_platform_data imx_ion_data = {
   .nr = 1,
   .heaps = {
       {
       .type = ION_HEAP_TYPE_CARVEOUT,
       .name = "vpu_ion",
       .size = SZ_64M,
       },
   },
};
#endif

static struct imx_ipuv3_platform_data ipu_data[] = {
   {
       .rev = 4,
       .csi_clk[0] = "clko_clk",
       .bypass_reset = false,
   }, {
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

/* Backlight PWM for Orient Display */
static struct platform_pwm_backlight_data mx6q_jupiter_pwm_backight_data = {
	.pwm_id = 0,
	.max_brightness = 255,
	.dft_brightness = 128,
	.pwm_period_ns = 1000,
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

static const struct anatop_thermal_platform_data
   mx6q_jupiter_anatop_thermal_data __initconst = {
       .name = "anatop_thermal",
};

static void mx6q_jupiter_suspend_enter(void)
{
   /* enter suspend */
}

static void mx6q_jupiter_suspend_exit(void)
{
   /* exit suspend */
}

static const struct pm_platform_data mx6q_jupiter_pm_data __initconst = {
   .name = "imx_pm",
   .suspend_enter = mx6q_jupiter_suspend_enter,
   .suspend_exit  = mx6q_jupiter_suspend_exit,
};

static inline void mx6q_jupiter_init_uart(void)
{
	imx6q_add_imx_uart(0, NULL);
	imx6q_add_imx_uart(2, NULL);
}

static void mx6q_jupiter_usbotg_vbus(bool on)
{
	if (on)
		gpio_set_value(MX6Q_JUPITER_USB_OTG_ON, 1);
	else
		gpio_set_value(MX6Q_JUPITER_USB_OTG_ON, 0);
}

static void __init mx6q_jupiter_init_usb(void)
{
	int ret = 0;
	imx_otg_base = MX6_IO_ADDRESS(MX6Q_USB_OTG_BASE_ADDR);

	ret = gpio_request(MX6Q_JUPITER_USB_OTG_ON, "otg-on");
	if (ret) {
		printk(KERN_ERR "failed to get GPIO MX6Q_JUPITER_USB_OTG_ON"
			" %d\n", ret);
		return;
	}
	gpio_direction_output(MX6Q_JUPITER_USB_OTG_ON, 0);

	ret = gpio_request(MX6Q_JUPITER_USB_OTG_OC, "otg-oc");
	if (ret) {
		printk(KERN_ERR "failed to get GPIO MX6Q_JUPITER_USB_OTG_OC:"
			" %d\n", ret);
		return;
	}
	gpio_direction_input(MX6Q_JUPITER_USB_OTG_OC);

	mxc_iomux_set_gpr_register(1, 13, 1, 0);
	mx6_set_otghost_vbus_func(mx6q_jupiter_usbotg_vbus);
}

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct resource ram_console_resource = {
   .name = "android ram console",
   .flags = IORESOURCE_MEM,
};

static struct platform_device android_ram_console = {
   .name = "ram_console",
   .num_resources = 1,
   .resource = &ram_console_resource,
};

static int __init imx6x_add_ram_console(void)
{
   return platform_device_register(&android_ram_console);
}
#endif

static void __init mx6_board_init(void)
{
	mxc_iomux_v3_setup_multiple_pads(mx6q_jupiter_pads,
			ARRAY_SIZE(mx6q_jupiter_pads));

	gp_reg_id =  mx6q_jupiter_dvfscore_data.reg_id;
	soc_reg_id = mx6q_jupiter_dvfscore_data.soc_id;

	mx6q_jupiter_init_uart();

#ifdef CONFIG_ANDROID_RAM_CONSOLE
    imx6x_add_ram_console();
#endif

	imx6q_add_ipuv3(0, &ipu_data[0]);
	imx6q_add_ipuv3(1, &ipu_data[1]);
	imx6q_add_ipuv3fb(0, &mx6q_jupiter_fb_data[0]);
    imx6q_add_vdoa();
    imx6q_add_ldb(&mx6q_jupiter_ldb_data);
    imx6q_add_v4l2_output(0);
    /* reuest pmic interrupt gpio */
    gpio_request(MX6Q_JUPITER_PMIC_INT, "pfuze-int");
    gpio_direction_input(MX6Q_JUPITER_PMIC_INT);
    /* enable pfuze regulators */
    mx6q_jupiter_init_pfuze100(MX6Q_JUPITER_PMIC_INT);

	gpio_request(MX6Q_JUPITER_DISP_EN, "disp-en0");
	gpio_direction_output(MX6Q_JUPITER_DISP_EN, 1);
	gpio_export(MX6Q_JUPITER_DISP_EN, 0);
	gpio_request(MX6Q_JUPITER_BKLT_EN, "bklt-en0");
	gpio_direction_output(MX6Q_JUPITER_BKLT_EN, 1);
	gpio_export(MX6Q_JUPITER_BKLT_EN, 0);
	imx6q_add_mxc_pwm(0);
	imx6q_add_mxc_pwm_backlight(0, &mx6q_jupiter_pwm_backight_data);

    imx6q_add_vpu();
    imx6q_add_otp();
    imx6q_add_imx2_wdt(0, NULL);
    imx6q_add_dma();

#if defined(CONFIG_ION)
    if (imx_ion_data.heaps[0].size) {
        imx6q_add_ion(0, &imx_ion_data,
            sizeof(imx_ion_data) +
            sizeof(struct ion_platform_heap));
    }
#endif

    imx6q_add_dvfs_core(&mx6q_jupiter_dvfscore_data);
    imx6q_add_anatop_thermal_imx(1, &mx6q_jupiter_anatop_thermal_data);
    imx6q_add_pm_imx(0, &mx6q_jupiter_pm_data);
    imx_add_viv_gpu(&imx6_gpu_data, &imx6q_gpu_pdata);
	/* SD, eMMC as mmcblk0 */
	imx6q_add_sdhci_usdhc_imx(3, &mx6q_jupiter_sd4_data);
	imx6q_add_sdhci_usdhc_imx(0, &mx6q_jupiter_sd1_data);
	platform_device_register(&mx6q_jupiter_vmmc_reg_devices);
	mx6q_jupiter_init_usb();
	/* ethernet phy */
	imx6_init_fec(mx6q_jupiter_fec_data);

    imx6q_add_busfreq();

    imx6q_add_perfmon(0);
    imx6q_add_perfmon(1);
    imx6q_add_perfmon(2);
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
	early_console_setup(UART1_BASE_ADDR, uart_clk);
}

static struct sys_timer mx6_timer = {
	.init = mx6_timer_init,
};

static void __init mx6_board_fixup(struct machine_desc *desc, struct tag *tags,
		char **cmdline, struct meminfo *mi)
{
	char *str;
	struct tag *t;
	int i = 0;
	struct ipuv3_fb_platform_data *pdata_fb = mx6q_jupiter_fb_data;

	for_each_tag(t, tags) {
	if (t->hdr.tag == ATAG_CMDLINE) {
		str = t->u.cmdline.cmdline;
		str = strstr(str, "fbmem=");
		if (str != NULL) {
			str += 6;
			pdata_fb[i++].res_size[0] = memparse(str, &str);
			while (*str == ',' && i < ARRAY_SIZE(mx6q_jupiter_fb_data)) {
				str++;
				mx6q_jupiter_fb_data[i++].res_size[0] =
					memparse(str, &str);
			}
		}
#if defined(CONFIG_ION)
		/* ion reserve memory */
		str = t->u.cmdline.cmdline;
		str = strstr(str, "iomem=");
		if (str != NULL) {
			str += 7;
			imx_ion_data.heaps[0].size = memparse(str, &str);
		}
#endif
		/* Primary frabuffer address */
		str = t->u.cmdline.cmdline;
		str = strstr(str, "fb0base=");
		if (str != NULL) {
			str += 8;
			pdata_fb[0].res_base[0] =
				simple_strtol(str, &str, 16);
		}
		/* GPU reserve memory */
		str = t->u.cmdline.cmdline;
		str = strstr(str, "gpumem=");
		if (str != NULL) {
			str += 7;
			imx6q_gpu_pdata.reserved_mem_size = memparse(str, &str);
		}
		break;
	}
	}
}

static void __init mx6q_reserve(void)
{
	phys_addr_t phys;

#ifdef CONFIG_ANDROID_RAM_CONSOLE
    phys = memblock_alloc_base(SZ_128K, SZ_4K, SZ_1G);
    memblock_remove(phys, SZ_128K);
    memblock_free(phys, SZ_128K);
    ram_console_resource.start = phys;
    ram_console_resource.end   = phys + SZ_128K - 1;
#endif

#if defined(CONFIG_MXC_GPU_VIV) || defined(CONFIG_MXC_GPU_VIV_MODULE)

	if (imx6q_gpu_pdata.reserved_mem_size) {
		phys = memblock_alloc_base(imx6q_gpu_pdata.reserved_mem_size,
				SZ_4K, SZ_1G);
		memblock_remove(phys, imx6q_gpu_pdata.reserved_mem_size);
		imx6q_gpu_pdata.reserved_mem_base = phys;
	}
#endif

#if defined(CONFIG_ION)
    if (imx_ion_data.heaps[0].size) {
        phys = memblock_alloc(imx_ion_data.heaps[0].size, SZ_4K);
        memblock_free(phys, imx_ion_data.heaps[0].size);
        memblock_remove(phys, imx_ion_data.heaps[0].size);
        imx_ion_data.heaps[0].base = phys;
    }
#endif
}

MACHINE_START(MX6Q_JUPITER, "Freescale i.MX6Q Jupiter Board")
	.boot_params = MX6_PHYS_OFFSET + 0x100,
	.fixup = mx6_board_fixup,
	.map_io = mx6_map_io,
	.init_irq = mx6_init_irq,
	.init_machine = mx6_board_init,
	.timer = &mx6_timer,
	.reserve = mx6q_reserve,
MACHINE_END
