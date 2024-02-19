#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/timer.h>

struct debix_pwm_dev {
	struct pwm_chip chip; 			 /* pwm设备 */
	int power_gpio;
	int pwm_gpio;                        /* pwm输出gpio */
	struct device_node *node;        /* pwm设备节点 */
	struct timer_list period_timer;  /* 周期定时器 */
	struct timer_list duty_timer;    /* 占空比定时器 */
	unsigned int period;             /* pwm周期 */
	unsigned int duty;               /* pwm占空比 */
};

static inline struct debix_pwm_dev *to_debix_pwm_dev(struct pwm_chip *chip) {
	return container_of(chip, struct debix_pwm_dev, chip);
}

#if 0
int debix_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm,
		  int duty_ns, int period_ns) {

	struct debix_pwm_dev *debix_pwm = to_debix_pwm_dev(chip);

	/* 占空比和周期都以ms为单位 */
	if(duty_ns >= period_ns) {
		duty_ns = period_ns - 1;
	}

	debix_pwm->period = period_ns;
	debix_pwm->duty = duty_ns;

	return 0;
}
#endif

#if 1
int debix_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm) {

	struct debix_pwm_dev *debix_pwm = to_debix_pwm_dev(chip);
	printk("debix_pwm: %s \n", __func__);

	/* pwm gpio拉高 */
	gpio_set_value(debix_pwm->pwm_gpio, 1);
	gpio_set_value(debix_pwm->power_gpio, 1);

	/* 开启周期定时器和占空比定时器 */
#if 0 //John_gao 修改为 微妙级别 - period 越大 时间越短
		mod_timer(&debix_pwm->period_timer, jiffies + msecs_to_jiffies(debix_pwm->period));
		mod_timer(&debix_pwm->duty_timer, jiffies + msecs_to_jiffies(debix_pwm->duty));
#else
		mod_timer(&debix_pwm->period_timer, jiffies + usecs_to_jiffies(debix_pwm->period));
		mod_timer(&debix_pwm->duty_timer, jiffies + usecs_to_jiffies(debix_pwm->duty));
#endif

	
	return 0;
}

void debix_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm) {

	struct debix_pwm_dev *debix_pwm = to_debix_pwm_dev(chip);
	printk("debix_pwm: %s \n", __func__);

	/* 删除定时器 */
	del_timer_sync(&debix_pwm->duty_timer);
	del_timer_sync(&debix_pwm->period_timer);

	/* pwm gpio拉低*/
	gpio_set_value(debix_pwm->pwm_gpio, 1);
	gpio_set_value(debix_pwm->power_gpio, 1);

}
#endif
static int debix_pwm_apply(struct pwm_chip *chip, struct pwm_device *pwm,
                           const struct pwm_state *state)
{
	struct debix_pwm_dev *debix_pwm = to_debix_pwm_dev(chip);
	printk("debix_pwm: %s \n", __func__);
	if(state->enabled == true){
		debix_pwm->period = state->period;
		debix_pwm->duty = state->duty_cycle;
		debix_pwm_enable(chip,pwm);
	}else{
		debix_pwm->period = 0;
		debix_pwm->duty = 0;
		debix_pwm_disable(chip,pwm);	
	}

	

	return 0;
}
static void debix_pwm_get_state(struct pwm_chip *chip,
                                struct pwm_device *pwm, struct pwm_state *state)
{
	printk("debix_pwm: %s \n", __func__);
}
static const struct pwm_ops debix_pwm_ops = {
	.owner = THIS_MODULE,
	//.config = debix_pwm_config,
	.apply = debix_pwm_apply,
	.get_state = debix_pwm_get_state,
	//.request = debix_pwm_enable,
	//.free = debix_pwm_disable
};

static void period_timer_function(struct timer_list *t) {
	struct debix_pwm_dev *debix_pwm = from_timer(debix_pwm, t, period_timer);

	//printk("GLS_PWM %s \n",__func__);

	/*设置 pwm gpio为1 */
	gpio_set_value(debix_pwm->pwm_gpio, 1);

	if( debix_pwm->period != 0 && debix_pwm->duty != 0){
		/* 重新开启周期定时器和占空比定时器 */
#if 0 //John_gao 修改为 微妙级别 - period 越大 时间越短
		mod_timer(&debix_pwm->period_timer, jiffies + msecs_to_jiffies(debix_pwm->period));
		mod_timer(&debix_pwm->duty_timer, jiffies + msecs_to_jiffies(debix_pwm->duty));
#else
		mod_timer(&debix_pwm->period_timer, jiffies + usecs_to_jiffies(debix_pwm->period));
		mod_timer(&debix_pwm->duty_timer, jiffies + usecs_to_jiffies(debix_pwm->duty));
#endif
	}
}

static void duty_timer_function(struct timer_list *t) {
	struct debix_pwm_dev *debix_pwm = from_timer(debix_pwm, t, duty_timer);

	//printk("GLS_PWM %s \n",__func__);
	/* 设置pwm gpio为0 */
	gpio_set_value(debix_pwm->pwm_gpio, 0);
}

