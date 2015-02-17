/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
 
/*
 * Latest Version: V1.6 
 * Release Date: 2013/03/11
 * Contact: andrew@goodix.com, meta@goodix.com
 * Revision Record:
 *      V1.6:
 *          1. New Heartbeat/ESD-protect Mechanism(external watchdog)
 *          2. doze mode, sliding wakeup
 *          3. config length verification & 3 more config groups(GT9 Sensor_ID: 0 ~ 5)
 *          4. charger status switch
 *                  By Meta, 2013/03/11
 */

#include "tpd.h"
#include "tpd_custom_gt9xx.h"
#ifndef TPD_NO_GPIO
#include "cust_gpio_usage.h"
#endif
#ifdef TPD_PROXIMITY
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#endif
#include <linux/slab.h>
extern struct tpd_device *tpd;


static int tpd_flag = 0;
int tpd_halt = 0;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

#if GTP_SLIDING_WAKEUP
u8 doze_enabled = 0;
#endif

#if GTP_CHARGER_SWITCH
    #ifdef MT6573
        #define CHR_CON0      (0xF7000000+0x2FA00)
    #else
        extern kal_bool upmu_is_chr_det(void);
    #endif
#endif 

#if GTP_HAVE_TOUCH_KEY
//const u16 touch_key_array[] = { KEY_MENU, KEY_HOMEPAGE, KEY_BACK, KEY_SEARCH };
const u16 touch_key_array[] = {KEY_BACK, KEY_HOMEPAGE, KEY_MENU, KEY_SEARCH};		//add by jun.zhou@imtele.com 
#define GTP_MAX_KEY_NUM ( sizeof( touch_key_array )/sizeof( touch_key_array[0] ) )
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
//static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

#define CFG_GROUP_LEN(p_cfg_grp)  (sizeof(p_cfg_grp) / sizeof(p_cfg_grp[0]))
s32 gtp_send_cfg(struct i2c_client *client);
static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en,
			  unsigned int pol, void (EINT_FUNC_PTR) (void),
			  unsigned int is_auto_umask);

#if GTP_CREATE_WR_NODE
extern s32 init_wr_node(struct i2c_client *);
extern void uninit_wr_node(void);
#endif

#if GTP_ESD_PROTECT
#define TPD_ESD_CHECK_CIRCLE        2000
static struct delayed_work gtp_esd_check_work;
static struct workqueue_struct *gtp_esd_check_workqueue = NULL;
static s32 gtp_init_ext_watchdog(struct i2c_client *client);
static void gtp_esd_check_func(struct work_struct *);
#endif

#ifdef TPD_PROXIMITY
#define TPD_PROXIMITY_VALID_REG                   0x814E
#define TPD_PROXIMITY_ENABLE_REG                  0x8042
static u8 tpd_proximity_flag = 0;
static u8 tpd_proximity_detect = 1;//0-->close ; 1--> far away
#endif

struct i2c_client *i2c_client_point = NULL;
static const struct i2c_device_id tpd_i2c_id[] = {{"mtk-tpd", 0}, {}};
static unsigned short force[] = {0, 0xBA, I2C_CLIENT_END, I2C_CLIENT_END};
static const unsigned short *const forces[] = { force, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces,};
static struct i2c_board_info __initdata i2c_tpd = { I2C_BOARD_INFO("mtk-tpd", (0xBA >> 1))};
static struct i2c_driver tpd_i2c_driver =
{
    .probe = tpd_i2c_probe,
    .remove = tpd_i2c_remove,
    .detect = tpd_i2c_detect,
    .driver.name = "mtk-tpd",
    .id_table = tpd_i2c_id,
    .address_list = (const unsigned short *) forces,
};


static u8 config[GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH]
    = {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};

#pragma pack(1)
typedef struct
{
    u16 pid;                 //product id   //
    u16 vid;                 //version id   //
} st_tpd_info;
#pragma pack()

static st_tpd_info tpd_info;
u8 int_type = 0;
u32 abs_x_max = 0;
u32 abs_y_max = 0;
u8 gtp_rawdiff_mode = 0;
u8 cfg_len = 0;

/* proc file system */
s32 i2c_read_bytes(struct i2c_client *client, u16 addr, u8 *rxbuf, int len);
s32 i2c_write_bytes(struct i2c_client *client, u16 addr, u8 *txbuf, int len);
static struct proc_dir_entry *gt91xx_config_proc = NULL;

#define VELOCITY_CUSTOM
#ifdef VELOCITY_CUSTOM
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#ifndef TPD_VELOCITY_CUSTOM_X
#define TPD_VELOCITY_CUSTOM_X 10
#endif
#ifndef TPD_VELOCITY_CUSTOM_Y
#define TPD_VELOCITY_CUSTOM_Y 10
#endif

// for magnify velocity********************************************
#define TOUCH_IOC_MAGIC 'A'

#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC,0)
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC,1)

int g_v_magnify_x = TPD_VELOCITY_CUSTOM_X;
int g_v_magnify_y = TPD_VELOCITY_CUSTOM_Y;
static int tpd_misc_open(struct inode *inode, struct file *file)
{
    return nonseekable_open(inode, file);
}

static int tpd_misc_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd,
                               unsigned long arg)
{
    //char strbuf[256];
    void __user *data;

    long err = 0;

    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }

    if (err)
    {
        printk("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
        return -EFAULT;
    }

    switch (cmd)
    {
        case TPD_GET_VELOCITY_CUSTOM_X:
            data = (void __user *) arg;

            if (data == NULL)
            {
                err = -EINVAL;
                break;
            }

            if (copy_to_user(data, &g_v_magnify_x, sizeof(g_v_magnify_x)))
            {
                err = -EFAULT;
                break;
            }

            break;

        case TPD_GET_VELOCITY_CUSTOM_Y:
            data = (void __user *) arg;

            if (data == NULL)
            {
                err = -EINVAL;
                break;
            }

            if (copy_to_user(data, &g_v_magnify_y, sizeof(g_v_magnify_y)))
            {
                err = -EFAULT;
                break;
            }

            break;

        default:
            printk("tpd: unknown IOCTL: 0x%08x\n", cmd);
            err = -ENOIOCTLCMD;
            break;

    }

    return err;
}


