/*
 * slic-le9643.h -- Microsemi miSLIC Le9643 control driver (header)
 *
 * Copyright (c) 2026 ZN504XG-D voice porting project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _SLIC_LE9643_H
#define _SLIC_LE9643_H

#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>

struct slic_le9643 {
	struct spi_device	*spi;
	struct device		*dev;

	/* Control interface pins */
	struct gpio_desc	*reset_gpio;	/* RST, active low */
	struct gpio_desc	*int_gpio;	/* INT, active low (hook/DTMF events) */

	/* State */
	bool			chip_ready;
	bool			offhook;
	bool			ringing;

	struct mutex		lock;
};

/*
 * Low-level bus access.
 *
 * TODO: fill in the exact HBI/SPI transaction format once the VP-API-II
 * sources (vp_api.h, sys_service) or the Le9643 datasheet are available.
 * The transaction shape below follows the VoicePath "HBI" convention:
 * a 16-bit address/command word followed by 16-bit data words, but the
 * on-wire encoding (W/R bit position, byte order) MUST be verified
 * against real documentation.
 */
int slic_hbi_read(struct slic_le9643 *slic, u16 addr, u16 *val);
int slic_hbi_write(struct slic_le9643 *slic, u16 addr, u16 val);

/* Chip-level operations, implemented on top of VP-API-II later. */
int slic_le9643_init_chip(struct slic_le9643 *slic);
int slic_le9643_set_ring(struct slic_le9643 *slic, bool on);
int slic_le9643_set_hook_report(struct slic_le9643 *slic, bool enable);

#endif /* _SLIC_LE9643_H */
