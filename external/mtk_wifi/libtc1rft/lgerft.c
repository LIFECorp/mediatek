/*
 * This is sample code for LGE WLAN AT command and Hidden Menu
 */

#include <stdio.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <cutils/log.h>
#include <cutils/misc.h> /* load_file() */
#include <libnvram.h>
#include <Custom_NvRam_LID.h>
#include <CFG_Wifi_File.h>

#include <iwlib.h>
#include "libwifitest.h"
#include "lgerft.h"

extern int init_module(void *, unsigned long, const char *);

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#define LGE_BSP_BUILD
#if defined(LGE_BSP_BUILD)
#define USE_LGE_NVRAM
#else
#define USE_MTK_NVRAM
#endif

/* NVRAM Configuration */
#if defined(USE_LGE_NVRAM)
extern bool LGE_FacWriteWifiMacAddr(unsigned char *wifiMacAddr, bool needFlashProgram);

#endif



#ifndef BIT
#define BIT(n)          ((uint32_t) 1 << (n))
#define BITS(m,n)       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

#define MAC_ADDR_LEN                            6

#define IS_BMCAST_MAC_ADDR(_pucDestAddr)            \
        ((bool) ( ((uint8_t *)(_pucDestAddr))[0] & BIT(0) ))

#define EQUAL_MAC_ADDR(_pucDestAddr, _pucSrcAddr)   \
        (!memcmp(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN))

/* Debug print format string for the MAC Address */
#define MACSTR      "%02x:%02x:%02x:%02x:%02x:%02x"

/* Debug print argument for the MAC Address */
#define MAC2STR(a)  ((uint8_t *)a)[0], ((uint8_t *)a)[1], ((uint8_t *)a)[2], \
    ((uint8_t *)a)[3], ((uint8_t *)a)[4], ((uint8_t *)a)[5]

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#ifndef WIFI_DRIVER_MODULE_PATH
#define WIFI_DRIVER_MODULE_PATH     "/system/lib/modules/wlan.ko"
#endif
#ifndef WIFI_DRIVER_MODULE_NAME
#define WIFI_DRIVER_MODULE_NAME     "wlan"
#endif
#ifndef WIFI_DRIVER_MODULE_ARG
#define WIFI_DRIVER_MODULE_ARG      ""
#endif

#ifndef WIFI_IF_NAME
#define WIFI_IF_NAME                "wlan0"
#endif

static const char DRIVER_PROP_NAME[]    = "wlan.driver.status";
static const char DRIVER_MODULE_NAME[]  = WIFI_DRIVER_MODULE_NAME;
//static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME " ";
static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME;
static const char DRIVER_MODULE_PATH[]  = WIFI_DRIVER_MODULE_PATH;
static const char DRIVER_MODULE_ARG[]   = WIFI_DRIVER_MODULE_ARG;
static const char MODULE_FILE[]         = "/proc/modules";
static const char WIFI_PROP_NAME[]      = "WIFI.SSID";

#define IOCTL_SET_STRUCT                (SIOCIWFIRSTPRIV + 8)
#define IOCTL_GET_STRUCT                (SIOCIWFIRSTPRIV + 9)
#define IOCTL_SET_STRUCT_FOR_EM         (SIOCIWFIRSTPRIV + 11)

#define PRIV_CMD_OID                    15
#define PRIV_CMD_SW_CTRL		20

/* RF Test specific OIDs */
#define OID_CUSTOM_TEST_MODE                            0xFFA0C901
#define OID_CUSTOM_ABORT_TEST_MODE                      0xFFA0C906
#define OID_CUSTOM_MTK_WIFI_TEST                        0xFFA0C911
#define OID_CUSTOM_SW_CTRL 				0xFFA0C911

/* command mask */
#define TEST_FUNC_IDX_MASK              BITS(0,7)
#define TEST_SET_CMD_OFFSET_MASK        BITS(16,31)
#define TEST_SET_CMD_OFFSET             16

/* RF Test Properties */
#define RF_AT_PARAM_RATE_MCS_MASK   BIT(31)
#define RF_AT_PARAM_RATE_MASK       BITS(0,7)
#define RF_AT_PARAM_RATE_1M         0
#define RF_AT_PARAM_RATE_2M         1
#define RF_AT_PARAM_RATE_5_5M       2
#define RF_AT_PARAM_RATE_11M        3
#define RF_AT_PARAM_RATE_6M         4
#define RF_AT_PARAM_RATE_9M         5
#define RF_AT_PARAM_RATE_12M        6
#define RF_AT_PARAM_RATE_18M        7
#define RF_AT_PARAM_RATE_24M        8
#define RF_AT_PARAM_RATE_36M        9
#define RF_AT_PARAM_RATE_48M        10
#define RF_AT_PARAM_RATE_54M        11

#define NULL_MAC_ADDR               {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
typedef struct _NDIS_TRANSPORT_STRUCT {
    uint32_t    ndisOidCmd;
    uint32_t    inNdisOidlength;
    uint32_t    outNdisOidLength;
    uint8_t     ndisOidContent[16];
} NDIS_TRANSPORT_STRUCT, *P_NDIS_TRANSPORT_STRUCT;