static struct file_operations tpd_fops =
{
//  .owner = THIS_MODULE,
    .open = tpd_misc_open,
    .release = tpd_misc_release,
    .unlocked_ioctl = tpd_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice tpd_misc_device =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "touch",
    .fops = &tpd_fops,
};

//**********************************************
#endif

static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    strcpy(info->type, "mtk-tpd");
    return 0;
}

#ifdef TPD_PROXIMITY
static s32 tpd_get_ps_value(void)
{
    return tpd_proximity_detect;
}

static s32 tpd_enable_ps(s32 enable)
{
    u8  state;
    s32 ret = -1;

    if (enable)
    {
        state = 1;
        tpd_proximity_flag = 1;
        GTP_INFO("TPD proximity function to be on.");
    }
    else
    {
        state = 0;
        tpd_proximity_flag = 0;
        GTP_INFO("TPD proximity function to be off.");
    }

    ret = i2c_write_bytes(i2c_client_point, TPD_PROXIMITY_ENABLE_REG, &state, 1);

    if (ret < 0)
    {
        GTP_ERROR("TPD %s proximity cmd failed.", state ? "enable" : "disable");
        return ret;
    }

    GTP_INFO("TPD proximity function %s success.", state ? "enable" : "disable");
    return 0;
}

s32 tpd_ps_operate(void *self, u32 command, void *buff_in, s32 size_in,
                   void *buff_out, s32 size_out, s32 *actualout)
{
    s32 err = 0;
    s32 value;
    hwm_sensor_data *sensor_data;

    switch (command)
    {
        case SENSOR_DELAY:
            if ((buff_in == NULL) || (size_in < sizeof(int)))
            {
                GTP_ERROR("Set delay parameter error!");
                err = -EINVAL;
            }

            // Do nothing
            break;

        case SENSOR_ENABLE:
            if ((buff_in == NULL) || (size_in < sizeof(int)))
            {
                GTP_ERROR("Enable sensor parameter error!");
                err = -EINVAL;
            }
            else
            {
                value = *(int *)buff_in;
                err = tpd_enable_ps(value);
            }

            break;

        case SENSOR_GET_DATA:
            if ((buff_out == NULL) || (size_out < sizeof(hwm_sensor_data)))
            {
                GTP_ERROR("Get sensor data parameter error!");
                err = -EINVAL;
            }
            else
            {
                sensor_data = (hwm_sensor_data *)buff_out;
                sensor_data->values[0] = tpd_get_ps_value();
                sensor_data->value_divide = 1;
                sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
            }

            break;

        default:
            GTP_ERROR("proxmy sensor operate function no this parameter %d!\n", command);
            err = -1;
            break;
    }

    return err;
}
#endif

static int gt91xx_config_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    char *ptr = page;
    char temp_data[GTP_CONFIG_MAX_LENGTH + 2] = {0};
    int i;

    ptr += sprintf(ptr, "==== GT9XX config init value====\n");

    for (i = 0 ; i < GTP_CONFIG_MAX_LENGTH ; i++)
    {
        ptr += sprintf(ptr, "0x%02X ", config[i + 2]);

        if (i % 8 == 7)
            ptr += sprintf(ptr, "\n");
    }

    ptr += sprintf(ptr, "\n");

    ptr += sprintf(ptr, "==== GT9XX config real value====\n");
    i2c_read_bytes(i2c_client_point, GTP_REG_CONFIG_DATA, temp_data, GTP_CONFIG_MAX_LENGTH);

    for (i = 0 ; i < GTP_CONFIG_MAX_LENGTH ; i++)
    {
        ptr += sprintf(ptr, "0x%02X ", temp_data[i]);

        if (i % 8 == 7)
            ptr += sprintf(ptr, "\n");
    }


    *eof = 1;
    return (ptr - page);
}

static int gt91xx_config_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
    s32 ret = 0;

    GTP_DEBUG("write count %ld\n", count);

    if (count > GTP_CONFIG_MAX_LENGTH)
    {
        GTP_ERROR("size not match [%d:%ld]\n", GTP_CONFIG_MAX_LENGTH, count);
        return -EFAULT;
    }

    if (copy_from_user(&config[2], buffer, count))
    {
        GTP_ERROR("copy from user fail\n");
        return -EFAULT;
    }

    ret = gtp_send_cfg(i2c_client_point);
    abs_x_max = (config[RESOLUTION_LOC + 1] << 8) + config[RESOLUTION_LOC];
    abs_y_max = (config[RESOLUTION_LOC + 3] << 8) + config[RESOLUTION_LOC + 2];
    int_type = (config[TRIGGER_LOC]) & 0x03;

    if (ret < 0)
    {
        GTP_ERROR("send config failed.");
    }

    return count;
}

int i2c_read_bytes(struct i2c_client *client, u16 addr, u8 *rxbuf, int len)
{
    u8 buffer[GTP_ADDR_LENGTH];
    u8 retry;
    u16 left = len;
    u16 offset = 0;

    struct i2c_msg msg[2] =
    {
        {
            //.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
            .addr = ((client->addr &I2C_MASK_FLAG) | (I2C_PUSHPULL_FLAG)),
            .flags = 0,
            .buf = buffer,
            .len = GTP_ADDR_LENGTH,
            .timing = I2C_MASTER_CLOCK
        },
        {
            //.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
            .addr = ((client->addr &I2C_MASK_FLAG) | (I2C_PUSHPULL_FLAG)),
            .flags = I2C_M_RD,
            .timing = I2C_MASTER_CLOCK
        },
    };

    if (rxbuf == NULL)
        return -1;

    GTP_DEBUG("i2c_read_bytes to device %02X address %04X len %d\n", client->addr, addr, len);

    while (left > 0)
    {
        buffer[0] = ((addr + offset) >> 8) & 0xFF;
        buffer[1] = (addr + offset) & 0xFF;

        msg[1].buf = &rxbuf[offset];

        if (left > MAX_TRANSACTION_LENGTH)
        {
            msg[1].len = MAX_TRANSACTION_LENGTH;
            left -= MAX_TRANSACTION_LENGTH;
            offset += MAX_TRANSACTION_LENGTH;
        }
        else
        {
            msg[1].len = left;
            left = 0;
        }

        retry = 0;

        if (i2c_transfer(client->adapter, &msg[0], 2) != 2)
        {
//            retry++;

//            if (retry == 20)
//            {
                GTP_ERROR("I2C read 0x%X length=%d failed\n", addr + offset, len);
                return -1;
            //}
        }
    }

    return 0;
}

