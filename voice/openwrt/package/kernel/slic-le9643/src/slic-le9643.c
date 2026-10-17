/*
 * slic-le9643.c -- Microsemi miSLIC Le9643 control driver (skeleton)
 *
 * Control-plane driver for the Le9643 SLIC on ZN504XG-D:
 *   - SPI (HBI) register access
 *   - reset + INT GPIO handling (hook / DTMF events)
 *   - sysfs status interface (compatible with the ZTE-style
 *     /sys/class/slic/slic-0/{chip_model,chip_ready,hook_state} layout
 *     documented in amenekowo/mu5250_tweaking, so userspace tooling
 *     can be shared)
 *   - ring control stub
 *
 * The chip initialization (DC feed, AC impedance, PCM format, DTMF,
 * cadencing) is delegated to VP-API-II (VoicePath API-II) once the
 * sources are obtained; see slic_le9643_init_chip() below.
 *
 * Copyright (c) 2026 ZN504XG-D voice porting project
 * GPL-2.0
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/spi/spi.h>

#include "slic-le9643.h"

#define DRV_NAME		"slic-le9643"
#define CHIP_MODEL_STR		"LE9643"

/* ------------------------------------------------------------------ */
/* Low-level HBI/SPI access                                            */
/* ------------------------------------------------------------------ */

/*
 * TODO: verify against VP-API-II / datasheet. Placeholder encoding:
 * one 16-bit command/address word (W/R bit + 15-bit address) followed
 * by one 16-bit data word, both MSB-first.
 */
int slic_hbi_write(struct slic_le9643 *slic, u16 addr, u16 val)
{
	u8 tx[4];

	tx[0] = (addr >> 8) & 0x7f;	/* W/R=0 (write) | addr high */
	tx[1] = addr & 0xff;
	tx[2] = val >> 8;
	tx[3] = val & 0xff;

	return spi_write(slic->spi, tx, sizeof(tx));
}

int slic_hbi_read(struct slic_le9643 *slic, u16 addr, u16 *val)
{
	u8 tx[4] = { 0x80 | ((addr >> 8) & 0x7f), addr & 0xff, 0, 0 };
	u8 rx[4] = { 0 };
	int ret;
	struct spi_transfer t = {
		.tx_buf = tx,
		.rx_buf = rx,
		.len = sizeof(tx),
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	ret = spi_sync(slic->spi, &m);
	if (ret)
		return ret;

	*val = (rx[2] << 8) | rx[3];
	return 0;
}

/* ------------------------------------------------------------------ */
/* Chip operations (VP-API-II integration points)                      */
/* ------------------------------------------------------------------ */

/*
 * Full chip initialization: this is where the VP-API-II port goes
 * (VpInitDevice / VpInitLine + profile pointers). Without the API
 * sources we can only do a best-effort probe read.
 */
int slic_le9643_init_chip(struct slic_le9643 *slic)
{
	u16 id = 0;
	int ret;

	/* Hardware reset pulse */
	if (slic->reset_gpio) {
		gpiod_set_value_cansleep(slic->reset_gpio, 1);
		msleep(20);
		gpiod_set_value_cansleep(slic->reset_gpio, 0);
		msleep(50);
	}

	ret = slic_hbi_read(slic, 0x0000, &id); /* TODO: real ID register */
	if (ret) {
		dev_err(slic->dev, "no response from SLIC, check SPI wiring\n");
		return ret;
	}

	dev_info(slic->dev, "SLIC responded, id=0x%04x (format unverified)\n", id);
	slic->chip_ready = true;
	return 0;
}

int slic_le9643_set_ring(struct slic_le9643 *slic, bool on)
{
	mutex_lock(&slic->lock);
	slic->ringing = on;
	/* TODO: VpStartRing / cadencing via VP-API-II */
	mutex_unlock(&slic->lock);
	return 0;
}

int slic_le9643_set_hook_report(struct slic_le9643 *slic, bool enable)
{
	/* TODO: line event mask via VP-API-II */
	return 0;
}

/* ------------------------------------------------------------------ */
/* INT pin: hook / DTMF events                                         */
/* ------------------------------------------------------------------ */

static irqreturn_t slic_le9643_irq(int irq, void *data)
{
	struct slic_le9643 *slic = data;

	/* TODO: read event status from SLIC, update offhook, report DTMF */
	slic->offhook = !slic->offhook;
	kobject_uevent(&slic->dev->kobj, KOBJ_CHANGE);
	return IRQ_HANDLED;
}

/* ------------------------------------------------------------------ */
/* Sysfs (ZTE slic class layout for userspace reuse)                   */
/* ------------------------------------------------------------------ */

static ssize_t chip_model_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%s\n", CHIP_MODEL_STR);
}
static DEVICE_ATTR_RO(chip_model);

