                                                                     
                                                                     
                                                                     
                                             
/* Consensic Pressure Sensor Driver
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * History: V2.0 --- Driver creation
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <linux/atomic.h>
#include <linux/module.h>
#include <linux/math64.h>

#include <cust_baro.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <linux/hwmsen_helper.h>

#include <mach/mt_pm_ldo.h>

#define POWER_NONE_MACRO MT65XX_POWER_NONE

#define CPS120_I2C_ADDRESS 0x28
#define CPS_DEV_NAME        "cps120"
#define CPS_DRIVER_VERSION "V1.0"
#define MAX_SENSOR_NAME  (32)
#define CPS_BUFSIZE			128

/* power mode */
enum CPS_POWERMODE_ENUM {
	CPS_SUSPEND_MODE = 0x0,
	CPS_NORMAL_MODE,

	CPS_UNDEFINED_POWERMODE = 0xff
};

/* trace */
enum BAR_TRC {
	BAR_TRC_READ  = 0x01,
	BAR_TRC_RAWDATA = 0x02,
	BAR_TRC_IOCTL   = 0x04,
	BAR_TRC_FILTER  = 0x08,
};

/* cps i2c client data */
struct cps_i2c_data {
	struct i2c_client *client;
	struct baro_hw *hw;

	/* sensor info */
	u8 sensor_name[MAX_SENSOR_NAME];
	enum CPS_POWERMODE_ENUM power_mode;
	u32 last_temp_measurement;
	u32 temp_measurement_period;
	/* calculated temperature correction coefficient */
	s32 t_fine;

	/*misc*/
	struct mutex lock;
	atomic_t trace;
	atomic_t suspend;
	atomic_t filter;

	/*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_drv;
#endif
};

#define BAR_TAG                  "[barometer] "
#define BAR_FUN(f)               printk(KERN_ERR BAR_TAG"%s\n", __func__)
#define BAR_ERR(fmt, args...) \
	printk(KERN_ERR BAR_TAG"%s %d : "fmt, __func__, __LINE__, ##args)
#define BAR_LOG(fmt, args...)    printk(KERN_ERR BAR_TAG fmt, ##args)

static struct platform_driver cps_barometer_driver;
static struct i2c_driver cps_i2c_driver;
static struct cps_i2c_data *obj_i2c_data;
static const struct i2c_device_id cps_i2c_id[] = {
	{CPS_DEV_NAME, 0},
	{}
};

static struct i2c_board_info __initdata cps_i2c_info = {
	I2C_BOARD_INFO(CPS_DEV_NAME, CPS120_I2C_ADDRESS)
};

/* I2C operation functions */
#define cps120_RETRY_COUNT	3
#define cps120_DELAY_TM	30	/* ms */
static int cps_i2c_read_block(struct i2c_client *client, u8 *data, u8 len)
{
	struct i2c_msg msgs_wakeup[1] = {
		{
			.addr = client->addr,	.flags = 0,
			.len = 1,		.buf = data
		},
	};
	struct i2c_msg msgs_read[1] = {
		{
			.addr = client->addr,	.flags = I2C_M_RD,
			.len = len,		.buf = data,
		}
	};
	int err;
	int i;

	BAR_FUN();

	if (!client)
		return -EINVAL;
	else if (len > C_I2C_FIFO_SIZE) {
		BAR_ERR(" length %d exceeds %d\n", len, C_I2C_FIFO_SIZE);
		return -EINVAL;
	}

	for (i = 0; i < cps120_RETRY_COUNT; i++) {
		if (i2c_transfer(client->adapter, msgs_wakeup, 1) >= 0) {
			break;
		}
		mdelay(cps120_DELAY_TM);
	}

	err = i2c_transfer(client->adapter, msgs_read, 1);
	if (err != 1) {
		BAR_ERR("i2c_transfer error: (%p %d) %d\n", data, len, err);
		err = -EIO;
	} else {
		err = 0;/*no error*/
	}

	BAR_LOG("%s: %x %x %x %x\n", __func__, data[0], data[1], data[2], data[3]);
	return err;
}

static void cps_power(struct baro_hw *hw, unsigned int on)
{
	static unsigned int power_on;

	BAR_FUN();

	if (hw->power_id != POWER_NONE_MACRO) {/* have externel LDO */
		BAR_LOG("power %s\n", on ? "on" : "off");
		if (power_on == on) {/* power status not change */
			BAR_LOG("ignore power control: %d\n", on);
		} else if (on) {/* power on */
			if (!hwPowerOn(hw->power_id, hw->power_vol,
				CPS_DEV_NAME))
				BAR_ERR("power on failed\n");
		} else {/* power off */
			if (!hwPowerDown(hw->power_id, CPS_DEV_NAME))
				BAR_ERR("power off failed\n");
		}
	}
	power_on = on;
}

static int cps_set_powermode(struct i2c_client *client,
		enum CPS_POWERMODE_ENUM power_mode)
{
	struct cps_i2c_data *obj = i2c_get_clientdata(client);
	u8 err = 0, data = 0, actual_power_mode = 0;