s32 gtp_i2c_read(struct i2c_client *client, u8 *buf, s32 len)
{
    s32 ret = -1;
    u16 addr = (buf[0] << 8) + buf[1];

    ret = i2c_read_bytes(client, addr, &buf[2], len - 2);

    if (!ret)
    {
        return 2;
    }
    else
    {
    
    #if GTP_SLIDING_WAKEUP
        if (doze_enabled)
        {
            return ret;
        }
    #endif
        gtp_reset_guitar(client, 20);
        return ret;
    }
}

int i2c_write_bytes(struct i2c_client *client, u16 addr, u8 *txbuf, int len)
{
    u8 buffer[MAX_TRANSACTION_LENGTH];
    u16 left = len;
    u16 offset = 0;
    u8 retry = 0;

    struct i2c_msg msg =
    {
        //.addr = ((client->addr &I2C_MASK_FLAG) | (I2C_ENEXT_FLAG)),
        .addr = ((client->addr &I2C_MASK_FLAG) | (I2C_PUSHPULL_FLAG)),
        .flags = 0,
        .buf = buffer,
        .timing = I2C_MASTER_CLOCK,
    };


    if (txbuf == NULL)
        return -1;

    GTP_DEBUG("i2c_write_bytes to device %02X address %04X len %d\n", client->addr, addr, len);

    while (left > 0)
    {
        retry = 0;

        buffer[0] = ((addr + offset) >> 8) & 0xFF;
        buffer[1] = (addr + offset) & 0xFF;

        if (left > MAX_I2C_TRANSFER_SIZE)
        {
            memcpy(&buffer[GTP_ADDR_LENGTH], &txbuf[offset], MAX_I2C_TRANSFER_SIZE);
            msg.len = MAX_TRANSACTION_LENGTH;
            left -= MAX_I2C_TRANSFER_SIZE;
            offset += MAX_I2C_TRANSFER_SIZE;
        }
        else
        {
            memcpy(&buffer[GTP_ADDR_LENGTH], &txbuf[offset], left);
            msg.len = left + GTP_ADDR_LENGTH;
            left = 0;
        }

        //GTP_DEBUG("byte left %d offset %d\n", left, offset);

        while (i2c_transfer(client->adapter, &msg, 1) != 1)
        {
            retry++;

            if (retry == 20)
            {
                GTP_ERROR("I2C write 0x%X%X length=%d failed\n", buffer[0], buffer[1], len);
                return -1;
            }


        }
    }

    return 0;
}

s32 gtp_i2c_write(struct i2c_client *client, u8 *buf, s32 len)
{
    s32 ret = -1;
    u16 addr = (buf[0] << 8) + buf[1];

    ret = i2c_write_bytes(client, addr, &buf[2], len - 2);

    if (!ret)
    {
        return 1;
    }
    else
    {
    #if GTP_SLIDING_WAKEUP
        if (doze_enabled)
        {
            return ret;
        }
    #endif
        gtp_reset_guitar(client, 20);
        return ret;
    }
}



/*******************************************************
Function:
    Send config Function.

Input:
    client: i2c client.

Output:
    Executive outcomes.0--success,non-0--fail.
*******************************************************/
s32 gtp_send_cfg(struct i2c_client *client)
{
    s32 ret = 0;
#if GTP_DRIVER_SEND_CFG	//don't do it,i2c_write will error
    s32 retry = 0;

    for (retry = 0; retry < 5; retry++)
    {//jun.zhou

        ret = gtp_i2c_write(client, config, GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH);

        if (ret > 0)
        {
            break;
        }
    }

#endif

    return ret;
}


