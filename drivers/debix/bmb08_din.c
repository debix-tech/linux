/*
 * polyhex debix John_gao 
 */
#define DEBUG
#define VERBOSE_DEBUG

//#undef DEBUG
//#undef VERBOSE_DEBUG


#include <linux/module.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>


#define DIN1 1
#define DIN2 2
#define DIN3 3
#define DIN4 4

struct bmb08_en {
	int gpio;
	int irq;
	int type;
	//struct gpio_desc *din;
	struct platform_device *pdev;
	const struct platform_data *pdata;
	struct device *dev;
};

struct platform_data {
        int (*enable)(struct bmb08_en *priv);
};

static irqreturn_t bmb08_din_irq(int irq, void *dev_id)
{
	struct bmb08_en *priv = (struct bmb08_en *)dev_id;
	struct platform_device *pdev = priv->pdev;
	struct device *dev = &pdev->dev;

	switch(priv->type){
	case DIN1:	
		dev_err(dev," DIN1 run ...\n");
		break;
	case DIN2:	
		dev_err(dev," DIN2 run ...\n");
		break;
	case DIN3:	
		dev_err(dev," DIN3 run ...\n");
		break;
	case DIN4:	
		dev_err(dev," DIN4 run ...\n");
		break;
	}
	return IRQ_HANDLED;
}

static int din1_enable(struct bmb08_en *priv)
{
	struct platform_device *pdev = priv->pdev;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;

	priv->gpio = of_get_named_gpio(np,"din-gpios",0);
	if (priv->gpio < 0) {
		dev_err(dev,"GLS_DIN gpio is not found\n");
		return -1;
	}
	dev_info(dev,"GLS_DIN1 gpio(%d)\n", priv->gpio);

	priv->type = DIN1;
	priv->irq = gpio_to_irq(priv->gpio);
	if (priv->irq > 0) {
		ret = devm_request_threaded_irq(dev, priv->irq,
				NULL, bmb08_din_irq,
				IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				"din1_irq", priv);
		if (ret < 0) {
			dev_err(dev, "irq %d request failed, %d\n",
					priv->irq, ret);
			return ret;
		}
	}

	return 0;	
}

static int din2_enable(struct bmb08_en *priv)
{
	struct platform_device *pdev = priv->pdev;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;
//	printk("GLS_DIN %s \n", __func__);
	priv->gpio = of_get_named_gpio(np,"din-gpios",0);
	if (priv->gpio < 0) {
		dev_err(dev,"GLS_DIN gpio is not found\n");
		return -1;
	}
	dev_info(dev,"GLS_DIN2 gpio(%d)\n", priv->gpio);

	priv->type = DIN2;
	priv->irq = gpio_to_irq(priv->gpio);
	if (priv->irq > 0) {
		ret = devm_request_threaded_irq(dev, priv->irq,
				NULL, bmb08_din_irq,
				IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				"din2_irq", priv);
		if (ret < 0) {
			dev_err(dev, "irq %d request failed, %d\n",
					priv->irq, ret);
			return ret;
		}
	}


	return 0;	
}

static int din3_enable(struct bmb08_en *priv)
{
	struct platform_device *pdev = priv->pdev;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;
//	printk("GLS_DIN %s \n", __func__);
	priv->gpio = of_get_named_gpio(np,"din-gpios",0);
	if (priv->gpio < 0) {
		dev_err(dev,"GLS_DIN gpio is not found\n");
		return -1;
	}
	dev_info(dev,"GLS_DIN3 gpio(%d)\n", priv->gpio);

	priv->type = DIN3;
	priv->irq = gpio_to_irq(priv->gpio);
	if (priv->irq > 0) {
		ret = devm_request_threaded_irq(dev, priv->irq,
				NULL, bmb08_din_irq,
				IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				"din3_irq", priv);
		if (ret < 0) {
			dev_err(dev, "irq %d request failed, %d\n",
					priv->irq, ret);
			return ret;
		}
	}


	return 0;	
}