typedef struct _PARAM_MTK_WIFI_TEST_STRUC_T {
    uint32_t u4FuncIndex;
    uint32_t u4FuncData;
} PARAM_MTK_WIFI_TEST_STRUC_T, *P_PARAM_MTK_WIFI_TEST_STRUC_T;
#if 0
typedef enum _ENUM_RF_AT_FUNCID_T {

    RF_AT_FUNCID_VERSION = 0,
    RF_AT_FUNCID_COMMAND,
    RF_AT_FUNCID_POWER,
    RF_AT_FUNCID_RATE,
    RF_AT_FUNCID_PREAMBLE,
    RF_AT_FUNCID_ANTENNA,
    RF_AT_FUNCID_PKTLEN,
    RF_AT_FUNCID_PKTCNT,
    RF_AT_FUNCID_PKTINTERVAL,
    RF_AT_FUNCID_TEMP_COMPEN,
    RF_AT_FUNCID_TXOPLIMIT,
    RF_AT_FUNCID_ACKPOLICY,
    RF_AT_FUNCID_PKTCONTENT,
    RF_AT_FUNCID_RETRYLIMIT,
    RF_AT_FUNCID_QUEUE,
    RF_AT_FUNCID_BANDWIDTH,
    RF_AT_FUNCID_GI,
    RF_AT_FUNCID_STBC,
    RF_AT_FUNCID_CHNL_FREQ,
    RF_AT_FUNCID_RIFS,
    RF_AT_FUNCID_TRSW_TYPE,
    RF_AT_FUNCID_RF_SX_SHUTDOWN,
    RF_AT_FUNCID_PLL_SHUTDOWN,
    RF_AT_FUNCID_SLOW_CLK_MODE,
    RF_AT_FUNCID_ADC_CLK_MODE,
    RF_AT_FUNCID_MEASURE_MODE,
    RF_AT_FUNCID_VOLT_COMPEN,
    RF_AT_FUNCID_DPD_TX_GAIN,
    RF_AT_FUNCID_DPD_MODE,
    RF_AT_FUNCID_TSSI_MODE,
    RF_AT_FUNCID_TX_GAIN_CODE,
    RF_AT_FUNCID_TX_PWR_MODE,

    /* Query command */
    RF_AT_FUNCID_TXED_COUNT = 32,
    RF_AT_FUNCID_TXOK_COUNT,
    RF_AT_FUNCID_RXOK_COUNT,
    RF_AT_FUNCID_RXERROR_COUNT,
    RF_AT_FUNCID_RESULT_INFO,
    RF_AT_FUNCID_TRX_IQ_RESULT,
    RF_AT_FUNCID_TSSI_RESULT,
    RF_AT_FUNCID_DPD_RESULT,
    RF_AT_FUNCID_RXV_DUMP,
    RF_AT_FUNCID_RX_PHY_STATIS,
    RF_AT_FUNCID_MEASURE_RESULT,
    RF_AT_FUNCID_TEMP_SENSOR,
    RF_AT_FUNCID_VOLT_SENSOR,
    RF_AT_FUNCID_READ_EFUSE,
    RF_AT_FUNCID_RX_RSSI,

    /* Set command */
    RF_AT_FUNCID_SET_DPD_RESULT = 64,
    RF_AT_FUNCID_SET_CW_MODE,
    RF_AT_FUNCID_SET_JAPAN_CH14_FILTER,
    RF_AT_FUNCID_WRITE_EFUSE,
    RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
    RF_AT_FUNCID_SET_MAC_SRC_ADDRESS,
    RF_AT_FUNCID_SET_RXOK_MATCH_RULE,

} ENUM_RF_AT_FUNCID_T;

typedef enum _ENUM_RF_AT_COMMAND_T {

    RF_AT_COMMAND_STOPTEST = 0,
    RF_AT_COMMAND_STARTTX,
    RF_AT_COMMAND_STARTRX,
    RF_AT_COMMAND_RESET,
    RF_AT_COMMAND_OUTPUT_POWER,     /* Payload */
    RF_AT_COMMAND_LO_LEAKAGE,       /* Local freq is renamed to Local leakage */
    RF_AT_COMMAND_CARRIER_SUPPR,    /* OFDM (LTF/STF), CCK (PI,PI/2) */
    RF_AT_COMMAND_TRX_IQ_CAL,
    RF_AT_COMMAND_TSSI_CAL,
    RF_AT_COMMAND_DPD_CAL,
    RF_AT_COMMAND_CW,
    RF_AT_COMMAND_NUM

} ENUM_RF_AT_COMMAND_T;

typedef enum _ENUM_RF_AT_PREAMBLE_T {

    RF_AT_PREAMBLE_NORMAL = 0,
    RF_AT_PREAMBLE_CCK_SHORT,
    RF_AT_PREAMBLE_11N_MM,
    RF_AT_PREAMBLE_11N_GF,
    RF_AT_PREAMBLE_NUM

} ENUM_RF_AT_PREAMBLE_T;