static ssize_t chip_ready_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct slic_le9643 *slic = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", slic->chip_ready ? 1 : 0);
}
static DEVICE_ATTR_RO(chip_ready);

static ssize_t hook_state_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct slic_le9643 *slic = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%s\n", slic->offhook ? "off-hook" : "on-hook");
}
static DEVICE_ATTR_RO(hook_state);

static ssize_t ring_show(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct slic_le9643 *slic = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", slic->ringing ? 1 : 0);
}

static ssize_t ring_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct slic_le9643 *slic = dev_get_drvdata(dev);
	bool on;
	int ret = kstrtobool(buf, &on);

	if (ret)
		return ret;
	slic_le9643_set_ring(slic, on);
	return count;
}
static DEVICE_ATTR_RW(ring);

static struct attribute *slic_le9643_attrs[] = {
	&dev_attr_chip_model.attr,
	&dev_attr_chip_ready.attr,
	&dev_attr_hook_state.attr,
	&dev_attr_ring.attr,
	NULL,
};
ATTRIBUTE_GROUPS(slic_le9643);

/* ------------------------------------------------------------------ */
/* SPI driver                                                          */
/* ------------------------------------------------------------------ */

static int slic_le9643_probe(struct spi_device *spi)
{
	struct slic_le9643 *slic;
	int ret;

	slic = devm_kzalloc(&spi->dev, sizeof(*slic), GFP_KERNEL);
	if (!slic)
		return -ENOMEM;

	slic->spi = spi;
	slic->dev = &spi->dev;
	mutex_init(&slic->lock);

	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
	spi->max_speed_hz = 4000000; /* TODO: per datasheet */
	ret = spi_setup(spi);
	if (ret)
		return ret;

	slic->reset_gpio = devm_gpiod_get_optional(&spi->dev, "reset",
						   GPIOD_OUT_LOW);
	if (IS_ERR(slic->reset_gpio))
		return PTR_ERR(slic->reset_gpio);

	slic->int_gpio = devm_gpiod_get_optional(&spi->dev, "int", GPIOD_IN);
	if (IS_ERR(slic->int_gpio))
		return PTR_ERR(slic->int_gpio);

	spi_set_drvdata(spi, slic);

	if (slic->int_gpio) {
		ret = devm_request_irq(&spi->dev, gpiod_to_irq(slic->int_gpio),
				       slic_le9643_irq,
				       IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				       dev_name(&spi->dev), slic);
		if (ret)
			dev_warn(&spi->dev, "no IRQ, polling fallback TODO\n");
	}

	ret = slic_le9643_init_chip(slic);
	if (ret)
		return ret; /* -EPROBE_DEFER not useful: wiring is static */

	return 0;
}

static void slic_le9643_remove(struct spi_device *spi)
{
	struct slic_le9643 *slic = spi_get_drvdata(spi);

	slic_le9643_set_ring(slic, false);
}

static const struct of_device_id slic_le9643_of_match[] = {
	{ .compatible = "microsemi,le9643" },
	{ .compatible = "lantiq,le9643" }, /* ZTE firmware labels it so */
	{ }
};
MODULE_DEVICE_TABLE(of, slic_le9643_of_match);

static struct spi_driver slic_le9643_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = slic_le9643_of_match,
		.dev_groups = slic_le9643_groups,
	},
	.probe = slic_le9643_probe,
	.remove = slic_le9643_remove,
};
module_spi_driver(slic_le9643_driver);

MODULE_DESCRIPTION("Microsemi miSLIC Le9643 control driver (skeleton)");
MODULE_LICENSE("GPL v2");