static int din4_enable(struct bmb08_en *priv)
{
	struct platform_device *pdev = priv->pdev;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;
//	printk("GLS_DIN %s \n", __func__);
	priv->gpio = of_get_named_gpio(np,"din-gpios",0);
	if (priv->gpio < 0) {
		dev_err(dev,"GLS_DIN gpio is not found\n");
		return -1;
	}
	dev_info(dev,"GLS_DIN4 gpio(%d)\n", priv->gpio);

	priv->type = DIN4;
	priv->irq = gpio_to_irq(priv->gpio);
	if (priv->irq > 0) {
		ret = devm_request_threaded_irq(dev, priv->irq,
				NULL, bmb08_din_irq,
				IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				"din4_irq", priv);
		if (ret < 0) {
			dev_err(dev, "irq %d request failed, %d\n",
					priv->irq, ret);
			return ret;
		}
	}


	return 0;	
}
static const struct platform_data din1_plat = {
        .enable = &din1_enable,
};

static const struct platform_data din2_plat = {
        .enable = &din2_enable,
};
static const struct platform_data din3_plat = {
        .enable = &din3_enable,
};
static const struct platform_data din4_plat = {
        .enable = &din4_enable,
};

static const struct of_device_id bmb08_din_dt_ids[] = {
#if 1
        { .compatible = "bmb08-din1", .data = &din1_plat },
        { .compatible = "bmb08-din2", .data = &din2_plat },
        { .compatible = "bmb08-din3", .data = &din3_plat },
        { .compatible = "bmb08-din4", .data = &din4_plat },
        {}
#else
        { .compatible = "polyhex,bmb08-din1" },
#endif
};
MODULE_DEVICE_TABLE(of, bmb08_din_dt_ids);

static int bmb08_din_probe(struct platform_device *pdev)
{
	struct bmb08_en *priv;
	struct device *dev = &pdev->dev;
	const struct of_device_id *of_id = of_match_device(bmb08_din_dt_ids, dev);
	//struct device_node      *np = dev->of_node;

	//int type,ret;
	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
        if (!priv)
                return -ENOMEM;
#if 0
	//if(np == NULL) printk("GLS_DIN np is null\n");
	//ret = of_property_read_u32(np, "din-type", &type);
	//printk("GLS_DIN ret(%d) type(%d) \n", ret, type);

	priv->din = devm_gpiod_get_optional(dev, "din",
			GPIOD_OUT_LOW |
			GPIOD_FLAGS_BIT_NONEXCLUSIVE);
        if (IS_ERR(priv->din)) {
                ret = PTR_ERR(priv->din);
                dev_err(dev, "GLS_DIN Failed to get reset gpio (%d)\n", ret);
                return ret;
        }
	printk("GLS_DIN gpio(%d)\n", of_get_named_gpio(np,"din-gpios",0));
	//priv->gpio = desc_to_gpio(priv->din);
	//printk("GLS_DIN gpio(%d) \n", priv->gpio);
#endif
	priv->pdata = of_id->data;
	priv->pdev = pdev;
	platform_set_drvdata(pdev,priv);
	priv->pdata->enable(priv);
	return 0;
}
static int bmb08_din_remove(struct platform_device *pdev)
{
        struct bmb08_en *priv = platform_get_drvdata(pdev);

	//disable_irq(priv->irq);
        devm_free_irq(&pdev->dev,priv->irq, priv);
        kfree(priv);

        return 0;
}

static struct platform_driver bmb08_din_driver = {
        .probe = bmb08_din_probe,
	.remove = bmb08_din_remove,
        .driver = {
                .name = "bmb08-din",
                .of_match_table = bmb08_din_dt_ids,
        },
};
//module_platform_driver(bmb08_din_driver);
//
static int __init bmb08_din_init(void)
{
        return platform_driver_register(&bmb08_din_driver);
}

static void __exit bmb08_din_exit(void)
{
        platform_driver_unregister(&bmb08_din_driver);
}

module_init(bmb08_din_init);
module_exit(bmb08_din_exit);


MODULE_DESCRIPTION("Polyhex Debix bmb08 din");
MODULE_AUTHOR("John gao <john@polyhex.net>");
MODULE_ALIAS("platform:bmb08-din");
MODULE_LICENSE("GPL");

