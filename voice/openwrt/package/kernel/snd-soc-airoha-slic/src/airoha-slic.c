/*
 * airoha-slic.c -- ASoC machine driver: AN7581 AFE <-> Le9643 SLIC PCM
 *
 * The Le9643 in PCM/SPI mode presents a plain synchronous PCM audio
 * stream (A-law/u-law/linear, 8 kHz narrowband).  The codec side is
 * format-only: all line control (hook, ring, DTMF) lives in the
 * slic-le9643 SPI driver and is exposed via sysfs; this machine driver
 * only wires the CPU DAI (airoha AFE, see airoha-afe-pcm) to a fixed
 * format codec DAI so that ALSA PCM devices appear for baresip.
 *
 * Copyright (c) 2026 ZN504XG-D voice porting project
 * GPL-2.0
 */

#include <linux/module.h>
#include <linux/of_platform.h>
#include <sound/soc.h>

static struct snd_soc_dai_link airoha_slic_dai_link = {
	.name = "AFE-PCM-SLIC",
	.stream_name = "SLIC PCM",
	.codec_dai_name = "snd-soc-dummy-dai", /* format-only; real codec
						  * ops come with VP-API glue */
	.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS,
};

static struct snd_soc_card airoha_slic_card = {
	.name = "airoha-slic",
	.owner = THIS_MODULE,
	.dai_link = &airoha_slic_dai_link,
	.num_links = 1,
};

static int airoha_slic_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &airoha_slic_card;
	struct device_node *afe_node;

	afe_node = of_parse_phandle(pdev->dev.of_node, "audio-cpu", 0);
	if (!afe_node)
		return -ENODEV;

	airoha_slic_dai_link.cpu_of_node = afe_node;
	airoha_slic_dai_link.platform_of_node = afe_node;
	airoha_slic_dai_link.codec_of_node =
		of_parse_phandle(pdev->dev.of_node, "audio-codec", 0);
	if (!airoha_slic_dai_link.codec_of_node)
		airoha_slic_dai_link.codec_name = "snd-soc-dummy";

	card->dev = &pdev->dev;
	return snd_soc_register_card(card);
}

static int airoha_slic_remove(struct platform_device *pdev)
{
	snd_soc_unregister_card(&airoha_slic_card);
	return 0;
}

static const struct of_device_id airoha_slic_of_match[] = {
	{ .compatible = "airoha,an7581-slic-sound" },
	{ }
};
MODULE_DEVICE_TABLE(of, airoha_slic_of_match);

static struct platform_driver airoha_slic_driver = {
	.probe = airoha_slic_probe,
	.remove = airoha_slic_remove,
	.driver = {
		.name = "airoha-slic-snd",
		.of_match_table = airoha_slic_of_match,
	},
};
module_platform_driver(airoha_slic_driver);

MODULE_DESCRIPTION("AN7581 AFE to Le9643 SLIC ASoC machine driver");
MODULE_LICENSE("GPL v2");
