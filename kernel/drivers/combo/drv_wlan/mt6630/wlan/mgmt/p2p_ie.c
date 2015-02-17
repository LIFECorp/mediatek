#include "precomp.h"



UINT_32
p2pCalculate_IEForAssocReq (
    IN P_ADAPTER_T prAdapter,
    IN UINT_8 ucBssIndex,
    IN P_STA_RECORD_T prStaRec
    )
{
    P_P2P_ROLE_FSM_INFO_T prP2pRoleFsmInfo = (P_P2P_ROLE_FSM_INFO_T)NULL;
    P_BSS_INFO_T prP2pBssInfo = (P_BSS_INFO_T)NULL;
    P_P2P_CONNECTION_REQ_INFO_T prConnReqInfo = (P_P2P_CONNECTION_REQ_INFO_T)NULL;
    UINT_32 u4RetValue = 0;

    do {
        ASSERT_BREAK((prStaRec != NULL) && (prAdapter != NULL));

        prP2pBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

        prP2pRoleFsmInfo = P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter, (UINT_8)prP2pBssInfo->u4PrivateData);
            
        prConnReqInfo = &(prP2pRoleFsmInfo->rConnReqInfo);

        u4RetValue = prConnReqInfo->u4BufLength;

		/* ADD WMM Information Element */
        u4RetValue += (ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO);    

        /* ADD HT Capability */
        if((prAdapter->rWifiVar.ucAvailablePhyTypeSet & PHY_TYPE_SET_802_11N) && 
            (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11N)) {
            u4RetValue += (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP);
        }
        
#if CFG_SUPPORT_802_11AC
        /* ADD VHT Capability */
        if((prAdapter->rWifiVar.ucAvailablePhyTypeSet & PHY_TYPE_SET_802_11AC) && 
            (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11AC)) {
            u4RetValue += (ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_CAP);
        }
#endif
    } while (FALSE);

    return u4RetValue;
} /* p2pCalculate_IEForAssocReq */



/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to generate P2P IE for Beacon frame.
*
* @param[in] prMsduInfo             Pointer to the composed MSDU_INFO_T.
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID
p2pGenerate_IEForAssocReq (
    IN P_ADAPTER_T prAdapter,
    IN P_MSDU_INFO_T prMsduInfo
    )
{
    P_BSS_INFO_T prBssInfo = (P_BSS_INFO_T)NULL;
    P_P2P_ROLE_FSM_INFO_T prP2pRoleFsmInfo = (P_P2P_ROLE_FSM_INFO_T)NULL;
    P_P2P_CONNECTION_REQ_INFO_T prConnReqInfo = (P_P2P_CONNECTION_REQ_INFO_T)NULL;
    PUINT_8 pucIEBuf = (PUINT_8)NULL;

    do {
        ASSERT_BREAK((prAdapter != NULL) && (prMsduInfo != NULL));

        prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);

        prP2pRoleFsmInfo = P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter, (UINT_8)prBssInfo->u4PrivateData);

        prConnReqInfo = &(prP2pRoleFsmInfo->rConnReqInfo);

        pucIEBuf = (PUINT_8)((UINT_32)prMsduInfo->prPacket + (UINT_32)prMsduInfo->u2FrameLength);

        kalMemCopy(pucIEBuf, prConnReqInfo->aucIEBuf, prConnReqInfo->u4BufLength);

        prMsduInfo->u2FrameLength += prConnReqInfo->u4BufLength;

        /* Add WMM IE */
        mqmGenerateWmmInfoIE(prAdapter,prMsduInfo);

        /* Add HT IE */
        rlmReqGenerateHtCapIE(prAdapter,prMsduInfo);

#if CFG_SUPPORT_802_11AC
        /* Add VHT IE */
        rlmReqGenerateVhtCapIE(prAdapter, prMsduInfo);
#endif
    } while (FALSE);

    return;

} /* p2pGenerate_IEForAssocReq */


