/*
** $Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/os/linux/include/gl_cfg80211.h#1 $
*/

/*! \file   gl_cfg80211.h
    \brief  This file is for Portable Driver linux cfg80211 support.
*/



/*
** $Log: gl_cfg80211.h $
**
** 08 23 2013 wh.su
** [BORA00002446] [MT6630] [Wi-Fi] [Driver] Update the security function code
** Add GTK re-key driver handle function
**
** 08 09 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** 1. integrate scheduled scan functionality
** 2. condition compilation for linux-3.4 & linux-3.8 compatibility
** 3. correct CMD queue access to reduce lock scope
**
** 07 29 2013 cp.wu
** [BORA00002725] [MT6630][Wi-Fi] Add MGMT TX/RX support for Linux port
** Preparation for porting remain_on_channel support
** 
** 09 17 2012 cm.chang
** [BORA00002149] [MT6630 Wi-Fi] Initial software development
** Duplicate source from MT6620 v2.3 driver branch
** (Davinci label: MT6620_WIFI_Driver_V2_3_120913_1942_As_MT6630_Base)
** 
** 08 24 2012 cp.wu
** [WCXRP00001269] [MT6620 Wi-Fi][Driver] cfg80211 porting merge back to DaVinci
** .
 *
*/

#ifndef _GL_CFG80211_H
#define _GL_CFG80211_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>

#include "gl_os.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
/* cfg80211 hooks */
int 
mtk_cfg80211_change_iface (
    struct wiphy *wiphy,
    struct net_device *ndev,
    enum nl80211_iftype type,
    u32 *flags,
    struct vif_params *params
    );


int
mtk_cfg80211_add_key (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 key_index,
    bool pairwise,
    const u8 *mac_addr,
    struct key_params *params
    );


int 
mtk_cfg80211_get_key (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 key_index,
    bool pairwise,
    const u8 *mac_addr,
    void *cookie,
    void (*callback)(void *cookie, struct key_params*)
    );


int
mtk_cfg80211_del_key (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 key_index,
    bool pairwise,
    const u8 *mac_addr
    );


int 
mtk_cfg80211_set_default_key (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 key_index,
    bool unicast,
    bool multicast
    );


int
mtk_cfg80211_get_station (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 *mac,
    struct station_info *sinfo
    );


int 
mtk_cfg80211_scan (
    struct wiphy *wiphy,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
    struct net_device *ndev,
#endif
    struct cfg80211_scan_request *request
    );


int
mtk_cfg80211_connect (
    struct wiphy *wiphy,
    struct net_device *ndev,
    struct cfg80211_connect_params *sme
    );


int 
mtk_cfg80211_disconnect (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u16 reason_code
    );


int
mtk_cfg80211_join_ibss (
    struct wiphy *wiphy,
    struct net_device *ndev,
    struct cfg80211_ibss_params *params
    );


int
mtk_cfg80211_leave_ibss (
    struct wiphy *wiphy,
    struct net_device *ndev
    );


int
mtk_cfg80211_set_power_mgmt (
    struct wiphy *wiphy,
    struct net_device *ndev,
    bool enabled,
    int timeout
    );


int
mtk_cfg80211_set_pmksa (
    struct wiphy *wiphy,
    struct net_device *ndev,
    struct cfg80211_pmksa *pmksa
    );


int
mtk_cfg80211_del_pmksa (
    struct wiphy *wiphy,
    struct net_device *ndev,
    struct cfg80211_pmksa *pmksa
    );


int
mtk_cfg80211_flush_pmksa (
    struct wiphy *wiphy,
    struct net_device *ndev
    );

int
mtk_cfg80211_set_rekey_data (
    struct wiphy *wiphy, 
    struct net_device *dev,
    struct cfg80211_gtk_rekey_data *data
    );

int 
mtk_cfg80211_remain_on_channel (
    struct wiphy *wiphy,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
    struct wireless_dev *wdev,
#else
    struct net_device *ndev,
#endif
    struct ieee80211_channel *chan,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
    enum nl80211_channel_type channel_type,
#endif
    unsigned int duration,
    u64 *cookie
    );


int
mtk_cfg80211_cancel_remain_on_channel (
    struct wiphy *wiphy,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
    struct wireless_dev *wdev,
#else
    struct net_device *ndev,
#endif
    u64 cookie
    );


int
mtk_cfg80211_mgmt_tx (
    struct wiphy *wiphy,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
    struct wireless_dev *wdev,
#else
    struct net_device *ndev,
#endif
    struct ieee80211_channel *channel,
    bool offscan,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
    enum nl80211_channel_type channel_type,
    bool channel_type_valid,
#endif
    unsigned int wait,
    const u8 *buf,
    size_t len,
    bool no_cck,
    bool dont_wait_for_ack,
    u64 *cookie
    );

void
mtk_cfg80211_mgmt_frame_register (
    IN struct wiphy *wiphy,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
    IN struct wireless_dev *wdev,
#else
    IN struct net_device *ndev,
#endif
    IN u16 frame_type,
    IN bool reg
    );

int
mtk_cfg80211_mgmt_tx_cancel_wait (
    struct wiphy *wiphy,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
    struct wireless_dev *wdev,
#else
    struct net_device *ndev,
#endif
    u64 cookie
    );

#if CONFIG_NL80211_TESTMODE
int
mtk_cfg80211_testmode_cmd(
    IN struct wiphy *wiphy,
    IN void *data,
    IN int len
    );

int
mtk_cfg80211_testmode_sw_cmd(
    IN struct wiphy *wiphy,
    IN void *data,
    IN int len
    );
#if CFG_SUPPORT_WAPI
int
mtk_cfg80211_testmode_set_key_ext(
    IN struct wiphy *wiphy,
    IN void *data,
    IN int len
    );
#endif
#else
    #error "Please ENABLE kernel config (CONFIG_NL80211_TESTMODE) to support Wi-Fi Direct"
#endif

int
mtk_cfg80211_sched_scan_start(
    IN struct wiphy *wiphy,
    IN struct net_device *ndev,
    IN struct cfg80211_sched_scan_request *request
    );

int
mtk_cfg80211_sched_scan_stop(
    IN struct wiphy *wiphy,
    IN struct net_device *ndev
    );


/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _GL_CFG80211_H */