	BAR_LOG("[%s] power_mode = %d, old power_mode = %d\n", __func__,
		power_mode, obj->power_mode);

	if (power_mode == obj->power_mode)
		return 0;

	mutex_lock(&obj->lock);

	if (err < 0)
		BAR_ERR("set power mode failed, err = %d, sensor name = %s\n",
			err, obj->sensor_name);
	else
		obj->power_mode = power_mode;

	mutex_unlock(&obj->lock);
	return err;
}

static int cps_read_raw_temperature(struct i2c_client *client,
			s32 *temperature)
{
	struct cps_i2c_data *obj = i2c_get_clientdata(client);
	s32 err = 0;
	u32 tmp;

	BAR_FUN();

	if (NULL == client) {
		err = -EINVAL;
		return err;
	}

	mutex_lock(&obj->lock);

	err = cps_i2c_read_block(client, (u8 *)&tmp, sizeof(tmp));
	if (err < 0) {
		BAR_ERR("read raw temperature failed, err = %d\n", err);
		mutex_unlock(&obj->lock);
		return err;
	}
	*temperature = (((tmp>>8)&0x0000ff00)|((tmp>>24)&0x000000ff))>>2 ;
	BAR_LOG("read raw temperature = %x\n", *temperature);
	
	obj->last_temp_measurement = jiffies;
	mutex_unlock(&obj->lock);

	return err;
}

static int cps_read_raw_pressure(struct i2c_client *client, s32 *pressure)
{
	struct cps_i2c_data *priv = i2c_get_clientdata(client);
	s32 err = 0;
	u32 tmp = 0;
	u8 data;

	BAR_FUN();

	if (NULL == client) {
		err = -EINVAL;
		return err;
	}

	mutex_lock(&priv->lock);

	err = cps_i2c_read_block(client, (u8 *)&tmp, 4);
	if (err < 0) {
		BAR_ERR("read raw pressure failed, err = %d\n", err);
		mutex_unlock(&priv->lock);
		return err;
	}
	*pressure = ((tmp<<8)&0x00003f00)|((tmp>>8)&0x000000ff) ;
	BAR_LOG("read raw pressure = %x\n", *pressure);

	mutex_unlock(&priv->lock);
	return err;
}

/*
*get compensated temperature
*unit:10 degrees centigrade
*/
static int cps_get_temperature(struct i2c_client *client,
		char *buf, int bufsize)
{
	struct cps_i2c_data *obj = i2c_get_clientdata(client);
	int status;
	s32 utemp = 0;/* uncompensated temperature */
	s32 temperature = 0;
	signed long temp;

	BAR_FUN();

	if (NULL == buf)
		return -1;

	if (NULL == client) {
		*buf = 0;
		return -2;
	}
	
	status = cps_read_raw_temperature(client, &utemp);
	if (status != 0)
		return status;

	temp = (utemp * 16500)/16384 - 4000;
	temp = (temp/100) * 100;
	BAR_LOG("%s: %ld \n", __func__, temp);
	printk("cps_get_temperature temp=%d\r\n",temp);
	temperature = (s32)temp;

	sprintf(buf, "%08x", temperature);
	BAR_LOG("compensated temperature value: %s\n", buf);
	printk("compensated temperature value: %s\n", buf);
	return status;
}

/*
*get compensated pressure
*unit: hectopascal(hPa)
*/
static int cps_get_pressure(struct i2c_client *client, char *buf, int bufsize)
{
	struct cps_i2c_data *obj = i2c_get_clientdata(client);
	int status;
	s32 upressure = 0, pressure = 0;
	signed long temp;

	BAR_FUN();

	if (NULL == buf)
		return -1;

	if (NULL == client) {
		*buf = 0;
		return -2;
	}

	status = cps_read_raw_pressure(client, &upressure);
	if (status != 0)
		goto exit;

	temp = (upressure * 90000)/16384 + 30000;
	temp = (temp/10) * 10;
	BAR_LOG("%s: %ld \n", __func__, temp);
	pressure = (s32)temp;

	sprintf(buf, "%08x", pressure);
	BAR_LOG("compensated pressure value: %s\n", buf);
	printk("compensated pressure value: %s\r\n",buf);
exit:
	return status;
}

/* cps setting initialization */
static int cps_init_client(struct i2c_client *client)
{
	int err = 0;
	BAR_FUN();

	err = cps_set_powermode(client, CPS_SUSPEND_MODE);
	if (err < 0) {
		BAR_ERR("set power mode failed, err = %d\n", err);
		return err;
	}

	return 0;
}

static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct cps_i2c_data *obj = obj_i2c_data;
	BAR_FUN();