/*******************************************************
Function:
    Read goodix touchscreen version function.

Input:
    client: i2c client struct.
    version:address to store version info

Output:
    Executive outcomes.0---succeed.
*******************************************************/
s32 gtp_read_version(struct i2c_client *client, u16 *version)
{
    s32 ret = -1;
    s32 i;
    u8 buf[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};

    GTP_DEBUG_FUNC();

    ret = gtp_i2c_read(client, buf, sizeof(buf));

    if (ret < 0)
    {
        GTP_ERROR("GTP read version failed");
        return ret;
    }

    if (version)
    {
        *version = (buf[7] << 8) | buf[6];
    }

    tpd_info.vid = *version;
    tpd_info.pid = 0x00;

    for (i = 0; i < 4; i++)
    {
        if (buf[i + 2] < 0x30)break;

        tpd_info.pid |= ((buf[i + 2] - 0x30) << ((3 - i) * 4));
    }

    if (buf[5] == 0x00)
    {
        GTP_INFO("IC VERSION: %c%c%c_%02x%02x",
             buf[2], buf[3], buf[4], buf[7], buf[6]);  
    }
    else
    {
        GTP_INFO("IC VERSION:%c%c%c%c_%02x%02x",
             buf[2], buf[3], buf[4], buf[5], buf[7], buf[6]);
    }
    return ret;
}
/*******************************************************
Function:
    GTP initialize function.

Input:
    client: i2c client private struct.

Output:
    Executive outcomes.0---succeed.
*******************************************************/
static s32 gtp_init_panel(struct i2c_client *client)
{
    s32 ret = -1;
	printk("zhoujun_gt915  [gtp_init_panel] is running\n");
    struct goodix_ts_data *ts = i2c_get_clientdata(client);
	ts = kmalloc(sizeof(struct goodix_ts_data),GFP_KERNEL);	
#if GTP_DRIVER_SEND_CFG
    s32 i;
    u8 check_sum = 0;
    u8 opr_buf[16];
    u8 sensor_id = 0;

    u8 cfg_info_group1[] = CTP_CFG_GROUP1;
    u8 cfg_info_group2[] = CTP_CFG_GROUP2;
    u8 cfg_info_group3[] = CTP_CFG_GROUP3;
    u8 cfg_info_group4[] = CTP_CFG_GROUP4;
    u8 cfg_info_group5[] = CTP_CFG_GROUP5;
    u8 cfg_info_group6[] = CTP_CFG_GROUP6;
    u8 *send_cfg_buf[] = {cfg_info_group1, cfg_info_group2, cfg_info_group3,
                        cfg_info_group4, cfg_info_group5, cfg_info_group6};
    u8 cfg_info_len[] = { CFG_GROUP_LEN(cfg_info_group1), 
                          CFG_GROUP_LEN(cfg_info_group2),
                          CFG_GROUP_LEN(cfg_info_group3),
                          CFG_GROUP_LEN(cfg_info_group4), 
                          CFG_GROUP_LEN(cfg_info_group5),
                          CFG_GROUP_LEN(cfg_info_group6)};

    GTP_DEBUG("Config Groups\' Lengths: %d, %d, %d, %d, %d, %d", 
        cfg_info_len[0], cfg_info_len[1], cfg_info_len[2], cfg_info_len[3],
        cfg_info_len[4], cfg_info_len[5]);


    if ((!cfg_info_len[1]) && (!cfg_info_len[2]) && 
        (!cfg_info_len[3]) && (!cfg_info_len[4]) && 
        (!cfg_info_len[5]))
    {
        sensor_id = 0; 
    }
    else
    {
        opr_buf[0] = (u8)(GTP_REG_SENSOR_ID >> 8);
        opr_buf[1] = (u8)(GTP_REG_SENSOR_ID & 0xff);
        ret = gtp_i2c_read(ts->client, opr_buf, 3);
        if (ret < 0)
        {
   			printk("zhoujun_gt915  Failed to read Sensor_ID\n");
            GTP_ERROR("Failed to read Sensor_ID, using DEFAULT config!");
            sensor_id = 0;
            if (cfg_info_len[0] != 0)
            {
                send_cfg_buf[0][0] = 0x00;      // RESET Config Version
            }
        }
        else
        {
            sensor_id = opr_buf[2] & 0x07;
        }
    }
    GTP_DEBUG("Sensor_ID: %d", sensor_id);

    //printk("zhoujun_gt915  cfg_info_len[sensor_id] is %d\n",cfg_info_len[sensor_id]);	
    ts->gtp_cfg_len = cfg_info_len[sensor_id];
    printk("zhoujun_gt915  ts->gtp_cfg_len is  %d\n",ts->gtp_cfg_len);	//186 paras
    if (ts->gtp_cfg_len == 0)
    {
        GTP_ERROR("Sensor_ID(%d) matches with NULL CONFIG GROUP!NO Config Send! You need to check you header file CFG_GROUP section!", sensor_id);
		printk("zhoujun_gt915  sensor_id is error\n");
        return -1;
    }
    memset(&config[GTP_ADDR_LENGTH], 0, GTP_CONFIG_MAX_LENGTH);
    memcpy(&config[GTP_ADDR_LENGTH], send_cfg_buf[sensor_id], ts->gtp_cfg_len);

	#if GTP_CUSTOM_CFG	//no used
		config[RESOLUTION_LOC]     = (u8)GTP_MAX_WIDTH;
		config[RESOLUTION_LOC + 1] = (u8)(GTP_MAX_WIDTH>>8);
		config[RESOLUTION_LOC + 2] = (u8)GTP_MAX_HEIGHT;
		config[RESOLUTION_LOC + 3] = (u8)(GTP_MAX_HEIGHT>>8);
		
		if (GTP_INT_TRIGGER == 0)  //RISING
		{
		    config[TRIGGER_LOC] &= 0xfe; 
		}
		else if (GTP_INT_TRIGGER == 1)  //FALLING
		{
		    config[TRIGGER_LOC] |= 0x01;
		}
	#endif  // GTP_CUSTOM_CFG
    
    check_sum = 0;
    for (i = GTP_ADDR_LENGTH; i < ts->gtp_cfg_len; i++)
    {
        check_sum += config[i];
    }
    config[ts->gtp_cfg_len] = (~check_sum) + 1;
   // printk("zhoujun_gt915 stop stop stop!!!!\n");
#else // DRIVER NOT SEND CONFIG
    ts->gtp_cfg_len = GTP_CONFIG_MAX_LENGTH;
    ret = gtp_i2c_read(ts->client, config, ts->gtp_cfg_len + GTP_ADDR_LENGTH);
    if (ret < 0)
    {
        GTP_ERROR("Read Config Failed, Using DEFAULT Resolution & INT Trigger!");
        ts->abs_x_max = GTP_MAX_WIDTH;
        ts->abs_y_max = GTP_MAX_HEIGHT;
        ts->int_trigger_type = GTP_INT_TRIGGER;
    }
#endif // GTP_DRIVER_SEND_CFG

    GTP_DEBUG_FUNC();
    if ((ts->abs_x_max == 0) && (ts->abs_y_max == 0))
    {
        ts->abs_x_max = (config[RESOLUTION_LOC + 1] << 8) + config[RESOLUTION_LOC];
        ts->abs_y_max = (config[RESOLUTION_LOC + 3] << 8) + config[RESOLUTION_LOC + 2];
        ts->int_trigger_type = (config[TRIGGER_LOC]) & 0x03; 
    }
 	printk("zhoujun_gt915 gtp_send_cfg start...\n");
    	ret = gtp_send_cfg(client);	//delete by jun.zhou
 	printk("zhoujun_gt915 stop stop stop!\n");
    if (ret < 0)
    {
 		printk("gtp_send_cfg  error!\n");
        GTP_ERROR("Send config error.");
    }
    GTP_DEBUG("X_MAX = %d, Y_MAX = %d, TRIGGER = 0x%02x",
        ts->abs_x_max,ts->abs_y_max,ts->int_trigger_type);

	kfree(ts);  //jun.zhou add release [struct goodix_ts_data *ts]
    msleep(10);
    return 0;
}

static s8 gtp_i2c_test(struct i2c_client *client)
{

    u8 retry = 0;
    s8 ret = -1;
    u32 hw_info = 0;
	printk("zhoujun_gt915 : [gtp_i2c_test] fun is running \n");
    GTP_DEBUG_FUNC();

    while (retry++ < 1)
    {
//GTP_REG_HW_INFO = 0x4220
        ret = i2c_read_bytes(client, GTP_REG_HW_INFO, (u8 *)&hw_info, sizeof(hw_info));
		printk("zhoujun_gt915 : [gtp_i2c_test_retry: %d ]\n",retry);
		printk("zhoujun_gt915 : [hw_info] [ret] = %x %d\n",hw_info,ret);
        if ((!ret) && (hw_info == 0x00900600))              //20121212
        {
			printk("zhoujun_gt915 : [hw_info return]\n");
            return ret;
        }
		printk("GTP_REG_HW_INFO : %08X", hw_info);
        GTP_ERROR("GTP_REG_HW_INFO : %08X", hw_info);
        GTP_ERROR("GTP i2c test failed time %d.", retry);
        msleep(10);
    }

    return -1;
}



