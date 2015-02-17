/*
** $Id: @(#) gl_cfg80211.c@@
*/

/*! \file   gl_cfg80211.c
    \brief  Main routines for supporintg MT6620 cfg80211 control interface

    This file contains the support routines of Linux driver for MediaTek Inc. 802.11
    Wireless LAN Adapters.
*/



/*
** $Log: gl_cfg80211.c $
**
** 09 05 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** correct calls to wlanoidSetBssid()
**
** 08 28 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** fix typo
**
** 08 28 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** add more protection in case cfg80211_sched_scan_request->match_sets[] == NULL
**
** 08 28 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** fix for KE issues because refering to wrong data member
**
** 08 23 2013 wh.su
** [BORA00002446] [MT6630] [Wi-Fi] [Driver] Update the security function code
** Add GTK re-key driver handle function
**
** 08 19 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** use kalMemFree() instead of kfree()
**
** 08 19 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** change to use dynamic-allocated memory for schedule scan to avoid stack overflow
**
** 08 15 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** enlarge  match_ssid_num to 16 for PNO support
**
** 08 12 2013 cp.wu
** [BORA00002227] [MT6630 Wi-Fi][Driver] Update for Makefile and HIFSYS modifications
** 1. fix on cancel_remain_on_channel() interface 
** 2. queue initialization for another linux kal API
**
** 08 09 2013 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** 1. integrate scheduled scan functionality
** 2. condition compilation for linux-3.4 & linux-3.8 compatibility
** 3. correct CMD queue access to reduce lock scope
**
** 07 30 2013 cp.wu
** [BORA00002725] [MT6630][Wi-Fi] Add MGMT TX/RX support for Linux port
** add kernel version awareness for building success between different version of linux kernel
**
** 07 29 2013 cp.wu
** [BORA00002725] [MT6630][Wi-Fi] Add MGMT TX/RX support for Linux port
** Preparation for porting remain_on_channel support
**
** 07 23 2013 wh.su
** [BORA00002446] [MT6630] [Wi-Fi] [Driver] Update the security function code
** Sync the latest jb2.mp 11w code as draft version
** Not the CM bit for avoid wapi 1x drop at re-key
**
** 07 05 2013 wh.su
** [BORA00002446] [MT6630] [Wi-Fi] [Driver] Update the security function code
** Fix to let the wpa-psk ok
**
** 07 02 2013 wh.su
** [BORA00002446] [MT6630] [Wi-Fi] [Driver] Update the security function code
** Refine security BMC wlan index assign
** Fix some compiling warning
**
** 07 01 2013 wh.su
** [BORA00002446] [MT6630] [Wi-Fi] [Driver] Update the security function code
** Add some debug code, fixed some compiling warning
**
** 03 20 2013 wh.su
** [BORA00002446] [MT6630] [Wi-Fi] [Driver] Update the security function code
** Add the security code for wlan table assign operation
**
** 11 15 2012 cp.wu
** [BORA00002253] [MT6630 Wi-Fi][Driver][Firmware] Add NLO and timeout mechanism to SCN module
** sync..
** 
** 09 17 2012 cm.chang
** [BORA00002149] [MT6630 Wi-Fi] Initial software development
** Duplicate source from MT6620 v2.3 driver branch
** (Davinci label: MT6620_WIFI_Driver_V2_3_120913_1942_As_MT6630_Base)
** 
** 08 30 2012 cp.wu
** [WCXRP00001269] [MT6620 Wi-Fi][Driver] cfg80211 porting merge back to DaVinci
** check pending scan only by the pointer instead of fgIsRegistered flag.
** 
** 08 24 2012 cp.wu
** [WCXRP00001269] [MT6620 Wi-Fi][Driver] cfg80211 porting merge back to DaVinci
** .
** 
** 08 24 2012 cp.wu
** [WCXRP00001269] [MT6620 Wi-Fi][Driver] cfg80211 porting merge back to DaVinci
** cfg80211 support merge back from ALPS.JB to DaVinci - MT6620 Driver v2.3 branch.
 *
**
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "gl_os.h"
#include "debug.h"
#include "wlan_lib.h"
#include "gl_wext.h"
#include "precomp.h"

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

#if CFG_SUPPORT_WAPI
    extern UINT_8 keyStructBuf[1024];   /* add/remove key shared buffer */
#else
    extern UINT_8 keyStructBuf[100];   /* add/remove key shared buffer */
#endif

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for change STA type between
 *        1. Infrastructure Client (Non-AP STA)
 *        2. Ad-Hoc IBSS
 *
 * @param
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int 
mtk_cfg80211_change_iface (
    struct wiphy *wiphy,
    struct net_device *ndev,
    enum nl80211_iftype type,
    u32 *flags,
    struct vif_params *params
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    ENUM_PARAM_OP_MODE_T eOpMode;
    UINT_32 u4BufLen;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    if(type == NL80211_IFTYPE_STATION) {
        eOpMode = NET_TYPE_INFRA;
    }
    else if(type == NL80211_IFTYPE_ADHOC) {
        eOpMode = NET_TYPE_IBSS;
    }
    else {
        return -EINVAL;
    }

    rStatus = kalIoctl(prGlueInfo,
            wlanoidSetInfrastructureMode,
            &eOpMode,
            sizeof(eOpMode),
            FALSE,
            FALSE,
            TRUE,
            &u4BufLen);
    
    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("set infrastructure mode error:%lx\n", rStatus));
    }

    /* reset wpa info */
    prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
    prGlueInfo->rWpaInfo.u4KeyMgmt = 0;
    prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_NONE;
    prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_NONE;
    prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;
#if CFG_SUPPORT_802_11W
    prGlueInfo->rWpaInfo.u4Mfp = IW_AUTH_MFP_DISABLED;