	if (NULL == obj) {
		BAR_ERR("cps i2c data pointer is null\n");
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "%s\n", obj->sensor_name);
}

static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct cps_i2c_data *obj = obj_i2c_data;
	char pre_strbuf[CPS_BUFSIZE] = "";
	char tmp_strbuf[CPS_BUFSIZE] = "";
	BAR_FUN();

	if (NULL == obj) {
		BAR_ERR("cps i2c data pointer is null\n");
		return 0;
	}

	cps_get_pressure(obj->client, pre_strbuf, CPS_BUFSIZE);
	cps_get_temperature(obj->client, tmp_strbuf, CPS_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "pressure:%s,temperature:%s\n", pre_strbuf, tmp_strbuf);
}

static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct cps_i2c_data *obj = obj_i2c_data;
	BAR_FUN();

	if (obj == NULL) {
		BAR_ERR("cps i2c data pointer is null\n");
		return 0;
	}

	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));
	return res;
}

static ssize_t store_trace_value(struct device_driver *ddri, const char *buf,
		size_t count)
{
	struct cps_i2c_data *obj = obj_i2c_data;
	int trace;
	BAR_FUN();

	if (obj == NULL) {
		BAR_ERR("i2c_data obj is null\n");
		return 0;
	}

	if (1 == sscanf(buf, "0x%x", &trace))
		atomic_set(&obj->trace, trace);
	else
		BAR_ERR("invalid content: '%s', length = %d\n", buf, count);

	return count;
}

static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	struct cps_i2c_data *obj = obj_i2c_data;
	BAR_FUN();

	if (obj == NULL) {
		BAR_ERR("cps i2c data pointer is null\n");
		return 0;
	}

	if (obj->hw)
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n",
			obj->hw->i2c_num,
			obj->hw->direction,
			obj->hw->power_id,
			obj->hw->power_vol);
	else
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");

	len += snprintf(buf+len, PAGE_SIZE-len, "i2c addr:%#x,ver:%s\n",
			obj->client->addr, CPS_DRIVER_VERSION);

	return len;
}

static ssize_t show_power_mode_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	struct cps_i2c_data *obj = obj_i2c_data;
	BAR_FUN();

	if (obj == NULL) {
		BAR_ERR("cps i2c data pointer is null\n");
		return 0;
	}

	len += snprintf(buf+len, PAGE_SIZE-len, "%s mode\n",
		obj->power_mode == CPS_NORMAL_MODE ? "normal" : "suspend");

	return len;
}

