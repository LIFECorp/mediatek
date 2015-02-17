


typedef enum _ENUM_P2P_DEV_STATE_T {
    P2P_DEV_STATE_IDLE = 0,
    P2P_DEV_STATE_SCAN,
    P2P_DEV_STATE_REQING_CHANNEL,
    P2P_DEV_STATE_CHNL_ON_HAND, 
    P2P_DEV_STATE_OFF_CHNL_TX, /* Requesting Channel to Send Specific Frame. */
    P2P_DEV_STATE_NUM
} ENUM_P2P_DEV_STATE_T, *P_ENUM_P2P_DEV_STATE_T;




/*-------------------- EVENT MESSAGE ---------------------*/
typedef struct _MSG_P2P_SCAN_REQUEST_T {
    MSG_HDR_T       rMsgHdr;    /* Must be the first member */
    UINT_8           ucBssIdx;
    ENUM_SCAN_TYPE_T eScanType;
    P_P2P_SSID_STRUCT_T prSSID;
    INT_32 i4SsidNum;
    UINT_32 u4NumChannel;
    PUINT_8 pucIEBuf;
    UINT_32 u4IELen;
    BOOLEAN fgIsAbort;
    RF_CHANNEL_INFO_T arChannelListInfo[1];
} MSG_P2P_SCAN_REQUEST_T, *P_MSG_P2P_SCAN_REQUEST_T;

typedef struct _MSG_P2P_CHNL_REQUEST_T {
    MSG_HDR_T       rMsgHdr;    /* Must be the first member */
    UINT_64 u8Cookie;
    UINT_32 u4Duration;
    ENUM_CHNL_EXT_T eChnlSco;
    RF_CHANNEL_INFO_T rChannelInfo;
    ENUM_CH_REQ_TYPE_T eChnlReqType;
} MSG_P2P_CHNL_REQUEST_T, *P_MSG_P2P_CHNL_REQUEST_T;


typedef struct _MSG_P2P_MGMT_TX_REQUEST_T {
    MSG_HDR_T rMsgHdr;
    UINT_8 ucBssIdx;
    P_MSDU_INFO_T prMgmtMsduInfo;
    UINT_64 u8Cookie;                   /* For indication. */
    BOOLEAN fgNoneCckRate;
    BOOLEAN fgIsOffChannel;
    RF_CHANNEL_INFO_T rChannelInfo;     /* Off channel TX. */
    ENUM_CHNL_EXT_T eChnlExt;
    BOOLEAN fgIsWaitRsp;
} MSG_P2P_MGMT_TX_REQUEST_T, *P_MSG_P2P_MGMT_TX_REQUEST_T;


/*-------------------- P2P FSM ACTION STRUCT ---------------------*/


typedef struct _P2P_OFF_CHNL_TX_REQ_INFO_T {
    LINK_ENTRY_T rLinkEntry;
    P_MSDU_INFO_T prMgmtTxMsdu;
    BOOLEAN fgNoneCckRate;
    RF_CHANNEL_INFO_T rChannelInfo;     /* Off channel TX. */
    ENUM_CHNL_EXT_T eChnlExt;
    BOOLEAN fgIsWaitRsp;    /* See if driver should keep at the same channel. */
} P2P_OFF_CHNL_TX_REQ_INFO_T, *P_P2P_OFF_CHNL_TX_REQ_INFO_T;

typedef struct _P2P_MGMT_TX_REQ_INFO_T {
    LINK_T rP2pTxReqLink;
    P_MSDU_INFO_T prMgmtTxMsdu;
    BOOLEAN fgIsWaitRsp;
} P2P_MGMT_TX_REQ_INFO_T, *P_P2P_MGMT_TX_REQ_INFO_T;

struct _P2P_DEV_FSM_INFO_T {
    UINT_8 ucBssIndex;
    /* State related. */
    ENUM_P2P_DEV_STATE_T    eCurrentState;

    /* Channel related. */
    P2P_CHNL_REQ_INFO_T rChnlReqInfo;

    /* Scan related. */
    P2P_SCAN_REQ_INFO_T rScanReqInfo;

    /* Mgmt tx related. */
    P2P_MGMT_TX_REQ_INFO_T rMgmtTxInfo;

      /* FSM Timer */
    TIMER_T rP2pFsmTimeoutTimer;

    /* Packet filter for P2P module. */
    UINT_32 u4P2pPacketFilter;

};




typedef struct _MSG_P2P_NETDEV_REGISTER_T {
    MSG_HDR_T       rMsgHdr;    /* Must be the first member */
    BOOLEAN         fgIsEnable;
    UINT_8          ucMode;
} MSG_P2P_NETDEV_REGISTER_T, *P_MSG_P2P_NETDEV_REGISTER_T;


/*========================= Initial ============================*/


UINT_8
p2pDevFsmInit (
    IN P_ADAPTER_T prAdapter
    );

VOID
p2pDevFsmUninit (
    IN P_ADAPTER_T prAdapter
    );

/*========================= FUNCTIONs ============================*/

VOID
p2pDevFsmStateTransition(
    IN P_ADAPTER_T prAdapter,
    IN P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo,
    IN ENUM_P2P_DEV_STATE_T eNextState
    );

VOID
p2pDevFsmRunEventAbort(
    IN P_ADAPTER_T prAdapter,
    IN P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo
    );

VOID
p2pDevFsmRunEventTimeout(
    IN P_ADAPTER_T prAdapter,
    IN UINT_32 u4Param
    );

/*================ Message Event =================*/
VOID
p2pDevFsmRunEventScanRequest(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );

VOID
p2pDevFsmRunEventScanDone(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr,
    IN P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo
    );

VOID
p2pDevFsmRunEventChannelRequest(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );

VOID
p2pDevFsmRunEventChannelAbort(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );

VOID
p2pDevFsmRunEventChnlGrant(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr,
    IN P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo
    );

VOID
p2pDevFsmRunEventMgmtTx(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );

WLAN_STATUS
p2pDevFsmRunEventMgmtFrameTxDone(
    IN P_ADAPTER_T prAdapter,
    IN P_MSDU_INFO_T prMsduInfo,
    IN ENUM_TX_RESULT_CODE_T rTxDoneStatus
    );


VOID 
p2pDevFsmRunEventMgmtFrameRegister(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );


/////////////////////////////////

VOID
p2pFsmRunEventScanRequest(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );


VOID
p2pFsmRunEventScanDone(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );

VOID
p2pFsmRunEventChGrant (
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );

VOID
p2pFsmRunEventNetDeviceRegister(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );
    
VOID
p2pFsmRunEventUpdateMgmtFrame(
    IN P_ADAPTER_T prAdapter,
    IN P_MSG_HDR_T prMsgHdr
    );