#endif

    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for adding key 
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_add_key (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 key_index,
    bool pairwise,
    const u8 *mac_addr,
    struct key_params *params
    )
{
    PARAM_KEY_T rKey;
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    INT_32 i4Rslt = -EINVAL;
    UINT_32 u4BufLen = 0;
    UINT_8 tmp1[8], tmp2[8];
    const UINT_8 aucBCAddr[] = BC_MAC_ADDR;
    const UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

#if DBG
    DBGLOG(RSN, TRACE, ("mtk_cfg80211_add_key\n"));
    if (mac_addr) {
        DBGLOG(RSN, TRACE, ("keyIdx = %d pairwise = %d mac = "MACSTR"\n", key_index, pairwise, MAC2STR(mac_addr)));
    } else {
        DBGLOG(RSN, TRACE, ("keyIdx = %d pairwise = %d null mac\n", key_index, pairwise));      
    }
    DBGLOG(RSN, TRACE, ("Cipher = %x\n", params->cipher));
    DBGLOG_MEM8(RSN, TRACE, params->key, params->key_len);
#endif
    
    kalMemZero(&rKey, sizeof(PARAM_KEY_T));

    rKey.u4KeyIndex = key_index;
    
    if (mac_addr) {
        if (EQUAL_MAC_ADDR(mac_addr, aucZeroMacAddr)) {
            COPY_MAC_ADDR(rKey.arBSSID, aucBCAddr);
        }
        else  {
            COPY_MAC_ADDR(rKey.arBSSID, mac_addr);
        }

        if (pairwise) {
            //if (!((rKey.arBSSID[0] & rKey.arBSSID[1] & rKey.arBSSID[2] & rKey.arBSSID[3] & rKey.arBSSID[4] & rKey.arBSSID[5]) == 0xFF)) {
            //    rKey.u4KeyIndex |= BIT(31);
            //}
            rKey.u4KeyIndex |= BIT(31);
            rKey.u4KeyIndex |= BIT(30);
        }
    }
    else { /* Group key */
        COPY_MAC_ADDR(rKey.arBSSID, aucBCAddr);
    }
    
    if (params->key) {
        kalMemCopy(rKey.aucKeyMaterial, params->key, params->key_len);
        if (params->key_len == 32) {
            kalMemCopy(tmp1, &params->key[16], 8);
            kalMemCopy(tmp2, &params->key[24], 8);        
            kalMemCopy(&rKey.aucKeyMaterial[16], tmp2, 8);
            kalMemCopy(&rKey.aucKeyMaterial[24], tmp1, 8);
        }
    }

    rKey.u4KeyLength = params->key_len;
    rKey.u4Length =  ((UINT_32)&(((P_PARAM_KEY_T)0)->aucKeyMaterial)) + rKey.u4KeyLength;

    rStatus = kalIoctl(prGlueInfo,
            wlanoidSetAddKey,
            &rKey,
            rKey.u4Length,
            FALSE,
            FALSE,
            TRUE,
            &u4BufLen);

    if (rStatus == WLAN_STATUS_SUCCESS)
        i4Rslt = 0;

    return i4Rslt;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for getting key for specified STA
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int 
mtk_cfg80211_get_key (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 key_index,
    bool pairwise,
    const u8 *mac_addr,
    void *cookie,
    void (*callback)(void *cookie, struct key_params*)
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

#if 1
    printk("--> %s()\n", __func__);
#endif

    /* not implemented */

    return -EINVAL;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for removing key for specified STA
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_del_key (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 key_index,
    bool pairwise,
    const u8 *mac_addr
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    PARAM_REMOVE_KEY_T rRemoveKey;
    UINT_32 u4BufLen = 0;
    INT_32 i4Rslt = -EINVAL;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

#if DBG
    DBGLOG(RSN, TRACE, ("mtk_cfg80211_del_key\n"));
    if (mac_addr) {
        DBGLOG(RSN, TRACE, ("keyIdx = %d pairwise = %d mac = "MACSTR"\n", key_index, pairwise, MAC2STR(mac_addr)));
    } else {
        DBGLOG(RSN, TRACE, ("keyIdx = %d pairwise = %d null mac\n", key_index, pairwise));
    }
#endif

    kalMemZero(&rRemoveKey, sizeof(PARAM_REMOVE_KEY_T));
    if (mac_addr)
        COPY_MAC_ADDR(rRemoveKey.arBSSID, mac_addr);
    rRemoveKey.u4KeyIndex = key_index;
    rRemoveKey.u4Length = sizeof(PARAM_REMOVE_KEY_T);
    

    rStatus = kalIoctl(prGlueInfo,
            wlanoidSetRemoveKey,
            &rRemoveKey,
            rRemoveKey.u4Length,
            FALSE,
            FALSE,
            TRUE,
            &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("remove key error:%lx\n", rStatus));
    }
    else {
        i4Rslt = 0;
    }

    return i4Rslt;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for setting default key on an interface
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int 
mtk_cfg80211_set_default_key (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 key_index,
    bool unicast,
    bool multicast
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    PARAM_DEFAULT_KEY_T rDefaultKey;
    WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
    INT_32 i4Rst = -EINVAL;
    UINT_32 u4BufLen = 0;
    BOOLEAN fgDef = FALSE, fgMgtDef = FALSE;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

#if DBG
    DBGLOG(RSN, TRACE, ("mtk_cfg80211_set_default_key\n"));
    DBGLOG(RSN, TRACE, ("keyIdx = %d unicast = %d multicast = %d\n", key_index, unicast, multicast));
#endif

    rDefaultKey.ucKeyID = key_index;
    rDefaultKey.ucUnicast = unicast;
    rDefaultKey.ucMulticast = multicast;
    if (rDefaultKey.ucUnicast && !rDefaultKey.ucMulticast)
        return WLAN_STATUS_SUCCESS;

    if (rDefaultKey.ucUnicast && rDefaultKey.ucMulticast)
        fgDef = TRUE;

    if (!rDefaultKey.ucUnicast && rDefaultKey.ucMulticast)
        fgMgtDef = TRUE;
    
    rStatus = kalIoctl(prGlueInfo,
        wlanoidSetDefaultKey,
        &rDefaultKey,
        sizeof(PARAM_DEFAULT_KEY_T),
        FALSE,
        FALSE,
        TRUE,
        &u4BufLen);

    if (rStatus == WLAN_STATUS_SUCCESS)
        i4Rst = 0;

    return i4Rst;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for getting station information such as RSSI
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/

int
mtk_cfg80211_get_station (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u8 *mac,
    struct station_info *sinfo
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus;
    PARAM_MAC_ADDRESS arBssid;
    UINT_32 u4BufLen, u4Rate;
    INT_32 i4Rssi;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    kalMemZero(arBssid, MAC_ADDR_LEN);
    wlanQueryInformation(prGlueInfo->prAdapter,
            wlanoidQueryBssid,
            &arBssid[0],
            sizeof(arBssid),
            &u4BufLen);

    /* 1. check BSSID */
    if(UNEQUAL_MAC_ADDR(arBssid, mac)) {
        /* wrong MAC address */
        DBGLOG(REQ, WARN, ("incorrect BSSID: ["MACSTR"] currently connected BSSID["MACSTR"]\n", 
                    MAC2STR(mac), MAC2STR(arBssid)));
        return -ENOENT;
    }

    /* 2. fill TX rate */
    rStatus = kalIoctl(prGlueInfo,
        wlanoidQueryLinkSpeed,
        &u4Rate,
        sizeof(u4Rate),
        TRUE,
        FALSE,
        FALSE,
        &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("unable to retrieve link speed\n"));
    }
    else {
        sinfo->filled |= STATION_INFO_TX_BITRATE;
        sinfo->txrate.legacy = u4Rate / 1000; /* convert from 100bps to 100kbps */
    }

    if(prGlueInfo->eParamMediaStateIndicated != PARAM_MEDIA_STATE_CONNECTED) {
        /* not connected */
        DBGLOG(REQ, WARN, ("not yet connected\n"));
    }
    else {
        /* 3. fill RSSI */
        rStatus = kalIoctl(prGlueInfo,
                wlanoidQueryRssi,
                &i4Rssi,
                sizeof(i4Rssi),
                TRUE,
                FALSE,
                FALSE,
                &u4BufLen);
        
        if (rStatus != WLAN_STATUS_SUCCESS) {
            DBGLOG(REQ, WARN, ("unable to retrieve link speed\n"));
        }
        else {
            sinfo->filled |= STATION_INFO_SIGNAL;
            sinfo->signal = i4Rssi; /* dBm */
        }
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to do a scan
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int 
mtk_cfg80211_scan (
    struct wiphy *wiphy,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
    struct net_device *ndev,
#endif
    struct cfg80211_scan_request *request
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus;
    UINT_32 i, u4BufLen;
    PARAM_SCAN_REQUEST_ADV_T rScanRequest;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    /* check if there is any pending scan/sched_scan not yet finished */
    if(prGlueInfo->prScanRequest != NULL || prGlueInfo->prSchedScanRequest != NULL) {
        return -EBUSY;
    }

    if(request->n_ssids == 0) {
        rScanRequest.u4SsidNum = 0;
    }
    else if(request->n_ssids <= SCN_SSID_MAX_NUM) {
        rScanRequest.u4SsidNum = request->n_ssids;

        for(i = 0 ; i < request->n_ssids ; i++) {
            COPY_SSID(rScanRequest.rSsid[i].aucSsid,
                    rScanRequest.rSsid[i].u4SsidLen,
                    request->ssids[i].ssid,
                    request->ssids[i].ssid_len);
        }
    }
    else {
        return -EINVAL;
    }

    rScanRequest.u4IELength = request->ie_len;
    if(request->ie_len > 0) {
        rScanRequest.pucIE = (PUINT_8)(request->ie);
    }

    rStatus = kalIoctl(prGlueInfo,
        wlanoidSetBssidListScanAdv,
        &rScanRequest,
        sizeof(PARAM_SCAN_REQUEST_ADV_T),
        FALSE,
        FALSE,
        FALSE,
        &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("scan error:%lx\n", rStatus));
        return -EINVAL;
    }

    prGlueInfo->prScanRequest = request;

    return 0;
}

static UINT_8 wepBuf[48];

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to connect to 
 *        the ESS with the specified parameters
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_connect (
    struct wiphy *wiphy,
    struct net_device *ndev,
    struct cfg80211_connect_params *sme
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus;
    UINT_32 u4BufLen;
    ENUM_PARAM_ENCRYPTION_STATUS_T eEncStatus;
    ENUM_PARAM_AUTH_MODE_T eAuthMode;
    UINT_32 cipher;
    PARAM_SSID_T rNewSsid;
    BOOLEAN fgCarryWPSIE = FALSE;
    ENUM_PARAM_OP_MODE_T eOpMode;
    UINT_32 i, u4AkmSuite = 0;    
    P_DOT11_RSNA_CONFIG_AUTHENTICATION_SUITES_ENTRY prEntry;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    //printk("[wlan]mtk_cfg80211_connect\n");
    if (prGlueInfo->prAdapter->rWifiVar.rConnSettings.eOPMode > NET_TYPE_AUTO_SWITCH)
        eOpMode = NET_TYPE_AUTO_SWITCH;
    else 
        eOpMode = prGlueInfo->prAdapter->rWifiVar.rConnSettings.eOPMode;
    
    rStatus = kalIoctl(prGlueInfo,
        wlanoidSetInfrastructureMode,
        &eOpMode,
        sizeof(eOpMode),
        FALSE,
        FALSE,
        TRUE,
        &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(INIT, INFO, ("wlanoidSetInfrastructureMode fail 0x%lx\n", rStatus));
        return -EFAULT;
    }

    /* after set operation mode, key table are cleared */

    /* reset wpa info */
    prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
    prGlueInfo->rWpaInfo.u4KeyMgmt = 0;
    prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_NONE;
    prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_NONE;
    prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;
#if CFG_SUPPORT_802_11W
    prGlueInfo->rWpaInfo.u4Mfp = IW_AUTH_MFP_DISABLED;
    switch (sme->mfp){
    case NL80211_MFP_NO:
        prGlueInfo->rWpaInfo.u4Mfp = IW_AUTH_MFP_DISABLED;
        break;
    case NL80211_MFP_REQUIRED:
        prGlueInfo->rWpaInfo.u4Mfp = IW_AUTH_MFP_REQUIRED;
        break;
    default:
        prGlueInfo->rWpaInfo.u4Mfp = IW_AUTH_MFP_DISABLED;
        break;
    }
#endif

    if (sme->crypto.wpa_versions & NL80211_WPA_VERSION_1)
        prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_WPA;
    else if (sme->crypto.wpa_versions & NL80211_WPA_VERSION_2)
        prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_WPA2;
    else
        prGlueInfo->rWpaInfo.u4WpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
 
    switch (sme->auth_type) {
    case NL80211_AUTHTYPE_OPEN_SYSTEM:
        prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;
        break;
    case NL80211_AUTHTYPE_SHARED_KEY:
        prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_SHARED_KEY;
        break;
    default:
        prGlueInfo->rWpaInfo.u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM | IW_AUTH_ALG_SHARED_KEY;
        break;
    }


    if (sme->crypto.n_ciphers_pairwise) {

        //printk("[wlan] cipher pairwise (%d)\n", sme->crypto.ciphers_pairwise[0]);
        prGlueInfo->prAdapter->rWifiVar.rConnSettings.rRsnInfo.au4PairwiseKeyCipherSuite[0] = sme->crypto.ciphers_pairwise[0];
        switch (sme->crypto.ciphers_pairwise[0]) {
        case WLAN_CIPHER_SUITE_WEP40:
            prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_WEP40;
            break;
        case WLAN_CIPHER_SUITE_WEP104:
            prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_WEP104;
            break;
        case WLAN_CIPHER_SUITE_TKIP:
            prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_TKIP;
            break;
        case WLAN_CIPHER_SUITE_CCMP:
            prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_CCMP;
            break;
        case WLAN_CIPHER_SUITE_AES_CMAC:
            prGlueInfo->rWpaInfo.u4CipherPairwise = IW_AUTH_CIPHER_CCMP;
            break;
        default:
            DBGLOG(REQ, WARN, ("invalid cipher pairwise (%d)\n",
                   sme->crypto.ciphers_pairwise[0]));
            return -EINVAL;
        }
    }

    if (sme->crypto.cipher_group) {
        prGlueInfo->prAdapter->rWifiVar.rConnSettings.rRsnInfo.u4GroupKeyCipherSuite = sme->crypto.cipher_group;
        switch (sme->crypto.cipher_group) {
        case WLAN_CIPHER_SUITE_WEP40:
            prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_WEP40;
            break;          
        case WLAN_CIPHER_SUITE_WEP104:
            prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_WEP104;
            break;
        case WLAN_CIPHER_SUITE_TKIP:
            prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_TKIP;
            break;
        case WLAN_CIPHER_SUITE_CCMP:
            prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_CCMP;
            break;
        case WLAN_CIPHER_SUITE_AES_CMAC:
            prGlueInfo->rWpaInfo.u4CipherGroup = IW_AUTH_CIPHER_CCMP;
            break;
        default:
            DBGLOG(REQ, WARN, ("invalid cipher group (%d)\n",
                   sme->crypto.cipher_group));
            return -EINVAL;
        }
    }

    if (sme->crypto.n_akm_suites) {
        prGlueInfo->prAdapter->rWifiVar.rConnSettings.rRsnInfo.au4AuthKeyMgtSuite[0] = sme->crypto.akm_suites[0];
        if (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_WPA) {
            switch (sme->crypto.akm_suites[0]) {
            case WLAN_AKM_SUITE_8021X:
                eAuthMode = AUTH_MODE_WPA;
                u4AkmSuite = WPA_AKM_SUITE_802_1X;
                break;
            case WLAN_AKM_SUITE_PSK:
                eAuthMode = AUTH_MODE_WPA_PSK;
                u4AkmSuite = WPA_AKM_SUITE_PSK;
            break;
            default:
                DBGLOG(REQ, WARN, ("invalid auth mode (%d)\n",
                       eAuthMode));
                return -EINVAL;
            }
        } else if (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_WPA2) {
            switch (sme->crypto.akm_suites[0]) {
            case WLAN_AKM_SUITE_8021X:
                eAuthMode = AUTH_MODE_WPA2;
                u4AkmSuite = RSN_AKM_SUITE_802_1X;
            break;
            case WLAN_AKM_SUITE_PSK:
                eAuthMode = AUTH_MODE_WPA2_PSK;
                u4AkmSuite = RSN_AKM_SUITE_PSK;
            break;
#if CFG_SUPPORT_802_11W
            //Notice:: Need kernel patch!!
            case WLAN_AKM_SUITE_8021X_SHA256:    
                eAuthMode = AUTH_MODE_WPA2;
                u4AkmSuite = RSN_AKM_SUITE_802_1X_SHA256;
            break;
            case WLAN_AKM_SUITE_PSK_SHA256:
                eAuthMode = AUTH_MODE_WPA2_PSK;
                u4AkmSuite = RSN_AKM_SUITE_PSK_SHA256;
            break;
#endif
            default:
                DBGLOG(REQ, WARN, ("invalid auth mode (%d)\n",
                       eAuthMode));
                return -EINVAL;
            }
        }
    }

    if (prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_DISABLED) {
        eAuthMode = (prGlueInfo->rWpaInfo.u4AuthAlg == IW_AUTH_ALG_OPEN_SYSTEM) ?
             AUTH_MODE_OPEN : AUTH_MODE_AUTO_SWITCH;
    }

    prGlueInfo->rWpaInfo.fgPrivacyInvoke = sme->privacy;
    prGlueInfo->fgWpsActive = FALSE;

    if (sme->ie && sme->ie_len > 0) {
        WLAN_STATUS rStatus;
        UINT_32 u4BufLen;
        PUINT_8 prDesiredIE = NULL;

#if CFG_SUPPORT_WAPI
        rStatus = kalIoctl(prGlueInfo,
                wlanoidSetWapiAssocInfo,
                sme->ie,
                sme->ie_len,
                FALSE,
                FALSE,
                FALSE,
                &u4BufLen);
        
        if (rStatus != WLAN_STATUS_SUCCESS) {
            DBGLOG(SEC, WARN, ("[wapi] set wapi assoc info error:%lx\n", rStatus));
        }
#endif
#if CFG_SUPPORT_WPS2
        if (wextSrchDesiredWPSIE(sme->ie,
                    sme->ie_len,
                    0xDD,
                    (PUINT_8 *)&prDesiredIE)) {
            prGlueInfo->fgWpsActive = TRUE;
            fgCarryWPSIE = TRUE;

            rStatus = kalIoctl(prGlueInfo,
                    wlanoidSetWSCAssocInfo,
                    prDesiredIE,
                    IE_SIZE(prDesiredIE),
                    FALSE,
                    FALSE,
                    FALSE,
                    &u4BufLen);
            if (rStatus != WLAN_STATUS_SUCCESS) {
                DBGLOG(SEC, WARN, ("[WSC] set WSC assoc info error:%lx\n", rStatus));
            }
        }
#endif
    }

    /* clear WSC Assoc IE buffer in case WPS IE is not detected */
    if(fgCarryWPSIE == FALSE) {
        kalMemZero(&prGlueInfo->aucWSCAssocInfoIE, 200);
        prGlueInfo->u2WSCAssocInfoIELen = 0;
    }

    rStatus = kalIoctl(prGlueInfo,
            wlanoidSetAuthMode,
            &eAuthMode,
            sizeof(eAuthMode),
            FALSE,
            FALSE,
            FALSE,
            &u4BufLen);
    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("set auth mode error:%lx\n", rStatus));
    }

    /* Enable the specific AKM suite only. */
    for (i = 0; i < MAX_NUM_SUPPORTED_AKM_SUITES; i++) {
        prEntry = &prGlueInfo->prAdapter->rMib.dot11RSNAConfigAuthenticationSuitesTable[i];

        if (prEntry->dot11RSNAConfigAuthenticationSuite == u4AkmSuite) {
            prEntry->dot11RSNAConfigAuthenticationSuiteEnabled = TRUE;
            //printk("match AuthenticationSuite = 0x%x", u4AkmSuite);
        }
        else {
            prEntry->dot11RSNAConfigAuthenticationSuiteEnabled = FALSE;
        }
    }

    cipher = prGlueInfo->rWpaInfo.u4CipherGroup | prGlueInfo->rWpaInfo.u4CipherPairwise;

    if (prGlueInfo->rWpaInfo.fgPrivacyInvoke) {
        if (cipher & IW_AUTH_CIPHER_CCMP) {
            eEncStatus = ENUM_ENCRYPTION3_ENABLED;
        }
        else if (cipher & IW_AUTH_CIPHER_TKIP) {
            eEncStatus = ENUM_ENCRYPTION2_ENABLED;
        }
        else if (cipher & (IW_AUTH_CIPHER_WEP104 | IW_AUTH_CIPHER_WEP40)) {
            eEncStatus = ENUM_ENCRYPTION1_ENABLED;
        }
        else if (cipher & IW_AUTH_CIPHER_NONE){
            if (prGlueInfo->rWpaInfo.fgPrivacyInvoke)
                eEncStatus = ENUM_ENCRYPTION1_ENABLED;
            else
                eEncStatus = ENUM_ENCRYPTION_DISABLED;
        }
        else {
            eEncStatus = ENUM_ENCRYPTION_DISABLED;
        }
    }
    else {
        eEncStatus = ENUM_ENCRYPTION_DISABLED;
    }
    
    rStatus = kalIoctl(prGlueInfo,
            wlanoidSetEncryptionStatus,
            &eEncStatus,
            sizeof(eEncStatus),
            FALSE,
            FALSE,
            FALSE,
            &u4BufLen);
    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("set encryption mode error:%lx\n", rStatus));
    }

    if (sme->key_len != 0 && prGlueInfo->rWpaInfo.u4WpaVersion == IW_AUTH_WPA_VERSION_DISABLED) {
        P_PARAM_WEP_T prWepKey = (P_PARAM_WEP_T) wepBuf;
        
        kalMemSet(prWepKey, 0, sizeof(prWepKey));
        prWepKey->u4Length = 12 + sme->key_len;
        prWepKey->u4KeyLength = (UINT_32) sme->key_len;
        prWepKey->u4KeyIndex = (UINT_32) sme->key_idx;
        prWepKey->u4KeyIndex |= BIT(31);
        if (prWepKey->u4KeyLength > 32) {
            DBGLOG(REQ, WARN, ("Too long key length (%lu)\n", prWepKey->u4KeyLength));
            return -EINVAL;
        }
        kalMemCopy(prWepKey->aucKeyMaterial, sme->key, prWepKey->u4KeyLength);

        rStatus = kalIoctl(prGlueInfo,
                     wlanoidSetAddWep,
                     prWepKey,
                     prWepKey->u4Length,
                     FALSE,
                     FALSE,
                     TRUE,
                     &u4BufLen);
        
        if (rStatus != WLAN_STATUS_SUCCESS) {
            DBGLOG(INIT, INFO, ("wlanoidSetAddWep fail 0x%lx\n", rStatus));
            return -EFAULT;
        }
    }

    if (sme->ssid_len > 0) {
        /* connect by SSID */
        COPY_SSID(rNewSsid.aucSsid, rNewSsid.u4SsidLen, sme->ssid, sme->ssid_len);

        rStatus = kalIoctl(prGlueInfo,
                wlanoidSetSsid,
                (PVOID) &rNewSsid,
                sizeof(PARAM_SSID_T),
                FALSE,
                FALSE,
                TRUE,
                &u4BufLen);

        if (rStatus != WLAN_STATUS_SUCCESS) {
            DBGLOG(REQ, WARN, ("set SSID:%lx\n", rStatus));
            return -EINVAL;
        }
    }
    else {
        /* connect by BSSID */
        rStatus = kalIoctl(prGlueInfo,
                wlanoidSetBssid,
                (PVOID) sme->bssid,
                MAC_ADDR_LEN,
                FALSE,
                FALSE,
                TRUE,
                &u4BufLen);

        if (rStatus != WLAN_STATUS_SUCCESS) {
            DBGLOG(REQ, WARN, ("set BSSID:%lx\n", rStatus));
            return -EINVAL;
        }
    }