typedef enum _ENUM_RF_AT_BW_T {

    RF_AT_BW_20 = 0,
    RF_AT_BW_40,
    RF_AT_BW_U20,
    RF_AT_BW_L20,
    RF_AT_BW_NUM

} ENUM_RF_AT_BW_T, *P_ENUM_RF_AT_BW_T;


typedef enum _ENUM_RF_AT_RXOK_MATCH_RULE_T {

    RF_AT_RXOK_DISABLED = 0,
    RF_AT_RXOK_MATCH_RA_ONLY,
    RF_AT_RXOK_MATCH_TA_ONLY,
    RF_AT_RXOK_MATCH_RA_TA,
    RF_AT_RXOK_NUM

} ENUM_RF_AT_RXOK_MATCH_RULE_T, *P_ENUM_RF_AT_RXOK_MATCH_RULE_T;
#endif
/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
#if 0
static int wifi_rfkill_id = -1;
static char *wifi_rfkill_state_path = NULL;
#endif
static int skfd = -1;
static bool fgIsTesting = false;


/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to initial rfkill variables
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
#if 0
static int
wifi_init_rfkill(
    void
    )
{
    char path[64];
    char buf[16];
    int fd;
    int sz;
    int id;

    for (id = 0; ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            ALOGW("[%s] open(%s) failed: %s (%d)\n", __func__, path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= 4 && memcmp(buf, "wlan", 4) == 0) {
            wifi_rfkill_id = id;
            break;
        }
    }

    asprintf(&wifi_rfkill_state_path, "/sys/class/rfkill/rfkill%d/state",
            wifi_rfkill_id);
    return 0;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to turn on/off Wi-Fi interface via rfkill
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
#if 0
static int
wifi_set_power(
    int on
    )
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');

    ALOGD("[%s] %d", __func__, on);

    if (wifi_rfkill_id == -1) {
        if (wifi_init_rfkill()) {
            goto out;
        }
    }

    fd = open(wifi_rfkill_state_path, O_WRONLY);
    ALOGD("[%s] %s", __func__, wifi_rfkill_state_path);
    if (fd < 0) {
        ALOGE("open(%s) for write failed: %s (%d)",
                wifi_rfkill_state_path,
                strerror(errno),
                errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        ALOGE("write(%s) failed: %s (%d)",
                wifi_rfkill_state_path,
                strerror(errno),
                errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) {
        close(fd);
    }
    return ret;
}
#else

#define WIFI_POWER_PATH                 "/dev/wmtWifi"

int wifi_set_power(int enable) {
    int sz;
    int fd = -1;
    const char buffer = (enable ? '1' : '0');

    fd = open(WIFI_POWER_PATH, O_WRONLY);
    if (fd < 0) {
        ALOGE("Open \"%s\" failed", WIFI_POWER_PATH);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        ALOGE("Set \"%s\" [%c] failed", WIFI_POWER_PATH, buffer);
        goto out;
    }

out:
    if (fd >= 0) close(fd);
    return 0;
}

#endif

#if 0 //built-in

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to insert module
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
insmod(
    const char *filename,
    const char *args
    )
{
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);

    free(module);

    return ret;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to check if driver is loaded or not
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
check_driver_loaded(
    void
    )
{
    FILE *proc;
    char line[sizeof(DRIVER_MODULE_TAG)+10];

    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        ALOGW("Could not open %s: %s", MODULE_FILE, strerror(errno));
        return 0;
    }

    while ((fgets(line, sizeof(line), proc)) != NULL) {
        if (strncmp(line, DRIVER_MODULE_TAG, strlen(DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            return 1;
        }
    }

    fclose(proc);
    return 0;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to extract interface name
 *
 * @param   name    pointer to name buffer
 *          nsize   size of name buffer
 *          buf     current position in buffer
 * @return
 */
/*----------------------------------------------------------------------------*/
static inline char *
iw_get_ifname(
    char *name,
    int	  nsize,
    char *buf
    )
{
    char *end;

    /* Skip leading spaces */
    while(isspace(*buf))
        buf++;

    end = strrchr(buf, ':');

    /* Not found ??? To big ??? */
    if((end == NULL) || (((end - buf) + 1) > nsize))
        return(NULL);

    /* Copy */
    memcpy(name, buf, (end - buf));
    name[end - buf] = '\0';

    /* Return value currently unused, just make sure it's non-NULL */
    return(end);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to check whether wlan0 has been spawn or not
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
find_wifi_device(
    void
    )
{
    FILE *fh;
    char  buff[1024];
    int   ret = -1;

    fh = fopen(PROC_NET_DEV, "r");

    if(fh != NULL) {
        /* Success : use data from /proc/net/wireless */
        /* Eat 2 lines of header */
        fgets(buff, sizeof(buff), fh);
        fgets(buff, sizeof(buff), fh);

        /* Read each device line */
        while(fgets(buff, sizeof(buff), fh)) {
            char  name[IFNAMSIZ + 1];
            char *s;

            /* Skip empty or almost empty lines. It seems that in some
             * *        * cases fgets return a line with only a newline. */
            if ((buff[0] == '\0') || (buff[1] == '\0'))
                continue;
            /* Extract interface name */
            s = iw_get_ifname(name, sizeof(name), buff);

            if(s) {
                ALOGD("[%s] %s", __func__, name);
                if (strcmp(name, WIFI_IF_NAME) == 0 ){
                    ret = 0;
                    break;
                }
            }
        }

        fclose(fh);
    }

    return ret;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief   Wrapper to push some Wireless Parameter in the driver
 *
 * @param   request Wireless extension identifier
 *          pwrq    Pointer to wireless extension request
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static inline int
ioctl_iw_ext(
    int             request,
    struct iwreq   *pwrq
    )
{
    if(skfd > 0) {
        /* Set device name */
        strncpy(pwrq->ifr_name, WIFI_IF_NAME, IFNAMSIZ);

        /* Do the request */
        return(ioctl(skfd, request, pwrq));
    }
    else {
        return -1;
    }
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This API is to ask underlying software to enter/leave RF test mode
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
wifi_switch_test_mode(
    int on
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure NDIS_TRANSPORT_STRUC */
    if(on == 1) {
        rNdisStruct.ndisOidCmd = OID_CUSTOM_TEST_MODE;
    }
    else if(on == 0) {
        rNdisStruct.ndisOidCmd = OID_CUSTOM_ABORT_TEST_MODE;
    }
    else {
        return -1;
    }

    rNdisStruct.inNdisOidlength = 0;
    rNdisStruct.outNdisOidLength = 0;

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_SET_STRUCT_FOR_EM, &wrq);

    if(retval == 0) {
        if(on == 1) {
            fgIsTesting = true;
        }
        else {
            fgIsTesting = false;
        }
    }

    return retval;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a generic service for RF test set commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
wifi_test_set(
    uint32_t  u4FuncIndex,
    uint32_t  u4FuncData,
    uint32_t *pu4FuncIndex,
    uint32_t *pu4FuncData
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;

    prTestStruct = (P_PARAM_MTK_WIFI_TEST_STRUC_T)rNdisStruct.ndisOidContent;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure TEST_STRUCT */
    prTestStruct->u4FuncIndex = u4FuncIndex;
    prTestStruct->u4FuncData = u4FuncData;

    /* configure NDIS_TRANSPORT_STRUC */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_SET_STRUCT_FOR_EM, &wrq);

    if(retval == 0) {
        if(pu4FuncIndex) {
            *pu4FuncIndex = prTestStruct->u4FuncIndex;
        }

        if(pu4FuncData) {
            *pu4FuncData = prTestStruct->u4FuncData;
        }
    }

    return retval;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a generic service for RF test query commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
wifi_test_get(
    uint32_t  u4FuncIndex,
    uint32_t  u4FuncData,
    uint32_t *pu4FuncIndex,
    uint32_t *pu4FuncData
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;

    prTestStruct = (P_PARAM_MTK_WIFI_TEST_STRUC_T)rNdisStruct.ndisOidContent;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure TEST_STRUCT */
    prTestStruct->u4FuncIndex = u4FuncIndex;
    prTestStruct->u4FuncData = u4FuncData;

    /* configure NDIS_TRANSPORT_STRUC */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_GET_STRUCT, &wrq);

    if(retval == 0) {
        if(pu4FuncIndex) {
            *pu4FuncIndex = prTestStruct->u4FuncIndex;
        }

        if(pu4FuncData) {
            *pu4FuncData = prTestStruct->u4FuncData;
        }
    }

    return retval;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a generic service for RF test set sw_ctrl commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
wifi_test_set_swctrl(
    uint32_t  u4FuncIndex,
    uint32_t  u4FuncData,
    uint32_t *pu4FuncIndex,
    uint32_t *pu4FuncData
    )
{

    int retval;
    struct iwreq wrq;
    PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));
	memset(&prTestStruct, 0, sizeof(PARAM_MTK_WIFI_TEST_STRUC_T));

    /* configure TEST_STRUCT */
    prTestStruct.u4FuncIndex = u4FuncIndex;
    prTestStruct.u4FuncData = u4FuncData;

    /* configure struct iwreq */
    wrq.u.data.pointer = &prTestStruct;
    wrq.u.data.length = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    wrq.u.data.flags = PRIV_CMD_SW_CTRL;

    retval = ioctl_iw_ext(IOCTL_SET_STRUCT_FOR_EM, &wrq);

    if(retval == 0) {
        if(pu4FuncIndex) {
            *pu4FuncIndex = prTestStruct.u4FuncIndex;
        }

        if(pu4FuncData) {
            *pu4FuncData = prTestStruct.u4FuncData;
        }
    }

    return retval;


}
/* API
 * ========================================================================== */
bool LGE_RFT_OpenDUT(void)
{
    int count = 60;
    bool retval = false;

    ALOGD("[%s] entry\n", __func__);

    wifi_set_power(1);

#if 0 //built-in
    if (!check_driver_loaded()) {
        ALOGD("[%s] loading wifi driver ... ...\n", __func__);

        if (insmod(DRIVER_MODULE_PATH, DRIVER_MODULE_ARG) < 0) {
            ALOGD("[%s] failed to load wifi driver!!\n", __func__);
            goto error;
        }
    }
#endif 

    sched_yield();

    while(count -- > 0) {
        if(find_wifi_device()==0) {
            retval = true;
            ALOGD("[%s] find wifi device\n", __func__);
            break;
        }
        usleep(50000);
    }

    if (retval == false) {
        goto error;
    }
    else {
        /* initialize skfd */
        if ((skfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            ALOGE("[%s] failed to open net socket\n", __func__);
            goto error;
        }
    }

    /* switch into test mode */
    if(wifi_switch_test_mode(1) != 0) {
        goto error;
    }

    return true;

error:
    ALOGD("[%s] failure", __func__);

    LGE_RFT_CloseDUT();

    return false;
}

bool LGE_RFT_CloseDUT(void)
{
    /* turn off test mode */
    wifi_switch_test_mode(0);

    /* close socket if necessary */
    if(skfd > 0) {
        close(skfd);
        skfd = -1;
    }

    /* no need to remove module, just turn off host power via rfkill */
    wifi_set_power(0);

    return true;
}

bool LGE_RFT_TxDataRate(int TxDataRate)
{
    int retval;

	switch (TxDataRate) {
	case LGE_RFT_RATE_AUTO:
        return false;   //@FIXME

	case LGE_RFT_RATE_1MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_1M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_2MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_2M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_5_5MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_5_5M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_6MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_6M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_9MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_9M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_11MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_11M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_12MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_12M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_18MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_18M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_24MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_24M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_36MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_36M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_48MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_48M,
                NULL,
                NULL);
        break;

	case LGE_RFT_RATE_54MBPS:
        retval = wifi_test_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_54M,
                NULL,
                NULL);
        break;

	default:
        return false;

	}

    /* return value checking */
    if(retval == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_SetPreamble(PreambleType_t PreambleType)
{
	int retval;

	switch (PreambleType) {
	case LGE_RFT_PREAMBLE_LONG:
        retval = wifi_test_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_NORMAL,
                NULL,
                NULL);
		break;

	case LGE_RFT_PREAMBLE_SHORT:
        retval = wifi_test_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_CCK_SHORT,
                NULL,
                NULL);
		break;

	default:
        return false;
	}

    /* return value checking */
    if(retval == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_Channel(int ChannelNo)
{
    uint32_t u4Freq;

    if(ChannelNo < 0) {
        return false; /* invalid channel number */
    }
    /* 2.4GHz band */
    else if(ChannelNo <= 13) {
        u4Freq = 2412000 + (ChannelNo - 1) * 5000;
    }
    else if(ChannelNo == 14) {
        u4Freq = 2484000;
    }
    /* 5GHz band */
    else if(ChannelNo >= 36) {
        u4Freq = 5180000 + (ChannelNo - 36) * 5000;
    }
    else {
        return false; /* invalid channel number */
    }

    if(wifi_test_set(RF_AT_FUNCID_CHNL_FREQ,
            u4Freq,
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_TxGain(int TxGain)
{
    /* assign TX power gain */
    if(wifi_test_set(RF_AT_FUNCID_POWER,
            TxGain * 2, // in unit of 0.5dBm
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_TxBurstInterval(int SIFS)
{
	if (SIFS < 20 || SIFS > 1000)
		return false;

    /* specify packet interval */
    if(wifi_test_set(RF_AT_FUNCID_PKTINTERVAL,
            SIFS,
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_TxPayloadLength(int TxPayLength)
{
	if (TxPayLength <= 0)
		return false;

    /* specify packet length */
    if(wifi_test_set(RF_AT_FUNCID_PKTLEN,
            TxPayLength,
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_TxBurstFrames(int Frames)
{
	if (Frames < 0)
		return false;

    /* specify packet count */
    if(wifi_test_set(RF_AT_FUNCID_PKTCNT,
                Frames,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_TxDestAddress(unsigned char *addr)
{
    uint8_t aucMacAddr[4];
    uint32_t u4MacAddr;

    /* specify MAC address[0:3] */
    memcpy(aucMacAddr, addr, sizeof(uint8_t) * 4);
    u4MacAddr = *((uint32_t *)&(aucMacAddr[0]));
    if(wifi_test_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
                u4MacAddr,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify MAC address[4:5] */
    memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
    memcpy(aucMacAddr, addr + 4, sizeof(uint8_t) * 2);
    u4MacAddr = *((uint32_t *)&(aucMacAddr[0]));
    if(wifi_test_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                u4MacAddr,
                NULL,
                NULL) != 0) {
        return false;
    }

    return true;
}

bool LGE_RFT_TxStart(void)
{
	/* tx start: without ack, async mode */
    if(wifi_test_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STARTTX,
                NULL,
                NULL) == 0) {

	if(true == LGE_RFT_TxBurstFrames(0))
        	return true;
        else
		return false;
    }
    else {
        return false;
    }
}

bool LGE_RFT_TxStop(void)
{
    /* checking for testing mode */
    if(LGE_RFT_IsRunning() == false) {
        LGE_RFT_OpenDUT();
    }

	/* tx stop */
    if(wifi_test_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_RxStart(struct pkteng_rx* lge_rft_pkteng_rx_temp)
{
    uint32_t u4Tmp;

    /* fill pkteng_rx */
    if(lge_rft_pkteng_rx_temp) {
        if(wifi_test_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                &u4Tmp) == 0) {
            lge_rft_pkteng_rx_temp->pktengrxducast_old = u4Tmp;
        }

        if(wifi_test_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                &u4Tmp) == 0) {
            lge_rft_pkteng_rx_temp->rxbadfcs_old = u4Tmp;
        }
    }

    {
	int i = 1;
	if(false == LGE_RFT_SetUnicast(i))
		return false;
    }
    /* rx start: without ack, async mode */
    if(wifi_test_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STARTRX,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_RxStop(struct pkteng_rx* lge_rft_pkteng_rx_temp)
{
    uint32_t u4Tmp;

    /* checking for testing mode */
    if(LGE_RFT_IsRunning() == false) {
        LGE_RFT_OpenDUT();
    }

	/* get pkteng rx */
    if(lge_rft_pkteng_rx_temp) {
        if(wifi_test_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                &u4Tmp) == 0) {
            lge_rft_pkteng_rx_temp->pktengrxducast_new = u4Tmp;
        }

        if(wifi_test_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                &u4Tmp) == 0) {
            lge_rft_pkteng_rx_temp->rxbadfcs_new = u4Tmp;
        }
    }

	/* rx stop */
    if(wifi_test_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_FRError(int *FError)
{
	if (!FError)
		return false;

    if(wifi_test_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                (uint32_t *)FError) == 0) {
        ALOGD("[%s] Error=[%d]\n", __func__, *FError);
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_FRGood(int *FRGood)
{
	if (!FRGood)
		return false;

    if(wifi_test_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                (uint32_t *)FRGood) == 0) {
        ALOGD("[%s] Good=[%d]\n", __func__, *FRGood);
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_RSSI(int *RSSI)
{
    uint32_t u4Result;

	if (!RSSI)
		return false;

    if(wifi_test_get(RF_AT_FUNCID_RX_RSSI,
                0,
                NULL,
                &u4Result) == 0) {
        /* u4Result[0:7]    Average RSSI (dbM)
         * u4Result[8:15]   Minimum RCPI
         * u4Result[16:23]  Maximum RCPI
         * u4Result[24:31]  Last RCPI
         */
        *RSSI = (int)(u4Result & BITS(0,7));
        ALOGD("[%s] RSSI=[%d]\n", __func__, *RSSI);
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_IsRunning(void)
{
	return fgIsTesting;
}

bool LGE_RFT_IsUp(void)
{
    struct ifreq ifr;
    int sk, up;

    /* NULL ptr checking */
    sk = socket(PF_INET, SOCK_STREAM, 0);
    if (sk == -1) {
        return false;
    }

    strncpy(ifr.ifr_name, WIFI_IF_NAME, sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

    /* retrieve hardware address */
    if (ioctl(sk, SIOCGIFFLAGS, &ifr) == -1) {
        close(sk);
        return false;
    }

    up = ifr.ifr_flags & IFF_UP;

    close(sk);

    if(up) {
        return true;
    }
    else {
        return false;
    }
}

/* for 11n test */
// TxDataRate11n     : 1~8
// FrameFormat        : 1 (Mixed Mode), 2 (Green field mode)
// GI (Gard interval) : 1 (Long GI), 2 (Short GI)
bool LGE_RFT_TxDataRate11n(int TxDataRate11n, int FrameFormat, int GI)
{
    uint32_t u4Rate, u4Preamble;
	TxDataRate11n--;
	FrameFormat--;
	GI--;

	if( (TxDataRate11n < LGE_RFT_MCS_RATE_0) || (TxDataRate11n > LGE_RFT_MCS_RATE_7) )
	{
		return false;
	}

	if( (FrameFormat != 0) && (FrameFormat != 1) )
	{
		return false;
	}

	if( (GI != 0) && (GI != 1) )
	{
		return false;
	}

    /* specify 11n rate */
    u4Rate = RF_AT_PARAM_RATE_MCS_MASK | TxDataRate11n;
    if(wifi_test_set(RF_AT_FUNCID_RATE,
                u4Rate,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify preamble type */
    switch(FrameFormat) {
    case 0:
        u4Preamble = RF_AT_PREAMBLE_11N_MM;
        break;

    case 1:
        u4Preamble = RF_AT_PREAMBLE_11N_GF;
        break;

    default:
        return false;
    }
    if(wifi_test_set(RF_AT_FUNCID_PREAMBLE,
                u4Preamble,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify Guard Interval type */
    if(wifi_test_set(RF_AT_FUNCID_GI,
                GI,
                NULL,
                NULL) != 0) {
        return false;
    }

	return true;
}

/* for 11n test */
bool LGE_RFT_FrequencyAccuracy(int band, int ChannelNo)
{
    ALOGD("[%s] unused band parameter: (%d)\n", __func__, band);

    /* set channel */
    if(LGE_RFT_Channel(ChannelNo) == false) {
        return false;
    }

    /* start carrier tone */
    if(wifi_test_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_LO_LEAKAGE,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

/* for 11n test */
bool LGE_RFT_FrequencyAccuracy_Stop()
{
    /* checking for testing mode */
    if(LGE_RFT_IsRunning() == false) {
        LGE_RFT_OpenDUT();
    }

	/* stop */
    if(wifi_test_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool LGE_RFT_GetMACAddr(unsigned char *macAddr)
{
    struct ifreq ifr;
    int sk;

    /* NULL ptr checking */
    if(macAddr == NULL) {
        return false;
    }

    sk = socket(PF_INET, SOCK_STREAM, 0);
    if (sk == -1) {
        return false;
    }

    strncpy(ifr.ifr_name, WIFI_IF_NAME, sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

    /* retrieve hardware address */
    if (ioctl(sk, SIOCGIFHWADDR, &ifr) == -1) {
        close(sk);
        return false;
    }

    memcpy(macAddr, ifr.ifr_hwaddr.sa_data, sizeof(unsigned char) * 6);

    close(sk);

    return true;
}

bool LGE_RFT_SetMACAddr(unsigned char *macAddr)
{
    const uint8_t aucZeroMacAddr[] = NULL_MAC_ADDR;

#if defined(USE_MTK_NVRAM)
    F_INFO rFileInfo;
    int fd;
#endif

    /* MAC address validation */
    if(IS_BMCAST_MAC_ADDR(macAddr)
            || EQUAL_MAC_ADDR(aucZeroMacAddr, macAddr)) {
        ALOGW("[%s] invalid address "MACSTR"\n", __func__, MAC2STR(macAddr));
        return false;
    }


#if defined(USE_MTK_NVRAM)
    // MTK SOLUTION
    rFileInfo = NVM_ReadFileVerInfo(AP_CFG_RDEB_FILE_WIFI_LID);
    fd = open(rFileInfo.cFileName, O_WRONLY|O_CREAT, S_IRUSR);
    if(fd) {
        lseek(fd, offsetof(WIFI_CFG_PARAM_STRUCT, aucMacAddress), SEEK_SET);
        write(fd, macAddr, MAC_ADDR_LEN);
        close(fd);

        // invoke auto backup mechanism
        NVM_AddBackupFileNum(AP_CFG_RDEB_FILE_WIFI_LID);

        return true;
    }
    else {
        ALOGW("[%s] open(%s) failed: %s (%d)\n", __func__, rFileInfo.cFileName, strerror(errno), errno);
        return false;
    }
#elif defined(USE_LGE_NVRAM)
    return LGE_FacWriteWifiMacAddr(macAddr, true);
#else
    return false;
#endif
}

bool LGE_RFT_SetTestMode(bool bEnable)
{
    F_INFO rFileInfo;
    int fd;
    uint8_t ucMode;

    // MTK SOLUTION
    rFileInfo = NVM_ReadFileVerInfo(AP_CFG_RDEB_FILE_WIFI_LID);
    fd = open(rFileInfo.cFileName, O_WRONLY|O_CREAT, S_IRUSR);

    if(!fd) {
        ALOGW("[%s] open(%s) failed: %s (%d)\n", __func__, rFileInfo.cFileName, strerror(errno), errno);
        return false;
    }
    else {
        lseek(fd, offsetof(WIFI_CFG_PARAM_STRUCT, ucDefaultTestMode), SEEK_SET);

        if(bEnable == true) {
            ucMode = 1;
        }
        else {
            ucMode = 0;
        }

        write(fd, &ucMode, sizeof(uint8_t));
        close(fd);

        // invoke auto backup mechanism
        NVM_AddBackupFileNum(AP_CFG_RDEB_FILE_WIFI_LID);

        return true;
    }
}


bool LGE_RFT_Enable_Chain(int is_tx) //optinal
{
	int tx = is_tx;

	tx++;

	return true;
}

bool LGE_RFT_Disable_Chain()//optina
{

	return true;
}

bool LGE_RFT_CloseTCPLoop()//optinal
{
	return true;
}

// bool LGE_RFT_TxSetParameters()  //what's parameters
// {}

bool LGE_RFT_SetRXFilter(char *aucSrcAddr, char *aucDstAddr)
{
        uint8_t aucMacAddr[4];
        uint32_t u4MacAddr;
        ENUM_RF_AT_RXOK_MATCH_RULE_T eRxRule;

        if(aucSrcAddr && aucDstAddr) {
            eRxRule = RF_AT_RXOK_DISABLED;
        }
        else if(aucDstAddr) {
            eRxRule = RF_AT_RXOK_MATCH_RA_ONLY;
        }
        else if(aucSrcAddr) {
            eRxRule = RF_AT_RXOK_MATCH_TA_ONLY;
        }
        else {
            eRxRule = RF_AT_RXOK_DISABLED;
        }

        /* specify policy */
        if(wifi_test_set(RF_AT_FUNCID_SET_RXOK_MATCH_RULE,
                    eRxRule,
                    NULL,
                    NULL) != 0) {
            return false;
        }

        if(aucDstAddr) {
            /* specify MAC address[0:3] */
            memcpy(aucMacAddr, aucDstAddr, sizeof(uint8_t) * 4);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(wifi_test_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }

            /* specify MAC address[4:5] */
            memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
            memcpy(aucMacAddr, &(aucDstAddr[4]), sizeof(uint8_t) * 2);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(wifi_test_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }
        }

        if(aucSrcAddr) {
            /* specify MAC address[0:3] */
            memcpy(aucMacAddr, aucSrcAddr, sizeof(uint8_t) * 4);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(wifi_test_set(RF_AT_FUNCID_SET_MAC_SRC_ADDRESS,
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }

            /* specify MAC address[4:5] */
            memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
            memcpy(aucMacAddr, &(aucSrcAddr[4]), sizeof(uint8_t) * 2);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(wifi_test_set(RF_AT_FUNCID_SET_MAC_SRC_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }
        }

        return  true;
}

bool LGE_RFT_SetUnicast(int start)
{
	bool ret = false;
	char txMac[6] = {0x00, 0x90, 0x0c, 0xba, 0xcd, 0x88};
    char txMac2[6] = {0x00, 0x90, 0x0c, 0xba, 0xcd, 0x99};
	ALOGD("[%s] Tx MAC[%2X:%2X:%2X:%2X:%2X:%2X]\n", __func__, txMac[0],txMac[1],txMac[2],txMac[3], txMac[4],txMac[5]);
	if(1 == start){
 if(true == LGE_RFT_SetRXFilter(txMac2, txMac))
			ret = true;
	}

	return ret;
}


bool LGE_NoMod_TxGain(int rf_gain)
{
	int ret = 0;

	if(false == LGE_RFT_TxGain(rf_gain))
		ret = false;

	return ret;
}


bool LGE_NoMod_TxStart()//mandatory
{
	int ret = 0;

    if(wifi_test_set(RF_AT_FUNCID_COMMAND,
            RF_AT_COMMAND_CW,
            NULL,
            NULL) == 0) {
	    if(true == LGE_RFT_TxStart())
		    ret = true;
    }else{
        ALOGD("[%s] RF_AT_COMMAND_CW failed !\n", __func__);
        ret = false;
    }

	return ret;
}

bool LGE_NoMod_TxStop()//mandatory
{
	int ret = 0;

	if(true == LGE_RFT_TxStop())
		ret = true;

	return ret;
}

// bool LGE_RFT_RxResetCnt()//optional
// {}

bool LGE_RFT_TotalPkt(int *FRTotal) //Mandatory
{
	int error = 0, good = 0;
	bool ret = false;

	if(true	== LGE_RFT_FRError(&error) &&
		true == LGE_RFT_FRGood(&good) ){
		*FRTotal = error + good;
		ret = true;
	}

	return ret;
}

bool LGE_RFT_SetWaveform()  //optional set OFDM, DSSS, FHSS
{
	return true;
}

bool LGE_RFT_WriteMac(char *addr)
{
	return  LGE_RFT_SetMACAddr((unsigned char*)addr);
}

bool LGE_RFT_ReadMac(char *addr)
{
	return LGE_RFT_GetMACAddr((unsigned char*) addr);
}

//for the OTA  test
bool LGE_RFT_PowerSave(bool enable)
{
	bool ret = false;

    /* initialize skfd */
    if ((skfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        ALOGE("[%s] failed to open net socket\n", __func__);
        return false;
    }

	if(false == enable){  //disable power saving
		if(0 == wifi_test_set_swctrl(0x10008000, 0, NULL, NULL))
			ret = true;
	}else if(true == enable){ // enable power saving
		if(0 == wifi_test_set_swctrl(0x10008000, 2, NULL, NULL))
			ret = true;
	}

    /* close socket if necessary */
    if(skfd > 0) {
        close(skfd);
        skfd = -1;
    }

	return ret;

}

bool LGE_RFT_Roaming(bool enable)
{
	bool ret = false;
        /* initialize skfd */
    if ((skfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        ALOGE("[%s] failed to open net socket\n", __func__);
        return false;
    }

	if(false == enable){// disable roaming
		if(0 == wifi_test_set_swctrl(0x90000204, 0, NULL, NULL) &&
			0 == wifi_test_set_swctrl(0x90000200, 0x00820000, NULL, NULL) &&
			0 == wifi_test_set_swctrl(0x10010001, 1, NULL, NULL) )
			ret = true;
	}else if(true == enable){ //enable roaming
		if(0 == wifi_test_set_swctrl(0x90000204, 1, NULL, NULL) &&
			0 == wifi_test_set_swctrl(0x90000200, 0x00820000, NULL, NULL) &&
			0 == wifi_test_set_swctrl(0x10010001, 0, NULL, NULL) )
			ret = true;
	}
    /* close socket if necessary */
    if(skfd > 0) {
        close(skfd);
        skfd = -1;
    }

	return ret;
}
