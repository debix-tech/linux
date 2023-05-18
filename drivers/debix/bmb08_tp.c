#include <linux/module.h>
#include <linux/clk-provider.h>
#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

struct bmb08_tp {
        struct i2c_client       *client;
	int reset_gpio;
};


static irqreturn_t bmb08_tp_irq(int irq, void *dev_id)
{
        struct bmb08_tp *bmb08_tp = (struct bmb08_tp *)dev_id;
        struct i2c_client *client = bmb08_tp->client;
	int i , flag=0;

	for (i = 0 ; i < 100; i++){
		i2c_smbus_write_byte_data(client, i, i);

		if(bmb08_tp->reset_gpio > 0){
			if(flag==0){
				flag = 1;
			}else{
				flag = 0;

			}
			msleep(10);
			gpio_direction_output(bmb08_tp->reset_gpio, flag);
		}
	}

	printk("GLS_TP %s\n", __func__);

	return IRQ_HANDLED;
}


static int bmb08_tp_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
	struct bmb08_tp *bmb08_tp;
	struct device_node *np = client->dev.of_node;
	int ret = 0 ;
	bmb08_tp = devm_kzalloc(&client->dev, sizeof(*bmb08_tp), GFP_KERNEL);
        if (!bmb08_tp)
                return -ENOMEM;

	printk("GLS_TP %s\n", __func__);
	bmb08_tp->client = client;
        i2c_set_clientdata(client, bmb08_tp);


	bmb08_tp->reset_gpio = of_get_named_gpio(np,"reset-gpios",0);
	if (bmb08_tp->reset_gpio < 0) {
                printk("GLS_TP can't read property reset_gpio\n");
                bmb08_tp->reset_gpio = -1;
        }else{
                ret = devm_gpio_request_one(&client->dev, bmb08_tp->reset_gpio,
                                            GPIOF_DIR_OUT, "bmb08_tp_reset");
                if (ret) {
                        printk( "GLS_TP Failed to request reset_gpio\n");
                        bmb08_tp->reset_gpio = -1;
                }else{
                        gpio_direction_output(bmb08_tp->reset_gpio, 0);
                }

	}

        if (client->irq > 0) {
                ret = devm_request_threaded_irq(&client->dev, client->irq,
                                                NULL, bmb08_tp_irq,
                                                IRQF_TRIGGER_RISING | IRQF_ONESHOT,
                                                client->name, bmb08_tp);
                if (ret < 0) {
                        dev_err(&client->dev, "irq %d request failed, %d\n",
                                client->irq, ret);
                        return ret;
                }
        }

   return 0;
}


static const struct i2c_device_id bmb08_tp_id[] = {
        { "bmb08_tp", 0 },
        {},
};
MODULE_DEVICE_TABLE(i2c, bmb08_tp_id);

static const struct of_device_id bmb08_tp_dt_idtable[] = {
        { .compatible = "polyhex,bmb08_tp" },
        {},
};
MODULE_DEVICE_TABLE(of, bmb08_tp_dt_idtable);


static struct i2c_driver bmb08_tp_driver = {
        .driver         = {
                .name   = "bmb08-tp",
                .of_match_table = bmb08_tp_dt_idtable,
        },
        .probe          = bmb08_tp_probe,
        .id_table       = bmb08_tp_id,
};

module_i2c_driver(bmb08_tp_driver);

MODULE_AUTHOR("John_gao <john@polyhex.net>");
MODULE_DESCRIPTION("BMB08-TP driver");
MODULE_LICENSE("GPL");

