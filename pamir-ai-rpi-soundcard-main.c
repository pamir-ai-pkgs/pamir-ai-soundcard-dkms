// SPDX-License-Identifier: GPL-2.0
/*
 * Pamir AI RPI soundcard driver - standalone DAI link implementation
 *
 * Based on rpi-simple-soundcard.c
 * Copyright (C) 2018 Raspberry Pi.
 * Copyright (C) 2025 Pamir AI Incorporated.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

/* Parameters for Pamir AI RPI soundcard */
struct snd_pamir_ai_simple_drvdata {
	struct snd_soc_dai_link *dai;
	const char* card_name;
	unsigned int fixed_bclk_ratio;
};

static struct snd_soc_card snd_pamir_ai_simple = {
	.driver_name  = "PamirAI-simple",
	.owner        = THIS_MODULE,
	.dai_link     = NULL,
	.num_links    = 1, /* Only a single DAI supported at the moment */
};

static int snd_pamir_ai_simple_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_pamir_ai_simple_drvdata *drvdata =
		snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);

	if (drvdata->fixed_bclk_ratio > 0)
		return snd_soc_dai_set_bclk_ratio(cpu_dai,
				drvdata->fixed_bclk_ratio);

	return 0;
}

static int snd_pamir_ai_simple_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);
	struct snd_pamir_ai_simple_drvdata *drvdata;
	unsigned int sample_bits;

	drvdata = snd_soc_card_get_drvdata(rtd->card);

	if (drvdata->fixed_bclk_ratio > 0)
		return 0; // BCLK is configured in .init

	/* The simple drivers just set the bclk_ratio to sample_bits * 2 so
	 * hard-code this for now, but sticking to powers of 2 to allow for
	 * integer clock divisors. More complex drivers could just replace
	 * the hw_params routine.
	 */
	sample_bits = snd_pcm_format_width(params_format(params));
	sample_bits = sample_bits <= 16 ? 16 : 32;

	return snd_soc_dai_set_bclk_ratio(cpu_dai, sample_bits * 2);
}

static struct snd_soc_ops snd_pamir_ai_simple_ops = {
	.hw_params = snd_pamir_ai_simple_hw_params,
};

SND_SOC_DAILINK_DEFS(pamir_ai,
	DAILINK_COMP_ARRAY(COMP_EMPTY()),
	DAILINK_COMP_ARRAY(COMP_CODEC("pamir-ai-soundcard", "pamir-ai-hifi")),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

static struct snd_soc_dai_link snd_pamir_ai_soundcard_dai[] = {
{
	.name           = "Pamir AI SoundCard",
	.stream_name    = "Pamir AI SoundCard HiFi",
	.dai_fmt        =  SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
				SND_SOC_DAIFMT_CBS_CFS,
	SND_SOC_DAILINK_REG(pamir_ai),
},
};

static struct snd_pamir_ai_simple_drvdata drvdata_pamir_ai = {
	.card_name = "snd_pamir_ai_soundcard",
	.dai       = snd_pamir_ai_soundcard_dai,
};

static const struct of_device_id snd_pamir_ai_simple_of_match[] = {
	{ .compatible = "pamir-ai,soundcard",
		.data = (void *) &drvdata_pamir_ai },
	{},
};

static int snd_pamir_ai_simple_probe(struct platform_device *pdev)
{
	int ret = 0;
	const struct of_device_id *of_id;

	snd_pamir_ai_simple.dev = &pdev->dev;
	of_id = of_match_node(snd_pamir_ai_simple_of_match, pdev->dev.of_node);

	if (pdev->dev.of_node && of_id->data) {
		struct device_node *i2s_node;
		struct snd_pamir_ai_simple_drvdata *drvdata =
			(struct snd_pamir_ai_simple_drvdata *) of_id->data;
		struct snd_soc_dai_link *dai = drvdata->dai;

		snd_soc_card_set_drvdata(&snd_pamir_ai_simple, drvdata);

		/* More complex drivers might override individual functions */
		if (!dai->init)
			dai->init = snd_pamir_ai_simple_init;
		if (!dai->ops)
			dai->ops = &snd_pamir_ai_simple_ops;

		snd_pamir_ai_simple.name = drvdata->card_name;

		snd_pamir_ai_simple.dai_link = dai;
		i2s_node = of_parse_phandle(pdev->dev.of_node,
				"i2s-controller", 0);
		if (!i2s_node) {
			pr_err("Failed to find i2s-controller DT node\n");
			return -ENODEV;
		}

		dai->cpus->of_node = i2s_node;
		dai->platforms->of_node = i2s_node;
	}

	ret = devm_snd_soc_register_card(&pdev->dev, &snd_pamir_ai_simple);
	if (ret && ret != -EPROBE_DEFER)
		dev_err(&pdev->dev, "Failed to register card %d\n", ret);

	return ret;
}

static struct platform_driver snd_pamir_ai_simple_driver = {
	.driver = {
		.name   = "snd-pamir-ai-simple",
		.owner  = THIS_MODULE,
		.of_match_table = snd_pamir_ai_simple_of_match,
	},
	.probe          = snd_pamir_ai_simple_probe,
};
MODULE_DEVICE_TABLE(of, snd_pamir_ai_simple_of_match);

module_platform_driver(snd_pamir_ai_simple_driver);

MODULE_AUTHOR("Pamir AI Incorporated");
MODULE_DESCRIPTION("ASoC Pamir AI simple soundcard driver");
MODULE_LICENSE("GPL v2");