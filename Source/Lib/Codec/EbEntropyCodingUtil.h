/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbEntropyCodingUtil_h
#define EbEntropyCodingUtil_h

#include "EbDefinitions.h"
#include "EbEntropyCodingObject.h"
#include "EbCodingUnit.h"
#include "EbPredictionUnit.h"
#include "EbPictureBufferDesc.h"
#include "EbSequenceControlSet.h"
#include "EbPictureControlSet.h"
#include "EbCabacContextModel.h"
#include "EbModeDecision.h"
#include "EbIntraPrediction.h"
#include "EbBitstreamUnit.h"
#include "EbPacketizationProcess.h"
/**************************************
* Defines
**************************************/
#ifdef __cplusplus
extern "C" {
#endif

    /**************************************
    * Data Structures
    **************************************/
    typedef struct BacEncContext {
        OutputBitstreamUnit   m_pcTComBitIf;
    } BacEncContext;

    typedef struct CabacEncodeContext {
        BacEncContext            bacEncContext;
    } CabacEncodeContext;


#ifdef __cplusplus
}
#endif
#endif //EbEntropyCodingUtil_h