/*******************************************************
Function:
    Set INT pin  as input for FW sync.

Note:
  If the INT is high, It means there is pull up resistor attached on the INT pin.
  Pull low the INT pin manaully for FW sync.
*******************************************************/
void gtp_int_sync(s32 ms)
{
    GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
    msleep(ms);
    GTP_GPIO_AS_INT(GTP_INT_PORT);
}

void gtp_reset_guitar(struct i2c_client *client, s32 ms)
{
    GTP_INFO("GTP RESET!\n");
    GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);
    msleep(ms);
    GTP_GPIO_OUTPUT(GTP_INT_PORT, client->addr == 0x14);

    msleep(2);
    GTP_GPIO_OUTPUT(GTP_RST_PORT, 1);

    msleep(6);                      //must >= 6ms
    gtp_int_sync(50);

#if GTP_ESD_PROTECT
    gtp_init_ext_watchdog(i2c_client_point);
#endif
}

static int tpd_power_on(struct i2c_client *client)
{
    int ret = 0;
    int reset_count = 0;
	printk("zhoujun_gt915 : [tpd_power_on] start reset pin=%d\n", GTP_RST_PORT);
reset_proc:
    // RST INT pulldown until power on accomplished
    GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);   
    GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
#ifdef MT6573
    // power on CTP
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif
#ifdef MT6575
    //power on, need confirm with SA
    hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
    hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "TP");
#endif
#ifdef MT6577
    //power on, need confirm with SA
    hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
    hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "TP");
#endif
#ifdef MT6589
hwPowerOn(MT65XX_POWER_LDO_VGP5, VOL_1800, "TP");
mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
msleep(100);
#endif
	printk("zhoujun_gt915 : [gtp_reset_guitar] start \n");
    gtp_reset_guitar(client, 20);

	printk("zhoujun_gt915 : [gtp_i2c_test] start \n");

    ret = gtp_i2c_test(client);
    if(ret < 0)
	return ret;
    if (ret < 0)
    {
		printk("zhoujun_gt915 : [I2C communication ERROR] \n");
        GTP_ERROR("I2C communication ERROR!");

        if (reset_count < TPD_MAX_RESET_COUNT)
        {
            reset_count++;
            goto reset_proc;
        }
    }
	printk("zhoujun_gt915 : [gtp_i2c_test] ok! \n");
    return ret;
}

static s32 tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    s32 err = 0;
    s32 ret = 0;

    u16 version_info;
#ifdef GTP_HAVE_TOUCH_KEY
    s32 idx = 0;
#endif
#ifdef TPD_PROXIMITY
    struct hwmsen_object obj_ps;
#endif
	printk("zhoujun_gt915 : [tpd_i2c_probe] start \n");
    i2c_client_point = client;
    ret = tpd_power_on(client);

    if (ret < 0)
    {
		printk("zhoujun_gt915 [I2C communication ERROR] %d",__LINE__);
        GTP_ERROR("I2C communication ERROR!");
	goto out_err;
    }
	printk("zhoujun_gt915 : [tpd_power_on] ok \n"); 

#if GTP_FW_DOWNLOAD                         //20121212
    ret = gup_init_fw_proc(client);

    if (ret < 0)
    {
		printk("zhoujun_gt915 [Create fw download thread error.] %d",__LINE__);
        GTP_ERROR("Create fw download thread error.");
	goto out_err;
    }

#endif

#if GTP_AUTO_UPDATE
    ret = gup_init_update_proc(client);

    if (ret < 0)
    {
		printk("zhoujun_gt915 [Create update thread error.] %d",__LINE__);
        GTP_ERROR("Create update thread error.");
	goto out_err;
    }
#endif

#ifdef VELOCITY_CUSTOM

    if ((err = misc_register(&tpd_misc_device)))
    {
		printk("zhoujun_gt915 [mtk_tpd: tpd_misc_device register failed] %d",__LINE__);
        printk("mtk_tpd: tpd_misc_device register failed\n");
    }

#endif
	printk("zhoujun_gt915  [gtp_init_panel] will start\n");
    ret = gtp_init_panel(client);
	printk("GTP init panel failed.] %d\n",__LINE__);
    if (ret < 0)
    {
		printk("GTP init panel failed.] %d",__LINE__);
        GTP_ERROR("GTP init panel failed.");
	goto out_err;
    }

    ret = gtp_read_version(client, &version_info);
    if (ret < 0)
    {
		printk("Read version failed.] %d",__LINE__);
        GTP_ERROR("Read version failed.");
    }
    // Create proc file system
    gt91xx_config_proc = create_proc_entry(GT91XX_CONFIG_PROC_FILE, 0666, NULL);

    if (gt91xx_config_proc == NULL)
    {
		printk("create_proc_entry failed.] %d",__LINE__);
        GTP_ERROR("create_proc_entry %s failed\n", GT91XX_CONFIG_PROC_FILE);
    }
    else
    {
        gt91xx_config_proc->read_proc = gt91xx_config_read_proc;
        gt91xx_config_proc->write_proc = gt91xx_config_write_proc;
    }

#if GTP_CREATE_WR_NODE
    init_wr_node(client);
#endif

    thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
    if (IS_ERR(thread))
    {
        err = PTR_ERR(thread);
		printk("failed to create kernel thread:] %d",__LINE__);
        GTP_INFO(TPD_DEVICE " failed to create kernel thread: %d\n", err);
    }

#if GTP_HAVE_TOUCH_KEY

    for (idx = 0; idx < GTP_MAX_KEY_NUM; idx++)
    {
        input_set_capability(tpd->dev, EV_KEY, touch_key_array[idx]);
    }
#endif

    // set INT mode
    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);

    msleep(50);

    mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
    mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);

    if (!int_type)
    {
        mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_POLARITY_HIGH, tpd_eint_interrupt_handler, 1);
    }
    else
    {
        mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_POLARITY_LOW, tpd_eint_interrupt_handler, 1);
    }

    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
