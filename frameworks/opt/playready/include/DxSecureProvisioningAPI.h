#ifndef _DX_SECURE_PR_PROVISIONING_API_H_
#define _DX_SECURE_PR_PROVISIONING_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmDefines.h"

//
//
//	This API will save the provided CEK into the SFS,
//	and decrypt the credentials package to be used for PlayReady Provisioning
//
//	Parameters:
//	pCek : ~CEK (derived from CEK, size is 16 bytes)
//	return value:
//	Success : DX_SUCCESS
//	Failure  : Specific failure error
//
DX_FUNC  DRM_C_API DxStatus SetProvisioningCEK (const DxUint8 *pCek);

//
//	This API initiates the provisioning process.
//  It uses the CEK provided in SetProvisioningCEK and the Decrypted credentials package.
//	return value:
//	Success : DX_SUCCESS
//	Failure  : Specific failure error
//
DX_FUNC  DRM_C_API DxStatus StartSecuredProvisioning ();

//
//	This API verifies that the CECK was stored correctly to the SFS
//
//	return value:
//	True : CEK stored successfully
//	False : CEK storage failed
//
DX_FUNC  DRM_C_API DxBool VerifyStoredCek (DxUint8 *encStr, DxUint32 size);


//
//	This API verifies that the provisioning process was successful.
//
//	return value:
//	True : provisioning successful
//	False : provisioning failed
//
DX_FUNC  DRM_C_API DxBool VerifySecuredProvisioning ();


#ifdef __cplusplus
}
#endif

#endif