#if 0
    if (sme->bssid != NULL && 1 /* prGlueInfo->fgIsBSSIDSet */) {
        /* connect by BSSID */
        if (sme->ssid_len > 0) {
            P_CONNECTION_SETTINGS_T prConnSettings = NULL;
            prConnSettings = &(prGlueInfo->prAdapter->rWifiVar.rConnSettings);
            //prGlueInfo->fgIsSSIDandBSSIDSet = TRUE;
            COPY_SSID(prConnSettings->aucSSID,
                prConnSettings->ucSSIDLen,
                sme->ssid,
                sme->ssid_len);
        } 
        rStatus = kalIoctl(prGlueInfo,
                    wlanoidSetBssid,
                    (PVOID) sme->bssid,
                    MAC_ADDR_LEN,
                    FALSE,
                    FALSE,
                    TRUE,
                    FALSE,
                    &u4BufLen);

        if (rStatus != WLAN_STATUS_SUCCESS) {
            DBGLOG(REQ, WARN, ("set BSSID:%lx\n", rStatus));
            return -EINVAL;
        }
    }
    else if(sme->ssid_len > 0) {
        /* connect by SSID */
        COPY_SSID(rNewSsid.aucSsid, rNewSsid.u4SsidLen, sme->ssid, sme->ssid_len);

        rStatus = kalIoctl(prGlueInfo,
                wlanoidSetSsid,
                (PVOID) &rNewSsid,
                sizeof(PARAM_SSID_T),
                FALSE,
                FALSE,
                TRUE,
                FALSE,
                &u4BufLen);

        if (rStatus != WLAN_STATUS_SUCCESS) {
            DBGLOG(REQ, WARN, ("set SSID:%lx\n", rStatus));
            return -EINVAL;
        }
    }