#ifdef TPD_PROXIMITY
    //obj_ps.self = cm3623_obj;
    obj_ps.polling = 0;         //0--interrupt mode;1--polling mode;
    obj_ps.sensor_operate = tpd_ps_operate;

    if ((err = hwmsen_attach(ID_PROXIMITY, &obj_ps)))
    {
        GTP_ERROR("hwmsen attach fail, return:%d.", err);
    }

#endif
#if GTP_ESD_PROTECT
    INIT_DELAYED_WORK(&gtp_esd_check_work, gtp_esd_check_func);
    gtp_esd_check_workqueue = create_workqueue("gtp_esd_check");
    queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, TPD_ESD_CHECK_CIRCLE);
#endif

    tpd_load_status = 1;	//i2c probe is ok
	printk("zhoujun_gt915 : i2c_probe is ok, status will set 1\n");
    return 0;
out_err:
    return -1;
}

static void tpd_eint_interrupt_handler(void)
{
    TPD_DEBUG_PRINT_INT;
    tpd_flag = 1;
    wake_up_interruptible(&waiter);
}
static int tpd_i2c_remove(struct i2c_client *client)
{
#if GTP_CREATE_WR_NODE
    uninit_wr_node();
#endif

#if GTP_ESD_PROTECT
    destroy_workqueue(gtp_esd_check_workqueue);
#endif

    return 0;
}

#if GTP_ESD_PROTECT
static s32 gtp_init_ext_watchdog(struct i2c_client *client)
{
    u8 opr_buffer[4] = {0x80, 0x40, 0xAA, 0xAA};

    return gtp_i2c_write(client, opr_buffer, 4);

}
static void force_reset_guitar(void)
{
    s32 i;
    s32 ret;

    GTP_INFO("force_reset_guitar\n");

    //Power off TP
#if (defined(MT6575) || defined(mt6577))
    hwPowerDown(MT65XX_POWER_LDO_VGP2, "TP");
    msleep(30);
    //Power on TP
    // RST INT pulldown until power on accomplished
    GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);   
    GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
    hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
    msleep(30);
#else
    //Power off TP
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
    msleep(30);
    //Power on TP
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
    msleep(30);
#endif

    for (i = 0; i < 5; i++)
    {
        //Reset Guitar
        gtp_reset_guitar(i2c_client_point, 20);
    
        //Send config
        ret = gtp_send_cfg(i2c_client_point);

        if (ret < 0)
        {
            continue;
        }

        break;
    }

}
static void gtp_esd_check_func(struct work_struct *work)
{
    int i;
    int ret = -1;
    u8 test[4] = {0x80, 0x40};

    if (tpd_halt)
    {
        return;
    }

    for (i = 0; i < 3; i++)
    {
        ret = gtp_i2c_read(i2c_client_point, test, 4);

        GTP_DEBUG("0x8040 = 0x%02X, 0x8041 = 0x%02X", test[2], test[3]);
        if (ret < 0)
        {
            // IC works abnormally..
            continue;
        }
        else 
        {
            if ((test[2] == 0xAA) || (test[3] != 0xAA))
            {
                // IC works abnormally...
                break;
            }
            else
            {
                // IC works normally, Write 0x8040 0xAA
                test[2] = 0xAA; 
                gtp_i2c_write(i2c_client_point, test, 3);
                break;
            }
        }
    }

    if (i >= 3)
    {
        GTP_INFO("IC Works ABNORMALLY! Resetting Guitar....");
        force_reset_guitar();
    }

    if (!tpd_halt)
    {
        queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, TPD_ESD_CHECK_CIRCLE);
    }

    return;
}
#endif

static void tpd_down(s32 x, s32 y, s32 size, s32 id)
{
    if ((!size) && (!id))
    {
        input_report_abs(tpd->dev, ABS_MT_PRESSURE, 100);
        input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 100);
    }
    else
    {
        input_report_abs(tpd->dev, ABS_MT_PRESSURE, size);
        input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, size);
        /* track id Start 0 */
        input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id);
    }

    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    input_mt_sync(tpd->dev);
    TPD_EM_PRINT(x, y, x, y, id, 1);

#if (defined(MT6575)||defined(MT6577))

    if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
    {
        tpd_button(x, y, 1);
    }

#endif
}

static void tpd_up(s32 x, s32 y, s32 id)
{
    input_report_abs(tpd->dev, ABS_MT_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
    input_mt_sync(tpd->dev);
    TPD_EM_PRINT(x, y, x, y, id, 0);

#if (defined(MT6575)||defined(MT6577))

    if (FACTORY_BOOT == get_boot_mode() || RECOVERY_BOOT == get_boot_mode())
    {
        tpd_button(x, y, 0);
    }

#endif
}

static int touch_event_handler(void *unused)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
    u8  end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0};
    u8  point_data[2 + 1 + 8 * GTP_MAX_TOUCH + 1] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};
    u8  touch_num = 0;
    u8  finger = 0;
    static u8 pre_touch = 0;
    static u8 pre_key = 0;
    u8  key_value = 0;
    u8 *coor_data = NULL;
    s32 input_x = 0;
    s32 input_y = 0;
    s32 input_w = 0;
    s32 id = 0;
    s32 i  = 0;
    s32 ret = -1;
    struct goodix_ts_data *ts;
#ifdef TPD_PROXIMITY
    s32 err = 0;
    hwm_sensor_data sensor_data;
    u8 proximity_status;
#endif

#if GTP_SLIDING_WAKEUP
    u8 doze_buf[3] = {0x81, 0x4B};
#endif

#if GTP_CHARGER_SWITCH
    u32 chr_status = 0;
    u8 chr_cmd[3] = {0x80, 0x40};
    static u8 chr_pluggedin = 0;