int debix_pwm_probe(struct platform_device *pdev) {

	struct debix_pwm_dev *debix_pwm;
	int ret;

	/* 申请debix_pwm_dev对象 */
	debix_pwm = devm_kzalloc(&pdev->dev, sizeof(struct debix_pwm_dev), GFP_KERNEL);
	if(!debix_pwm) {
		printk(KERN_ERR"debix_pwm:failed malloc debix_pwm_dev\n");
		return -ENOMEM;
	}
		
	/* 获取debix-pwm设备树节点 */
	debix_pwm->node = of_find_node_by_path("/debix-pwm");
	if(debix_pwm->node == NULL) {
		printk(KERN_ERR"debix_pwm:failed to get debix-pwm node\n");
		return -EINVAL;
	}

	/* 获取pwm输出gpio */
	debix_pwm->pwm_gpio = of_get_named_gpio(debix_pwm->node, "pwm-gpios", 0);
	if(!gpio_is_valid(debix_pwm->pwm_gpio)) {
		printk(KERN_ERR"debix_pwm:failed to get pwm gpio\n");
		return -EINVAL;
	}

	debix_pwm->power_gpio = of_get_named_gpio(debix_pwm->node, "power-gpios", 0);
	if(!gpio_is_valid(debix_pwm->power_gpio)) {
		printk(KERN_ERR"debix_pwm:failed to get power gpio\n");
		//return -EINVAL;
		debix_pwm->power_gpio = -1;
	}


	/* 申请gpio */
	
	ret = gpio_request(debix_pwm->pwm_gpio, "pwm-gpio");
	if(ret) {
		printk(KERN_ERR"debix_pwm:failed to request pwm gpio\n");
	}
	if(debix_pwm->power_gpio > 0){
		ret = gpio_request(debix_pwm->power_gpio, "power-gpio");
		if(ret) {
			printk(KERN_ERR"debix_pwm:failed to request pwm gpio\n");
		}
	}


	debix_pwm->period = 0;
	debix_pwm->duty = 0;

	/* 设置管脚为输出模式 */
	gpio_direction_output(debix_pwm->pwm_gpio, 1);
	if(debix_pwm->power_gpio > 0){
		gpio_direction_output(debix_pwm->power_gpio, 1);
	}

	{
		/*
		struct gpio_desc *desc = gpio_to_desc(debix_pwm->pwm_gpio);
		struct gpio_chip *gc = gpiod_to_chip(desc);	
		void __iomem *regs;
		printk("GLS gc->reg_dir_out = 0x%x \n", readl((gc->reg_dir_out)));
		printk("GLS gc->bgpio_dir = 0x%x \n", readl((gc->bgpio_dir)));

		regs = ioremap(0x43810040, 4);
		printk("GLS 0x43810040 = 0x%x \n", readl((regs)));
		*/
		printk("GLS pwm : %d \n", debix_pwm->pwm_gpio);
		printk("GLS power : %d \n", debix_pwm->power_gpio);
	}

	
	/* 初始化周期控制定时器 */
	timer_setup(&debix_pwm->period_timer, period_timer_function, 0);
	add_timer(&debix_pwm->period_timer);
	//debix_pwm->period_timer.function = period_timer_function;
	//debix_pwm->period_timer.data = (unsigned long)debix_pwm;

	/* 初始化占空比控制定时器 */
	timer_setup(&debix_pwm->duty_timer, duty_timer_function, 0);
	add_timer(&debix_pwm->duty_timer);
	//debix_pwm->duty_timer.function = duty_timer_function;
	//debix_pwm->duty_timer.data = (unsigned long)debix_pwm;

	/* 注册pwm设备 */
	debix_pwm->chip.dev = &pdev->dev;
	debix_pwm->chip.ops = &debix_pwm_ops;
	//debix_pwm->chip.base = 0;
	debix_pwm->chip.npwm = 1;
/*	ret = of_property_read_u32(debix_pwm->node, "npwm", &debix_pwm->chip.npwm);
	if(ret) {
		dev_err(&pdev->dev, "failed to read npwm\n");
		return ret;
	} */
	ret = devm_pwmchip_add(&pdev->dev,&debix_pwm->chip);
	if(ret < 0) {
		dev_err(&pdev->dev, "pwmchip_add failed:%d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, debix_pwm);
	
	return ret;
}

int debix_pwm_remove(struct platform_device *pdev) {

	struct debix_pwm_dev *debix_pwm = platform_get_drvdata(pdev);

	/* 删除定时器 */
	del_timer_sync(&debix_pwm->duty_timer);
	del_timer_sync(&debix_pwm->period_timer);

	/* 释放pwm gpio */
	gpio_set_value(debix_pwm->pwm_gpio, 0);
	if(debix_pwm->power_gpio > 0){
		gpio_set_value(debix_pwm->power_gpio, 0);
	}
	gpio_free(debix_pwm->pwm_gpio);
	if(debix_pwm->power_gpio > 0){
		gpio_free(debix_pwm->power_gpio);
	}
	
	/* 卸载pwm设备 */
	pwmchip_remove(&debix_pwm->chip);
	return 0;
}

static const struct of_device_id debix_pwm_of_match[] = {
	{.compatible = "debix,debix-pwm"},
	{}
};
MODULE_DEVICE_TABLE(of, debix_pwm_of_match);


static struct platform_driver debix_pwm_driver = {
	.driver = {
		.name = "debix-pwm",
		.of_match_table = debix_pwm_of_match
	},
	.probe = debix_pwm_probe,
	.remove = debix_pwm_remove
};
module_platform_driver(debix_pwm_driver);

MODULE_AUTHOR("John_gao debix pwm");
MODULE_DESCRIPTION("use timer create pwm output");
MODULE_LICENSE("GPL");


