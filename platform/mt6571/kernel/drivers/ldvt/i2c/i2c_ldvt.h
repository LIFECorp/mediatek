#ifndef I2C_LDVT_H
#define I2C_LDVT_H

#include <linux/ioctl.h>

/* test cases */
#define I2C_UVVF_HS_FIFO_1P5MHZ		_IO('i', 1)
#define I2C_UVVF_HS_FIFO_2MHZ		_IO('i', 2)
#define I2C_UVVF_FS_DMA_RS_DIR_200KHZ	_IO('i', 3)
#define I2C_UVVF_FS_DMA_RS_DIR_400KHZ	_IO('i', 4)
#define I2C_UVVF_CLK_EXT_INT		_IO('i', 5)
#define I2C_UVVF_CLK_REPEAT_RW		_IO('i', 6)
#define I2C_UVVF_CLK_EXT		_IO('i', 7)
#define I2C_UVVF_CLK_REPEAT_EXT		_IO('i', 8)


#if 0
#define I2C_UVVF_HS_FIFO_1P5MHZ		0x0731
#define I2C_UVVF_HS_FIFO_2MHZ		0x0732
#define I2C_UVVF_FS_DMA_RS_DIR_200KHZ	0x0733
#define I2C_UVVF_FS_DMA_RS_DIR_400KHZ	0x0734
#define I2C_UVVF_CLK_EXT_INT		0x0735
#define I2C_UVVF_CLK_REPEAT_RW		0x0736
#define I2C_UVVF_CLK_EXT		0x0737
#define I2C_UVVF_CLK_REPEAT_EXT		0x0738
#endif

#endif /* I2C_LDVT_H */