#endif

    sched_setscheduler(current, SCHED_RR, &param);
    ts = i2c_get_clientdata(i2c_client_point);
    do
    {
        set_current_state(TASK_INTERRUPTIBLE);

        while (tpd_halt)
        {
        #if GTP_SLIDING_WAKEUP
            if (doze_enabled)
            {
                break;
            }
        #endif
            tpd_flag = 0;
            msleep(20);
        }

        wait_event_interruptible(waiter, tpd_flag != 0);
        tpd_flag = 0;
        TPD_DEBUG_SET_TIME;
        set_current_state(TASK_RUNNING);

#if GTP_CHARGER_SWITCH
    #ifdef MT6573
        chr_status = *(volatile u32 *)CHR_CON0;
        chr_status &= (1 << 13);
    #else
        chr_status = upmu_is_chr_det();
    #endif

        if (chr_status)     // charger plugged in
        {
            if (!chr_pluggedin)
            {
                chr_cmd[2] = 6;
                ret = gtp_i2c_write(i2c_client_point, chr_cmd, 3);
                if (ret > 0)
                {
                    GTP_INFO("Update status for Charger Plugged in");
                }
                chr_pluggedin = 1;
            }
        }
        else            // charger plugged out
        {
            if (chr_pluggedin)
            {
                chr_cmd[2] = 7;
                ret = gtp_i2c_write(i2c_client_point, chr_cmd, 3);
                if (ret > 0)
                {
                    GTP_INFO("Update status for Charger Plugged out");
                }
                chr_pluggedin = 0;
            }
        }
#endif

#if GTP_SLIDING_WAKEUP

        if (doze_enabled)
        {
            ret = gtp_i2c_read(i2c_client_point, doze_buf, 3);
            GTP_DEBUG("0x814B = 0x%02X", doze_buf[2]);
            if (ret > 0)
            {               
                if (doze_buf[2] == 0xAA)
                {
                    GTP_INFO("Sliding To Light up the screen!");
                    input_report_key(ts->input_dev, KEY_POWER, 1);
                    input_report_key(ts->input_dev, KEY_POWER, 0);
                    doze_enabled = 0;
                }
            }

            goto exit_work_func;
        }
#endif

        ret = gtp_i2c_read(i2c_client_point, point_data, 12);

        if (ret < 0)
        {
            GTP_ERROR("I2C transfer error. errno:%d\n ", ret);
            goto exit_work_func;
        }

        finger = point_data[GTP_ADDR_LENGTH];

        if ((finger & 0x80) == 0)
        {
            goto exit_work_func;
        }
        

#ifdef TPD_PROXIMITY

        if (tpd_proximity_flag == 1)
        {
            proximity_status = point_data[GTP_ADDR_LENGTH];
            GTP_DEBUG("REG INDEX[0x814E]:0x%02X\n", proximity_status);

            if (proximity_status & 0x60)                //proximity or large touch detect,enable hwm_sensor.
            {
                tpd_proximity_detect = 0;
                //sensor_data.values[0] = 0;
            }
            else
            {
                tpd_proximity_detect = 1;
                //sensor_data.values[0] = 1;
            }

            //get raw data
            GTP_DEBUG(" ps change\n");
            GTP_DEBUG("PROXIMITY STATUS:0x%02X\n", tpd_proximity_detect);
            //map and store data to hwm_sensor_data
            sensor_data.values[0] = tpd_get_ps_value();
            sensor_data.value_divide = 1;
            sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
            //report to the up-layer
            ret = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data);

            if (ret)
            {
                GTP_ERROR("Call hwmsen_get_interrupt_data fail = %d\n", err);
            }
        }

#endif

        touch_num = finger & 0x0f;

        if (touch_num > GTP_MAX_TOUCH)
        {
            goto exit_work_func;
        }

        if (touch_num > 1)
        {
            u8 buf[8 * GTP_MAX_TOUCH] = {(GTP_READ_COOR_ADDR + 10) >> 8, (GTP_READ_COOR_ADDR + 10) & 0xff};

            ret = gtp_i2c_read(i2c_client_point, buf, 2 + 8 * (touch_num - 1));
            memcpy(&point_data[12], &buf[2], 8 * (touch_num - 1));
        }

#if GTP_HAVE_TOUCH_KEY
        key_value = point_data[3 + 8 * touch_num];

        if (key_value || pre_key)
        {
            for (i = 0; i < GTP_MAX_KEY_NUM; i++)
            {
                input_report_key(tpd->dev, touch_key_array[i], key_value & (0x01 << i));
            }

            touch_num = 0;
            pre_touch = 0;
        }

#endif
        pre_key = key_value;

        GTP_DEBUG("pre_touch:%02x, finger:%02x.", pre_touch, finger);

        if (touch_num)
        {
            for (i = 0; i < touch_num; i++)
            {
                coor_data = &point_data[i * 8 + 3];

                id = coor_data[0]&0x0F;
                input_x  = coor_data[1] | coor_data[2] << 8;
                input_y  = coor_data[3] | coor_data[4] << 8;
                input_w  = coor_data[5] | coor_data[6] << 8;

                input_x = TPD_WARP_X(abs_x_max, input_x);
                input_y = TPD_WARP_Y(abs_y_max, input_y);
                tpd_down(input_x, input_y, input_w, id);
		printk("input_x = %d, input_y = %d\n", input_x, input_y);
            }
        }
        else if (pre_touch)
        {
            GTP_DEBUG("Touch Release!");
            tpd_up(0, 0, 0);
        }

        pre_touch = touch_num;
        input_report_key(tpd->dev, BTN_TOUCH, (touch_num || key_value));

        if (tpd != NULL && tpd->dev != NULL)
        {
            input_sync(tpd->dev);
        }

exit_work_func:

        if (!gtp_rawdiff_mode)
        {
            ret = gtp_i2c_write(i2c_client_point, end_cmd, 3);

            if (ret < 0)
            {
                GTP_INFO("I2C write end_cmd  error!");
            }
        }

    }
    while (!kthread_should_stop());

    return 0;
}

static int tpd_local_init(void)
{
//jun.zhou
	printk("zhoujun_gt915 : [tpd_local_init]\n");
    if (i2c_add_driver(&tpd_i2c_driver) != 0)
    {
		printk("zhoujun_gt915 : [i2c_add_driver] err \n");
        GTP_INFO("unable to add i2c driver.\n");
        return -1;
    }

    if (tpd_load_status == 0) //if(tpd_load_status == 0) // disable auto load touch driver for linux3.0 porting
    {
        GTP_INFO("add error touch panel driver.\n");
		printk("zhoujun_gt915 : tpd_load_status is error and will delete the driver\n");
        i2c_del_driver(&tpd_i2c_driver);
        return -1;
    }

#ifdef TPD_HAVE_BUTTON
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT * 4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT * 4);
#endif

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_def_calmat_local, 8 * 4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8 * 4);
#endif

    // set vendor string
    tpd->dev->id.vendor = 0x00;
    tpd->dev->id.product = tpd_info.pid;
    tpd->dev->id.version = tpd_info.vid;

    GTP_INFO("end %s, %d\n", __FUNCTION__, __LINE__);
    tpd_type_cap = 1;

    return 0;
}

