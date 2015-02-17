
#define XHCI_MTK_TEST_MAJOR 235
#define DEVICE_NAME "xhci_mtk_test"

/* for auto test struct defs */

typedef enum 
{
	USB_TX = 0,
	USB_RX
} USB_DIR;

typedef enum 
{
	Ctrol_Transfer = 0,
	Bulk_Random,
	Test_Loopback,
	Test_End
} USB_TEST_CASE;

/* CTRL, BULK, INTR, ISO endpoint */
typedef enum
{
	USB_CTRL = 0,
	USB_BULK,
	USB_INTR,
	USB_ISO
}USB_TRANSFER_TYPE;

typedef enum
{
    SPEED_HIGH = 0,
    SPEED_FULL
}USB_SPEED;

typedef enum
{
    BUSY = 0,
    READY,
    END
}state;

typedef enum
{
    TRANSFER_SUCCESS = 0,
    TRANSFER_FAIL
}status;

typedef struct
{
    unsigned char    type;
    unsigned char    speed;
    unsigned int     length;
    unsigned short   maxp;
    unsigned char    state;
    unsigned char    status;
}USB_TRANSFER;


typedef struct
{
	unsigned short header;
	unsigned char  testcase;
	USB_TRANSFER   transfer;
    unsigned short end;
}USB_MSG;