static ssize_t store_power_mode_value(struct device_driver *ddri,
		const char *buf, size_t count)
{
	struct cps_i2c_data *obj = obj_i2c_data;
	unsigned long power_mode;
	int err;
	BAR_FUN();

	if (obj == NULL) {
		BAR_ERR("cps i2c data pointer is null\n");
		return 0;
	}

	err = kstrtoul(buf, 10, &power_mode);

	if (err == 0) {
		err = cps_set_powermode(obj->client,
			(enum CPS_POWERMODE_ENUM)(!!(power_mode)));
		if (err)
			return err;
		return count;
	}
	return err;
}

static DRIVER_ATTR(chipinfo,	S_IRUGO,	show_chipinfo_value,	NULL);
static DRIVER_ATTR(sensordata,	S_IRUGO,	show_sensordata_value,	NULL);
static DRIVER_ATTR(trace,	S_IWUSR | S_IRUGO,
		show_trace_value,	store_trace_value);
static DRIVER_ATTR(status,	S_IRUGO,	show_status_value,	NULL);
static DRIVER_ATTR(powermode,	S_IWUSR | S_IRUGO,
		show_power_mode_value,	store_power_mode_value);

static struct driver_attribute *cps_attr_list[] = {
	&driver_attr_chipinfo,	/* chip information*/
	&driver_attr_sensordata,/* dump sensor data*/
	&driver_attr_trace,	/* trace log*/
	&driver_attr_status,	/* cust setting */
	&driver_attr_powermode,	/* power mode */
};

static int cps_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = (int)(sizeof(cps_attr_list)/sizeof(cps_attr_list[0]));
	BAR_FUN();

	if (NULL == driver)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		err = driver_create_file(driver, cps_attr_list[idx]);
		if (err) {
			BAR_ERR("driver_create_file (%s) = %d\n",
			cps_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}

static int cps_delete_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = (int)(sizeof(cps_attr_list)/sizeof(cps_attr_list[0]));
	BAR_FUN();

	if (NULL == driver)
		return -EINVAL;

	for (idx = 0; idx < num; idx++)
		driver_remove_file(driver, cps_attr_list[idx]);

	return err;
}

int barometer_operate(void *self, uint32_t command, void *buff_in, int size_in,
		void *buff_out, int size_out, int *actualout)
{
	int err = 0;
	int value;
	struct cps_i2c_data *priv = (struct cps_i2c_data *)self;
	hwm_sensor_data *barometer_data;
	char buff[CPS_BUFSIZE];
	BAR_FUN();

	switch (command) {
	case SENSOR_DELAY:
		/* under construction */
	break;

	case SENSOR_ENABLE:
	if ((buff_in == NULL) || (size_in < sizeof(int))) {
		BAR_ERR("enable sensor parameter error\n");
		err = -EINVAL;
	} else {
		/* value:[0--->suspend, 1--->normal] */
		value = *(int *)buff_in;
		BAR_LOG("sensor enable/disable command: %s\n",
			value ? "enable" : "disable");

	err = cps_set_powermode(priv->client,
		(enum CPS_POWERMODE_ENUM)(!!value));
	if (err)
		BAR_ERR("set power mode failed, err = %d\n", err);
	}
	break;

	case SENSOR_GET_DATA:
	if ((buff_out == NULL) || (size_out < sizeof(hwm_sensor_data))) {
		BAR_ERR("get sensor data parameter error\n");
		err = -EINVAL;
	} else {
		barometer_data = (hwm_sensor_data *)buff_out;
		err = cps_get_pressure(priv->client, buff, CPS_BUFSIZE);
		if (err) {
			BAR_ERR("get compensated pressure value failed,"
				"err = %d\n", err);
			return -1;
		}
		sscanf(buff, "%x", &barometer_data->values[0]);
		barometer_data->values[1] = barometer_data->values[2] = 0;
		barometer_data->status = SENSOR_STATUS_ACCURACY_HIGH;
		barometer_data->value_divide = 100;
	}
	break;

	default:
		BAR_ERR("barometer operate function no this parameter %d\n",
			command);
		err = -1;
	break;
	}

	return err;
}