#if GTP_SLIDING_WAKEUP
static s8 gtp_enter_doze(struct i2c_client *client)
{
    s8 ret = -1;
    s8 retry = 0;
    u8 i2c_control_buf[3] = {(u8)(GTP_REG_SLEEP >> 8), (u8)GTP_REG_SLEEP, 8};

    GTP_DEBUG_FUNC();

    gtp_reset_guitar(client, 20);
    msleep(50);         // wait for INT port transferred into FLOATING INPUT STATUS
    GTP_DEBUG("entering doze mode...");
    while(retry++ < 5)
    {
        ret = gtp_i2c_write(client, i2c_control_buf, 3);
        if (ret > 0)
        {
            doze_enabled = 1;
            GTP_DEBUG("GTP has been working in doze mode!");
            return ret;
        }
        msleep(10);
    }
    GTP_ERROR("GTP send doze cmd failed.");
    return ret;
}

#else
/*******************************************************
Function:
    Eter sleep function.

Input:
    client:i2c_client.

Output:
    Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_enter_sleep(struct i2c_client *client)
{
    s8 ret = -1;
    s8 retry = 0;
    u8 i2c_control_buf[3] = {(u8)(GTP_REG_SLEEP >> 8), (u8)GTP_REG_SLEEP, 5};
#if GTP_POWER_CTRL_SLEEP
    hwPowerDown(MT65XX_POWER_LDO_VGP2, "TP");
    hwPowerDown(MT65XX_POWER_LDO_VGP, "TP");
    GTP_INFO("GTP enter sleep!");
    return 0;
#else
    GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
    msleep(5);

    while (retry++ < 5)
    {
        ret = gtp_i2c_write(client, i2c_control_buf, 3);

        if (ret > 0)
        {
            GTP_INFO("GTP enter sleep!");
                
            return ret;
        }

        msleep(10);
    }

#endif
    GTP_ERROR("GTP send sleep cmd failed.");
    return ret;
}
#endif

/*******************************************************
Function:
    Wakeup from sleep mode Function.

Input:
    client:i2c_client.

Output:
    Executive outcomes.0--success,non-0--fail.
*******************************************************/
static s8 gtp_wakeup_sleep(struct i2c_client *client)
{
    u8 retry = 0;
    s8 ret = -1;


    GTP_INFO("GTP wakeup begin.");
    
    
#if GTP_POWER_CTRL_SLEEP
    while (retry++ < 5)
    {
        ret = tpd_power_on(client);

        if (ret < 0)
        {
            GTP_ERROR("I2C Power on ERROR!");
        }

    #if GTP_DRIVER_SEND_CFG
        ret = gtp_send_cfg(client);

        if (ret > 0)
        {
            GTP_DEBUG("Wakeup sleep send config success.");
            return ret;
        }
    #else
        GTP_DEBUG("Wake up Ic successfully");
        return 1; 
    #endif
    }

#else

    while (retry++ < 10)
    {
        GTP_GPIO_OUTPUT(GTP_INT_PORT, 1);
        msleep(5);
        
        ret = gtp_i2c_test(client);

        if (ret >= 0)
        {
            GTP_INFO("GTP wakeup sleep.");
            gtp_int_sync(25);
        #if GTP_SLIDING_WAKEUP
            gtp_init_ext_watchdog(client);
        #endif    
            return ret;
        }

        gtp_reset_guitar(client, 20);

    }

#endif

    GTP_ERROR("GTP wakeup sleep failed.");
    return ret;
}
/* Function to manage low power suspend */
static void tpd_suspend(struct early_suspend *h)
{
    s32 ret = -1;

#if GTP_ESD_PROTECT
    cancel_delayed_work_sync(&gtp_esd_check_work);
#endif

#ifdef TPD_PROXIMITY

    if (tpd_proximity_flag == 1)
    {
        return ;
    }

#endif

    tpd_halt = 1;

#if GTP_SLIDING_WAKEUP
    ret = gtp_enter_doze(i2c_client_point);
#else
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    ret = gtp_enter_sleep(i2c_client_point);
#endif
    if (ret < 0)
    {
        GTP_ERROR("GTP early suspend failed.");
    }
    // to avoid waking up while not sleeping, delay 48 + 10ms to ensure reliability 
    msleep(58);
}

/* Function to manage power-on resume */
static void tpd_resume(struct early_suspend *h)
{
    s32 ret = -1;
#ifdef TPD_PROXIMITY

    if (tpd_proximity_flag == 1)
    {
        return ;
    }

#endif

    ret = gtp_wakeup_sleep(i2c_client_point);

    if (ret < 0)
    {
        GTP_ERROR("GTP later resume failed.");
    }

    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
#if GTP_SLIDING_WAKEUP
    doze_enabled = 0;
#endif
    tpd_halt = 0;
    
#if GTP_ESD_PROTECT
    queue_delayed_work(gtp_esd_check_workqueue, &gtp_esd_check_work, TPD_ESD_CHECK_CIRCLE);
#endif

}

static struct tpd_driver_t tpd_device_driver =
{
    .tpd_device_name = "gt915",
    .tpd_local_init = tpd_local_init,
    .suspend = tpd_suspend,
    .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
    .tpd_have_button = 1,
#else
    .tpd_have_button = 0,
#endif
};

/* called when loaded into kernel */
static int __init tpd_driver_init(void)
{
	GTP_INFO("MediaTek gt91xx touch panel driver init\n");
	i2c_register_board_info(1, &i2c_tpd, 1);

	if (tpd_driver_add(&tpd_device_driver) < 0)
	{
		GTP_INFO("add generic driver failed\n");
		printk("zhoujun_gt915 : [tpd_device_driver] is failed\n");
	}

	printk("zhoujun_gt915 : [%s] is ok\n",__FUNCTION__);
	return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void)
{
    GTP_INFO("MediaTek gt91xx touch panel driver exit\n");
    //input_unregister_device(tpd->dev);
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