#endif
    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to disconnect from
 *        currently connected ESS
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int 
mtk_cfg80211_disconnect (
    struct wiphy *wiphy,
    struct net_device *ndev,
    u16 reason_code
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus;
    UINT_32 u4BufLen;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    rStatus = kalIoctl(prGlueInfo,
        wlanoidSetDisassociate,
        NULL,
        0,
        FALSE,
        FALSE,
        TRUE,
        &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("disassociate error:%lx\n", rStatus));
        return -EFAULT;
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to join an IBSS group
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_join_ibss (
    struct wiphy *wiphy,
    struct net_device *ndev,
    struct cfg80211_ibss_params *params
    )
{
    PARAM_SSID_T rNewSsid;
    P_GLUE_INFO_T prGlueInfo = NULL;
    UINT_32 u4ChnlFreq; /* Store channel or frequency information */
    UINT_32 u4BufLen = 0;
    WLAN_STATUS rStatus;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    /* set channel */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
    if(params->channel) {
        u4ChnlFreq = nicChannelNum2Freq(params->channel->hw_value);
#else
    if(params->channel_fixed) {
        u4ChnlFreq = params->chandef.center_freq1;
#endif

        rStatus = kalIoctl(prGlueInfo,
                           wlanoidSetFrequency,
                           &u4ChnlFreq,
                           sizeof(u4ChnlFreq),
                           FALSE,
                           FALSE,
                           FALSE,
                           &u4BufLen);
        if (rStatus != WLAN_STATUS_SUCCESS) {
            return -EFAULT;
        }
    }

    /* set SSID */
    kalMemCopy(rNewSsid.aucSsid, params->ssid, params->ssid_len);
    rStatus = kalIoctl(prGlueInfo,
            wlanoidSetSsid,
            (PVOID) &rNewSsid,
            sizeof(PARAM_SSID_T),
            FALSE,
            FALSE,
            TRUE,
            &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("set SSID:%lx\n", rStatus));
        return -EFAULT;
    }

    return 0;


    return -EINVAL;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to leave from IBSS group
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_leave_ibss (
    struct wiphy *wiphy,
    struct net_device *ndev
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus;
    UINT_32 u4BufLen;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    rStatus = kalIoctl(prGlueInfo,
        wlanoidSetDisassociate,
        NULL,
        0,
        FALSE,
        FALSE,
        TRUE,
        &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("disassociate error:%lx\n", rStatus));
        return -EFAULT;
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to configure 
 *        WLAN power managemenet
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_set_power_mgmt (
    struct wiphy *wiphy,
    struct net_device *ndev,
    bool enabled,
    int timeout
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus;
    UINT_32 u4BufLen;
    PARAM_POWER_MODE ePowerMode;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    if(enabled) {
        if(timeout == -1) {
            ePowerMode = Param_PowerModeFast_PSP;
        }
        else {
            ePowerMode = Param_PowerModeMAX_PSP;
        }
    }
    else {
        ePowerMode = Param_PowerModeCAM;
    }

    rStatus = kalIoctl(prGlueInfo,
        wlanoidSet802dot11PowerSaveProfile,
        &ePowerMode,
        sizeof(ePowerMode),
        FALSE,
        FALSE,
        TRUE,
        &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("set_power_mgmt error:%lx\n", rStatus));
        return -EFAULT;
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to cache
 *        a PMKID for a BSSID
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_set_pmksa (
    struct wiphy *wiphy,
    struct net_device *ndev,
    struct cfg80211_pmksa *pmksa
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS    rStatus;
    UINT_32        u4BufLen;
    P_PARAM_PMKID_T  prPmkid;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    prPmkid =(P_PARAM_PMKID_T)kalMemAlloc(8 + sizeof(PARAM_BSSID_INFO_T), VIR_MEM_TYPE);
    if (!prPmkid) {
        DBGLOG(INIT, INFO, ("Can not alloc memory for IW_PMKSA_ADD\n"));
        return -ENOMEM;
    }
    
    prPmkid->u4Length = 8 + sizeof(PARAM_BSSID_INFO_T);
    prPmkid->u4BSSIDInfoCount = 1;
    kalMemCopy(prPmkid->arBSSIDInfo->arBSSID, pmksa->bssid, 6);
    kalMemCopy(prPmkid->arBSSIDInfo->arPMKID, pmksa->pmkid, IW_PMKID_LEN);
    
    rStatus = kalIoctl(prGlueInfo,
                 wlanoidSetPmkid,
                 prPmkid,
                 sizeof(PARAM_PMKID_T),
                 FALSE,
                 FALSE,
                 FALSE,
                 &u4BufLen);
    
    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(INIT, INFO, ("add pmkid error:%lx\n", rStatus));
    }
    kalMemFree(prPmkid, VIR_MEM_TYPE, 8 + sizeof(PARAM_BSSID_INFO_T));

    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to remove
 *        a cached PMKID for a BSSID
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_del_pmksa (
    struct wiphy *wiphy,
    struct net_device *ndev,
    struct cfg80211_pmksa *pmksa
    )
{

    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to flush
 *        all cached PMKID
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_flush_pmksa (
    struct wiphy *wiphy,
    struct net_device *ndev
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS    rStatus;
    UINT_32        u4BufLen;
    P_PARAM_PMKID_T  prPmkid;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    prPmkid =(P_PARAM_PMKID_T)kalMemAlloc(8, VIR_MEM_TYPE);
    if (!prPmkid) {
        DBGLOG(INIT, INFO, ("Can not alloc memory for IW_PMKSA_FLUSH\n"));
        return -ENOMEM;
    }
    
    prPmkid->u4Length = 8;
    prPmkid->u4BSSIDInfoCount = 0;
    
    rStatus = kalIoctl(prGlueInfo,
                 wlanoidSetPmkid,
                 prPmkid,
                 sizeof(PARAM_PMKID_T),
                 FALSE,
                 FALSE,
                 FALSE,
                 &u4BufLen);
    
    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(INIT, INFO, ("flush pmkid error:%lx\n", rStatus));
    }
    kalMemFree(prPmkid, VIR_MEM_TYPE, 8);

    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for setting the rekey data
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_set_rekey_data (
    struct wiphy *wiphy, 
    struct net_device *dev,
    struct cfg80211_gtk_rekey_data *data)
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS    rStatus;
    UINT_32        u4BufLen;
    P_PARAM_GTK_REKEY_DATA  prGtkData;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    prGtkData =(P_PARAM_GTK_REKEY_DATA)kalMemAlloc(sizeof(PARAM_GTK_REKEY_DATA), VIR_MEM_TYPE);
    if (!prGtkData) {
        DBGLOG(INIT, INFO, ("Can not alloc memory for PARAM_GTK_REKEY_DATA\n"));
        return -ENOMEM;
    }

    DBGLOG(RSN, TRACE, ("cfg80211_set_rekey_data!\n"));
    
    kalMemCopy(prGtkData, data, sizeof(PARAM_GTK_REKEY_DATA));
    
    rStatus = kalIoctl(prGlueInfo,
                 wlanoidSetGtkRekeyData,
                 prGtkData,
                 sizeof(PARAM_GTK_REKEY_DATA),
                 FALSE,
                 FALSE,
                 TRUE,
                 &u4BufLen);
    
    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(INIT, INFO, ("set GTK rekey data error:%lx\n", rStatus));
    }
    kalMemFree(prGtkData, VIR_MEM_TYPE, sizeof(PARAM_GTK_REKEY_DATA));

    return 0;
}


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
    )
{
#if 0
    P_MSG_P2P_MGMT_FRAME_REGISTER_T prMgmtFrameRegister = (P_MSG_P2P_MGMT_FRAME_REGISTER_T)NULL;
#endif
    P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T)NULL;

    do {

        DBGLOG(INIT, TRACE, ("mtk_cfg80211_mgmt_frame_register\n"));

        prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);

        switch (frame_type) {
        case MAC_FRAME_PROBE_REQ:
            if (reg) {
                prGlueInfo->u4OsMgmtFrameFilter |= PARAM_PACKET_FILTER_PROBE_REQ;
                DBGLOG(INIT, TRACE, ("Open packet filer probe request\n"));
            }
            else {
                prGlueInfo->u4OsMgmtFrameFilter &= ~PARAM_PACKET_FILTER_PROBE_REQ;
                DBGLOG(INIT, TRACE, ("Close packet filer probe request\n"));
            }
            break;
        case MAC_FRAME_ACTION:
            if (reg) {
                prGlueInfo->u4OsMgmtFrameFilter |= PARAM_PACKET_FILTER_ACTION_FRAME;
                DBGLOG(INIT, TRACE, ("Open packet filer action frame.\n"));
            }
            else {
                prGlueInfo->u4OsMgmtFrameFilter &= ~PARAM_PACKET_FILTER_ACTION_FRAME;
                DBGLOG(INIT, TRACE, ("Close packet filer action frame.\n"));
            }
            break;
        default:
                printk("Ask frog to add code for mgmt:%x\n", frame_type);
                break;
        }

        if (prGlueInfo->prAdapter != NULL){

            prGlueInfo->u4Flag |= GLUE_FLAG_FRAME_FILTER_AIS;

            /* wake up main thread */
            wake_up_interruptible(&prGlueInfo->waitq);

           if (in_interrupt()) {
              DBGLOG(INIT, TRACE, ("It is in interrupt level\n"));
           }
        }

#if 0


        prMgmtFrameRegister = (P_MSG_P2P_MGMT_FRAME_REGISTER_T)cnmMemAlloc(prGlueInfo->prAdapter, 
                                                                    RAM_TYPE_MSG, 
                                                                    sizeof(MSG_P2P_MGMT_FRAME_REGISTER_T));

        if (prMgmtFrameRegister == NULL) {
            ASSERT(FALSE);
            break;
        }

        prMgmtFrameRegister->rMsgHdr.eMsgId = MID_MNY_P2P_MGMT_FRAME_REGISTER;

        prMgmtFrameRegister->u2FrameType = frame_type;
        prMgmtFrameRegister->fgIsRegister = reg;

        mboxSendMsg(prGlueInfo->prAdapter,
                                    MBOX_ID_0,
                                    (P_MSG_HDR_T)prMgmtFrameRegister,
                                    MSG_SEND_METHOD_BUF);

#endif

    } while (FALSE);

    return;
} /* mtk_cfg80211_mgmt_frame_register */


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to stay on a 
 *        specified channel
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
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
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    INT_32 i4Rslt = -EINVAL;
    P_MSG_REMAIN_ON_CHANNEL_T prMsgChnlReq = (P_MSG_REMAIN_ON_CHANNEL_T)NULL;

    do {
        if ((wiphy == NULL) 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
                || (wdev == NULL)
#else
                || (ndev == NULL)
#endif
                || (chan == NULL)
                || (cookie == NULL)) {
            break;
        }

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

#if 1
    printk("--> %s()\n", __func__);
#endif

        *cookie = prGlueInfo->u8Cookie++;

        prMsgChnlReq = cnmMemAlloc(prGlueInfo->prAdapter, RAM_TYPE_MSG, sizeof(MSG_REMAIN_ON_CHANNEL_T));

        if (prMsgChnlReq == NULL) {
            ASSERT(FALSE);
            i4Rslt = -ENOMEM;
            break;
        }

        prMsgChnlReq->rMsgHdr.eMsgId = MID_MNY_AIS_REMAIN_ON_CHANNEL;
        prMsgChnlReq->u8Cookie = *cookie;
        prMsgChnlReq->u4DurationMs = duration;

        prMsgChnlReq->ucChannelNum = nicFreq2ChannelNum(chan->center_freq * 1000);

        switch (chan->band) {
        case IEEE80211_BAND_2GHZ:
            prMsgChnlReq->eBand = BAND_2G4;
            break;
        case IEEE80211_BAND_5GHZ:
            prMsgChnlReq->eBand = BAND_5G;
            break;
        default:
            prMsgChnlReq->eBand = BAND_2G4;
            break;
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
        switch (channel_type) {
        case NL80211_CHAN_NO_HT:
            prMsgChnlReq->eSco = CHNL_EXT_SCN;
            break;
        case NL80211_CHAN_HT20:
            prMsgChnlReq->eSco = CHNL_EXT_SCN;
            break;
        case NL80211_CHAN_HT40MINUS:
            prMsgChnlReq->eSco = CHNL_EXT_SCA;
            break;
        case NL80211_CHAN_HT40PLUS:
            prMsgChnlReq->eSco = CHNL_EXT_SCB;
            break;
        default:
            ASSERT(FALSE);
            prMsgChnlReq->eSco = CHNL_EXT_SCN;
            break;
        }
#else
        prMsgChnlReq->eSco = CHNL_EXT_SCN;
#endif

        mboxSendMsg(prGlueInfo->prAdapter,
                            MBOX_ID_0,
                            (P_MSG_HDR_T)prMsgChnlReq,
                            MSG_SEND_METHOD_BUF);

        i4Rslt = 0;
    } while (FALSE);

    return -EINVAL;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to cancel staying 
 *        on a specified channel
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_cancel_remain_on_channel (
    struct wiphy *wiphy,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
    struct wireless_dev *wdev,
#else
    struct net_device *ndev,
#endif
    u64 cookie
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    INT_32 i4Rslt = -EINVAL;
    P_MSG_CANCEL_REMAIN_ON_CHANNEL_T prMsgChnlAbort = (P_MSG_CANCEL_REMAIN_ON_CHANNEL_T)NULL;

    do {
        if ((wiphy == NULL) 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
                || (wdev == NULL)
#else
                || (ndev == NULL)
#endif
                ) {
            break;
        }

        prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
        ASSERT(prGlueInfo);

        prMsgChnlAbort = cnmMemAlloc(prGlueInfo->prAdapter, RAM_TYPE_MSG, sizeof(MSG_CANCEL_REMAIN_ON_CHANNEL_T));

        if (prMsgChnlAbort == NULL) {
            ASSERT(FALSE);
            i4Rslt = -ENOMEM;
            break;
        }

        prMsgChnlAbort->rMsgHdr.eMsgId = MID_MNY_AIS_CANCEL_REMAIN_ON_CHANNEL;
        prMsgChnlAbort->u8Cookie = cookie;

        mboxSendMsg(prGlueInfo->prAdapter,
                                    MBOX_ID_0,
                                    (P_MSG_HDR_T)prMsgChnlAbort,
                                    MSG_SEND_METHOD_BUF);

        i4Rslt = 0;
    } while (FALSE);

    return i4Rslt;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to send a management frame
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
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
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    INT_32 i4Rslt = -EINVAL;
    P_MSG_MGMT_TX_REQUEST_T prMsgTxReq = (P_MSG_MGMT_TX_REQUEST_T)NULL;
    P_MSDU_INFO_T prMgmtFrame = (P_MSDU_INFO_T)NULL;
    PUINT_8 pucFrameBuf = (PUINT_8)NULL;

    do {
        if ((wiphy == NULL) 
                || (buf == NULL) 
                || (len == 0) 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
                || (wdev == NULL)
#else
                || (ndev == NULL)
#endif
                || (cookie == NULL)) {
            break;
        }

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

        *cookie = prGlueInfo->u8Cookie++;

        /* Channel & Channel Type & Wait time are ignored. */
        prMsgTxReq = cnmMemAlloc(prGlueInfo->prAdapter, RAM_TYPE_MSG, sizeof(MSG_MGMT_TX_REQUEST_T));

        if (prMsgTxReq == NULL) {
            ASSERT(FALSE);
            i4Rslt = -ENOMEM;
            break;
        }

        prMsgTxReq->fgNoneCckRate = FALSE;
        prMsgTxReq->fgIsWaitRsp = TRUE;

        prMgmtFrame = cnmMgtPktAlloc(prGlueInfo->prAdapter, (UINT_32)(len + MAC_TX_RESERVED_FIELD));

        if ((prMsgTxReq->prMgmtMsduInfo = prMgmtFrame) == NULL) {
            ASSERT(FALSE);
            i4Rslt = -ENOMEM;
            break;
        }

        prMsgTxReq->u8Cookie = *cookie;
        prMsgTxReq->rMsgHdr.eMsgId = MID_MNY_AIS_MGMT_TX;

        pucFrameBuf = (PUINT_8)((UINT_32)prMgmtFrame->prPacket + MAC_TX_RESERVED_FIELD);

        kalMemCopy(pucFrameBuf, buf, len);

        prMgmtFrame->u2FrameLength = len;

        mboxSendMsg(prGlueInfo->prAdapter,
                            MBOX_ID_0,
                            (P_MSG_HDR_T)prMsgTxReq,
                            MSG_SEND_METHOD_BUF);

        i4Rslt = 0;
    } while (FALSE);

    if ((i4Rslt != 0) && (prMsgTxReq != NULL)) {
        if (prMsgTxReq->prMgmtMsduInfo != NULL) {
            cnmMgtPktFree(prGlueInfo->prAdapter, prMsgTxReq->prMgmtMsduInfo);
        }

        cnmMemFree(prGlueInfo->prAdapter, prMsgTxReq);
    }

    return i4Rslt;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for requesting to cancel the wait time
 *        from transmitting a management frame on another channel
 *
 * @param 
 *
 * @retval 0:       successful
 *         others:  failure
 */
/*----------------------------------------------------------------------------*/
int
mtk_cfg80211_mgmt_tx_cancel_wait (
    struct wiphy *wiphy,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
    struct wireless_dev *wdev,
#else
    struct net_device *ndev,
#endif
    u64 cookie
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

#if 1
    printk("--> %s()\n", __func__);
#endif

    /* not implemented */

    return -EINVAL;
}


#if CONFIG_NL80211_TESTMODE

#if CFG_SUPPORT_WAPI
int
mtk_cfg80211_testmode_set_key_ext(
    IN struct wiphy *wiphy,
    IN void *data,
    IN int len)
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    P_NL80211_DRIVER_SET_KEY_EXTS prParams = (P_NL80211_DRIVER_SET_KEY_EXTS)NULL;
    struct iw_encode_exts *prIWEncExt = (struct iw_encode_exts *)NULL;
    WLAN_STATUS rstatus = WLAN_STATUS_SUCCESS;
    int     fgIsValid = 0;
    UINT_32 u4BufLen = 0;
    
    P_PARAM_WPI_KEY_T prWpiKey = (P_PARAM_WPI_KEY_T) keyStructBuf;
    memset(keyStructBuf, 0, sizeof(keyStructBuf));

    ASSERT(wiphy);
    
    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    
#if 1
        printk("--> %s()\n", __func__);
#endif

    if(data && len) {
        prParams = (P_NL80211_DRIVER_SET_KEY_EXTS)data;
    }
    
    if(prParams) {
        prIWEncExt = (struct iw_encode_exts *) &prParams->ext;
    }

    if (prIWEncExt->alg == IW_ENCODE_ALG_SMS4) {
        /* KeyID */
        prWpiKey->ucKeyID = prParams->key_index;
        prWpiKey->ucKeyID --;
        if (prWpiKey->ucKeyID > 1) {
            /* key id is out of range */
            //printk(KERN_INFO "[wapi] add key error: key_id invalid %d\n", prWpiKey->ucKeyID);
            return -EINVAL;
        }

        if (prIWEncExt->key_len != 32) {
            /* key length not valid */
            //printk(KERN_INFO "[wapi] add key error: key_len invalid %d\n", prIWEncExt->key_len);
            return -EINVAL;
        }

        //printk(KERN_INFO "[wapi] %d ext_flags %d\n", prEnc->flags, prIWEncExt->ext_flags);

        if (prIWEncExt->ext_flags & IW_ENCODE_EXT_GROUP_KEY) {
            prWpiKey->eKeyType = ENUM_WPI_GROUP_KEY;
            prWpiKey->eDirection = ENUM_WPI_RX;
        }
        else if (prIWEncExt->ext_flags & IW_ENCODE_EXT_SET_TX_KEY) {
            prWpiKey->eKeyType = ENUM_WPI_PAIRWISE_KEY;
            prWpiKey->eDirection = ENUM_WPI_RX_TX;
        }

//#if CFG_SUPPORT_WAPI
        //handle_sec_msg_final(prIWEncExt->key, 32, prIWEncExt->key, NULL);
//#endif
        /* PN */
        memcpy(prWpiKey->aucPN, prIWEncExt->tx_seq, IW_ENCODE_SEQ_MAX_SIZE * 2);

        /* BSSID */
        memcpy(prWpiKey->aucAddrIndex, prIWEncExt->addr, 6);

        memcpy(prWpiKey->aucWPIEK, prIWEncExt->key, 16);
        prWpiKey->u4LenWPIEK = 16;

        memcpy(prWpiKey->aucWPICK, &prIWEncExt->key[16], 16);
        prWpiKey->u4LenWPICK = 16;

        rstatus = kalIoctl(prGlueInfo,
                     wlanoidSetWapiKey,
                     prWpiKey,
                     sizeof(PARAM_WPI_KEY_T),
                     FALSE,
                     FALSE,
                     TRUE,
                     &u4BufLen);

        if (rstatus != WLAN_STATUS_SUCCESS) {
            //printk(KERN_INFO "[wapi] add key error:%lx\n", rStatus);
            fgIsValid = -EFAULT;
        }

    }
    return fgIsValid;
}
#endif


int
mtk_cfg80211_testmode_sw_cmd(
    IN struct wiphy *wiphy,
    IN void *data,
    IN int len)
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    P_NL80211_DRIVER_SW_CMD_PARAMS prParams = (P_NL80211_DRIVER_SW_CMD_PARAMS)NULL;
    WLAN_STATUS rstatus = WLAN_STATUS_SUCCESS;
    int     fgIsValid = 0;
    UINT_32 u4SetInfoLen = 0;

    ASSERT(wiphy);

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);