int temperature_operate(void *self, uint32_t command, void *buff_in,
		int size_in, void *buff_out, int size_out, int *actualout)
{
	int err = 0;
	int value;
	struct cps_i2c_data *priv = (struct cps_i2c_data *)self;
	hwm_sensor_data *temperature_data;
	char buff[CPS_BUFSIZE];

	switch (command) {
	case SENSOR_DELAY:
	/* under construction */
	break;

	case SENSOR_ENABLE:
	if ((buff_in == NULL) || (size_in < sizeof(int))) {
		BAR_ERR("enable sensor parameter error\n");
		err = -EINVAL;
	} else {
		/* value:[0--->suspend, 1--->normal] */
		value = *(int *)buff_in;
		BAR_LOG("sensor enable/disable command: %s\n",
			value ? "enable" : "disable");

		err = cps_set_powermode(priv->client,
			(enum CPS_POWERMODE_ENUM)(!!value));
		if (err)
			BAR_ERR("set power mode failed, err = %d\n", err);
		}
	break;
	
	case SENSOR_GET_DATA:
	if ((buff_out == NULL) ||
		(size_out < sizeof(hwm_sensor_data))) {
		BAR_ERR("get sensor data parameter error\n");
		err = -EINVAL;
	} else {
		temperature_data = (hwm_sensor_data *)buff_out;
		err = cps_get_temperature(priv->client, buff, CPS_BUFSIZE);
		if (err) {
			BAR_ERR("get compensated temperature value failed,"
				"err = %d\n", err);
			return -1;
		}
		sscanf(buff, "%x", &temperature_data->values[0]);
		temperature_data->values[1] = temperature_data->values[2] = 0;
		temperature_data->status = SENSOR_STATUS_ACCURACY_HIGH;
		temperature_data->value_divide = 100;
	}
	break;

	default:
		BAR_ERR("temperature operate function no this parameter %d\n",
			command);
		err = -1;
	break;
	}

	return err;
}

