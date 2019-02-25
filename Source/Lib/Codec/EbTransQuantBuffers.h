/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbTransQuantBuffers_h
#define EbTransQuantBuffers_h

#include "EbDefinitions.h"
#include "EbPictureBufferDesc.h"

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct EbTransQuantBuffers
    {
        EbPictureBufferDesc         *tuTransCoeff2Nx2NPtr;
        EbPictureBufferDesc         *tuTransCoeffNxNPtr;
        EbPictureBufferDesc         *tuTransCoeffN2xN2Ptr;
        EbPictureBufferDesc         *tuQuantCoeffNxNPtr;
        EbPictureBufferDesc         *tuQuantCoeffN2xN2Ptr;

    } EbTransQuantBuffers;


    extern EbErrorType eb_trans_quant_buffers_ctor(
        EbTransQuantBuffers *trans_quant_buffers_ptr);


#ifdef __cplusplus
}
#endif
#endif // EbTransQuantBuffers_h