#if 1
        printk("--> %s()\n", __func__);
#endif

    if(data && len)
        prParams = (P_NL80211_DRIVER_SW_CMD_PARAMS)data;

    if(prParams) {
        if(prParams->set == 1){
            rstatus = kalIoctl(prGlueInfo,
                    (PFN_OID_HANDLER_FUNC)wlanoidSetSwCtrlWrite,
                    &prParams->adr,
                    (UINT_32)8,
                    FALSE,
                    FALSE,
                    TRUE,
                    &u4SetInfoLen);
        }
    }

    if (WLAN_STATUS_SUCCESS != rstatus) {
        fgIsValid = -EFAULT;
    }
 
    return fgIsValid;
}

int mtk_cfg80211_testmode_cmd(
    IN struct wiphy *wiphy,
    IN void *data,
    IN int len
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    P_NL80211_DRIVER_TEST_MODE_PARAMS prParams = (P_NL80211_DRIVER_TEST_MODE_PARAMS)NULL;
    BOOLEAN fgIsValid = 0;

    ASSERT(wiphy);

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);

#if 1
    printk("--> %s()\n", __func__);
#endif

    if(data && len)
        prParams = (P_NL80211_DRIVER_TEST_MODE_PARAMS)data;
        
    /* Clear the version byte */
    prParams->index = prParams->index & ~ BITS(24,31);
    
    if(prParams){
        switch(prParams->index){
            case 1: /* SW cmd */
                if(mtk_cfg80211_testmode_sw_cmd(wiphy, data, len))
                    fgIsValid = TRUE;
                break;
            case 2: /* WAPI */
#if CFG_SUPPORT_WAPI
                if(mtk_cfg80211_testmode_set_key_ext(wiphy, data, len))
                    fgIsValid = TRUE;
#endif
                break;
            default:
                fgIsValid = TRUE;
                break;
        }
    }


    return fgIsValid;
}
#endif