static int cps_open(struct inode *inode, struct file *file)
{
	file->private_data = obj_i2c_data;
	BAR_FUN();

	if (file->private_data == NULL) {
		BAR_ERR("null pointer\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}

static int cps_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static long cps_unlocked_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	struct cps_i2c_data *obj = (struct cps_i2c_data *)file->private_data;
	struct i2c_client *client = obj->client;
	char strbuf[CPS_BUFSIZE];
	u32 dat = 0;
	void __user *data;
	int err = 0;
	BAR_FUN();

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,
			(void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ,
			(void __user *)arg, _IOC_SIZE(cmd));

	if (err) {
		BAR_ERR("access error: %08X, (%2d, %2d)\n",
			cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch (cmd) {
	case BAROMETER_IOCTL_INIT:
		cps_init_client(client);
		err = cps_set_powermode(client, CPS_NORMAL_MODE);
		if (err) {
			err = -EFAULT;
			break;
		}
	break;

	case BAROMETER_IOCTL_READ_CHIPINFO:
		data = (void __user *) arg;
		if (NULL == data) {
			err = -EINVAL;
			break;
		}
		strcpy(strbuf, obj->sensor_name);
		if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
			err = -EFAULT;
			break;
		}
	break;
	
	case BAROMETER_GET_PRESS_DATA:
		data = (void __user *) arg;
		if (NULL == data) {
			err = -EINVAL;
			break;
		}

		cps_get_pressure(client, strbuf, CPS_BUFSIZE);
		sscanf(strbuf, "%x", &dat);
		if (copy_to_user(data, &dat, sizeof(dat))) {
			err = -EFAULT;
			break;
		}
	break;

	case BAROMETER_GET_TEMP_DATA:
		data = (void __user *) arg;
		if (NULL == data) {
			err = -EINVAL;
			break;
		}
		cps_get_temperature(client, strbuf, CPS_BUFSIZE);
		sscanf(strbuf, "%x", &dat);
		if (copy_to_user(data, &dat, sizeof(dat))) {
			err = -EFAULT;
			break;
		}
	break;

	default:
		BAR_ERR("unknown IOCTL: 0x%08x\n", cmd);
		err = -ENOIOCTLCMD;
	break;
	}

	return err;
}

static const struct file_operations cps_fops = {
	.owner = THIS_MODULE,
	.open = cps_open,
	.release = cps_release,
	.unlocked_ioctl = cps_unlocked_ioctl,
};

static struct miscdevice cps_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "barometer",
	.fops = &cps_fops,
};

#ifndef CONFIG_HAS_EARLYSUSPEND
static int cps_suspend(struct i2c_client *client, pm_message_t msg)
{
	struct cps_i2c_data *obj = i2c_get_clientdata(client);
	int err = 0;
	BAR_FUN();

	if (msg.event == PM_EVENT_SUSPEND) {
		if (NULL == obj) {
			BAR_ERR("null pointer\n");
			return -EINVAL;
		}

		atomic_set(&obj->suspend, 1);
		err = cps_set_powermode(obj->client, CPS_SUSPEND_MODE);
		if (err) {
			BAR_ERR("cps set suspend mode failed, err = %d\n", err);
			return;
		}
		cps_power(obj->hw, 0);
	}
	return err;
}

static int cps_resume(struct i2c_client *client)
{
	struct cps_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	BAR_FUN();

	if (NULL == obj) {
		BAR_ERR("null pointer\n");
		return -EINVAL;
	}

	cps_power(obj->hw, 1);

	err = cps_init_client(obj->client);
	if (err) {
		BAR_ERR("initialize client fail\n");
		return;
	}

	err = cps_set_powermode(obj->client, CPS_NORMAL_MODE);
	if (err) {
		BAR_ERR("cps set normal mode failed, err = %d\n", err);
		return;
	}

	atomic_set(&obj->suspend, 0);
	return 0;
}
#else
static void cps_early_suspend(struct early_suspend *h)
{
	struct cps_i2c_data *obj = container_of(h, struct cps_i2c_data,
					early_drv);
	int err;
	BAR_FUN();

	if (NULL == obj) {
		BAR_ERR("null pointer\n");
		return;
	}
	atomic_set(&obj->suspend, 1);
	err = cps_set_powermode(obj->client, CPS_SUSPEND_MODE);
	if (err) {
		BAR_ERR("cps set suspend mode failed, err = %d\n", err);
		return;
	}

	cps_power(obj->hw, 0);
}

static void cps_late_resume(struct early_suspend *h)
{
	struct cps_i2c_data *obj = container_of(h, struct cps_i2c_data,
					early_drv);
	int err;
	BAR_FUN();

	if (NULL == obj) {
		BAR_ERR("null pointer\n");
		return;
	}

	cps_power(obj->hw, 1);

	err = cps_init_client(obj->client);
	if (err) {
		BAR_ERR("initialize client fail\n");
		return;
	}

	err = cps_set_powermode(obj->client, CPS_NORMAL_MODE);
	if (err) {
		BAR_ERR("cps set normal mode failed, err = %d\n", err);
		return;
	}

	atomic_set(&obj->suspend, 0);
}
#endif/* CONFIG_HAS_EARLYSUSPEND */

static int cps_i2c_detect(struct i2c_client *client,
		struct i2c_board_info *info)
{
	strcpy(info->type, CPS_DEV_NAME);
	return 0;
}

static int cps_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct cps_i2c_data *obj;
	struct hwmsen_object sobj_p;
	struct hwmsen_object sobj_t;

	int err = 0;
	BAR_FUN();

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj) {
		err = -ENOMEM;
		goto exit;
	}

	obj->hw = get_cust_baro_hw();
	obj_i2c_data = obj;
	obj->client = client;
	i2c_set_clientdata(client, obj);

	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);
	obj->power_mode = CPS_UNDEFINED_POWERMODE;
	obj->last_temp_measurement = 0;
	obj->temp_measurement_period = 1*HZ;/* temperature update period:1s */
	strcpy(obj->sensor_name, "cps120");
	mutex_init(&obj->lock);

	err = cps_init_client(client);
	if (err)
		goto exit_init_client_failed;

	err = misc_register(&cps_device);
	if (err) {
		BAR_ERR("misc device register failed, err = %d\n", err);
		goto exit_misc_device_register_failed;
	}

	err = cps_create_attr(&cps_barometer_driver.driver);
	if (err) {
		BAR_ERR("create attribute failed, err = %d\n", err);
		goto exit_create_attr_failed;
	}

	sobj_p.self = obj;
	sobj_p.polling = 1;
	sobj_p.sensor_operate = barometer_operate;
	err = hwmsen_attach(ID_PRESSURE, &sobj_p);
	if (err) {
		BAR_ERR("hwmsen attach failed, err = %d\n", err);
		goto exit_hwmsen_attach_pressure_failed;
	}

	sobj_t.self = obj;
	sobj_t.polling = 1;
	sobj_t.sensor_operate = temperature_operate;
	err = hwmsen_attach(ID_TEMPRERATURE, &sobj_t);
	if (err) {
		BAR_ERR("hwmsen attach failed, err = %d\n", err);
		goto exit_hwmsen_attach_temperature_failed;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = cps_early_suspend,
	obj->early_drv.resume   = cps_late_resume,
	register_early_suspend(&obj->early_drv);
#endif

	BAR_LOG("%s: OK\n", __func__);
	return 0;

exit_hwmsen_attach_temperature_failed:
	hwmsen_detach(ID_PRESSURE);
exit_hwmsen_attach_pressure_failed:
	cps_delete_attr(&cps_barometer_driver.driver);
exit_create_attr_failed:
	misc_deregister(&cps_device);
exit_misc_device_register_failed:
exit_init_client_failed:
	kfree(obj);
exit:
	BAR_ERR("err = %d\n", err);
	return err;
}

