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

#ifndef _BOARD_MX6Q_JUPITER_H
#define _BOARD_MX6Q_JUPITER_H
#include <mach/iomux-mx6q.h>

static iomux_v3_cfg_t mx6q_jupiter_pads[] = {
	/* UART4 for debug */
	MX6Q_PAD_KEY_COL0__UART4_TXD,
	MX6Q_PAD_KEY_ROW0__UART4_RXD,
	/* SD1 4 bit */
	MX6Q_PAD_SD1_CLK__USDHC1_CLK,
	MX6Q_PAD_SD1_CMD__USDHC1_CMD,
	MX6Q_PAD_SD1_DAT0__USDHC1_DAT0,
	MX6Q_PAD_SD1_DAT1__USDHC1_DAT1,
	MX6Q_PAD_SD1_DAT2__USDHC1_DAT2,
	MX6Q_PAD_SD1_DAT3__USDHC1_DAT3,
	MX6Q_PAD_KEY_COL1__USDHC1_VSELECT,
	/* Ethernet RMII mode */
	MX6Q_PAD_ENET_MDC__ENET_MDC,
	MX6Q_PAD_ENET_MDIO__ENET_MDIO,
	MX6Q_PAD_ENET_RX_ER__ENET_RX_ER,
	MX6Q_PAD_ENET_TX_EN__ENET_TX_EN,
	MX6Q_PAD_ENET_RXD0__ENET_RDATA_0,
	MX6Q_PAD_ENET_RXD1__ENET_RDATA_1,
	MX6Q_PAD_ENET_TXD0__ENET_TDATA_0,
	MX6Q_PAD_ENET_TXD1__ENET_TDATA_1,
	MX6Q_PAD_ENET_CRS_DV__ENET_RX_EN,
	MX6Q_PAD_GPIO_16__ENET_ANATOP_ETHERNET_REF_OUT,
	/* I2C2 */
	MX6Q_PAD_KEY_COL3__I2C2_SCL,
	MX6Q_PAD_KEY_ROW3__I2C2_SDA,
	/* I2C3 */
	MX6Q_PAD_GPIO_6__I2C3_SDA,
	MX6Q_PAD_GPIO_3__I2C3_SCL,
};

#endif