int
mtk_cfg80211_sched_scan_start(
    IN struct wiphy *wiphy,
    IN struct net_device *ndev,
    IN struct cfg80211_sched_scan_request *request
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus;
    UINT_32 i, u4BufLen;
    P_PARAM_SCHED_SCAN_REQUEST prSchedScanRequest;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    /* check if there is any pending scan/sched_scan not yet finished */
    if(prGlueInfo->prScanRequest != NULL || prGlueInfo->prSchedScanRequest != NULL) {
        return -EBUSY;
    }
    else if(request == NULL || request->n_match_sets > CFG_SCAN_SSID_MATCH_MAX_NUM) {
        /* invalid scheduled scan request */
        return -EINVAL;
    }
    else if(!request->n_ssids || !request->n_match_sets) {
        /* invalid scheduled scan request */
        return -EINVAL;
    }

    prSchedScanRequest = (P_PARAM_SCHED_SCAN_REQUEST) kalMemAlloc(sizeof(PARAM_SCHED_SCAN_REQUEST), VIR_MEM_TYPE);
    if(prSchedScanRequest == NULL) {
        return -ENOMEM;
    }

    prSchedScanRequest->u4SsidNum = request->n_match_sets;
    for (i = 0 ; i < request->n_match_sets ; i++) {
        if(request->match_sets == NULL || &(request->match_sets[i]) == NULL) {
            prSchedScanRequest->arSsid[i].u4SsidLen = 0;
        }
        else {
            COPY_SSID(prSchedScanRequest->arSsid[i].aucSsid,
                    prSchedScanRequest->arSsid[i].u4SsidLen,
                    request->match_sets[i].ssid.ssid,
                    request->match_sets[i].ssid.ssid_len);
        }
    }

    prSchedScanRequest->u4IELength = request->ie_len;
    if(request->ie_len > 0) {
        prSchedScanRequest->pucIE = (PUINT_8)(request->ie);
    }

    prSchedScanRequest->u2ScanInterval = (UINT_16)(request->interval);

    rStatus = kalIoctl(prGlueInfo,
            wlanoidSetStartSchedScan,
            prSchedScanRequest,
            sizeof(PARAM_SCHED_SCAN_REQUEST),
            FALSE,
            FALSE,
            TRUE,
            &u4BufLen);

    kalMemFree(prSchedScanRequest, VIR_MEM_TYPE, sizeof(PARAM_SCHED_SCAN_REQUEST));

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("scheduled scan error:%lx\n", rStatus));
        return -EINVAL;
    }

    prGlueInfo->prSchedScanRequest = request;

    return 0;
}


int
mtk_cfg80211_sched_scan_stop(
    IN struct wiphy *wiphy,
    IN struct net_device *ndev
    )
{
    P_GLUE_INFO_T prGlueInfo = NULL;
    WLAN_STATUS rStatus;
    UINT_32 u4BufLen;

    prGlueInfo = (P_GLUE_INFO_T) wiphy_priv(wiphy);
    ASSERT(prGlueInfo);

    /* check if there is any pending scan/sched_scan not yet finished */
    if(prGlueInfo->prSchedScanRequest == NULL) {
        return -EBUSY;
    }

    rStatus = kalIoctl(prGlueInfo,
            wlanoidSetStopSchedScan,
            NULL,
            0,
            FALSE,
            FALSE,
            TRUE,
            &u4BufLen);

    if (rStatus != WLAN_STATUS_SUCCESS) {
        DBGLOG(REQ, WARN, ("scheduled scan error:%lx\n", rStatus));
        return -EINVAL;
    }

    return 0;
}