static int cps_i2c_remove(struct i2c_client *client)
{
	int err = 0;
	struct cps_i2c_data *obj = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&obj->early_drv);
#endif

	err = hwmsen_detach(ID_PRESSURE);
	if (err)
		BAR_ERR("hwmsen_detach ID_PRESSURE failed, err = %d\n", err);

	err = hwmsen_detach(ID_TEMPRERATURE);
	if (err)
		BAR_ERR("hwmsen_detach ID_TEMPRERATURE failed, err = %d\n",
			err);

	err = cps_delete_attr(&cps_barometer_driver.driver);
	if (err)
		BAR_ERR("cps_delete_attr failed, err = %d\n", err);

	err = misc_deregister(&cps_device);
	if (err)
		BAR_ERR("misc_deregister failed, err = %d\n", err);

	obj_i2c_data = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}

static int cps_probe(struct platform_device *pdev)
{
	struct baro_hw *hw = get_cust_baro_hw();
	BAR_FUN();

	cps_power(hw, 1);
	if (i2c_add_driver(&cps_i2c_driver)) {
		BAR_ERR("add i2c driver failed\n");
		return -1;
	}

	return 0;
}

static int cps_remove(struct platform_device *pdev)
{
	struct baro_hw *hw = get_cust_baro_hw();
	BAR_FUN();

	cps_power(hw, 0);
	i2c_del_driver(&cps_i2c_driver);

	return 0;
}

static struct i2c_driver cps_i2c_driver = {
	.driver = {
		.owner	=	THIS_MODULE,
		.name	=	CPS_DEV_NAME,
	},
	.probe	=	cps_i2c_probe,
	.remove	=	cps_i2c_remove,
	.detect	=	cps_i2c_detect,
#if !defined(CONFIG_HAS_EARLYSUSPEND)
	.suspend	=	cps_suspend,
	.resume	=	cps_resume,
#endif
	.id_table	=	cps_i2c_id,
};

static struct platform_driver cps_barometer_driver = {
	.probe      = cps_probe,
	.remove     = cps_remove,
	.driver     = {
		.name  = "barometer",
		.owner = THIS_MODULE,
	}
};

static int __init cps_init(void)
{
	struct baro_hw *hw = get_cust_baro_hw();

	BAR_FUN();
	i2c_register_board_info(hw->i2c_num, &cps_i2c_info, 1);
	if (platform_driver_register(&cps_barometer_driver)) {
		BAR_ERR("register cps platform driver failed");
		return -ENODEV;
	}
	return 0;
}

static void __exit cps_exit(void)
{
	BAR_FUN();
	platform_driver_unregister(&cps_barometer_driver);
}

module_init(cps_init);
module_exit(cps_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CPS120 I2C Driver");
MODULE_AUTHOR("leoli@consensic.com");
MODULE_VERSION(CPS_DRIVER_VERSION);