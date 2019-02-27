/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbAppContext_h
#define EbAppContext_h

#include "EbApi.h"
#include "EbAppConfig.h"

/***************************************

 * App Callback data struct
 ***************************************/
typedef struct EbAppContext {
    void                               *cmdSemaphoreHandle;
    void                               *inputSemaphoreHandle;
    void                               *streamSemaphoreHandle;
    EbSvtAv1EncConfiguration              ebEncParameters;

    // Output Ports Active Flags
    AppPortActiveType                   outputStreamPortActive;

    // Component Handle
    EbComponentType*                   svtEncoderHandle;

    // Buffer Pools
    EbBufferHeaderType                *inputBufferPool;
    EbBufferHeaderType                *streamBufferPool;
    EbBufferHeaderType                *recon_buffer;

    // Instance Index
    uint8_t                            instance_idx;

} EbAppContext;


/********************************
 * External Function
 ********************************/
extern EbErrorType init_encoder(
    EbConfig *config, 
    EbAppContext *callback_data, 
    uint32_t instance_idx);

extern EbErrorType de_init_encoder(
    EbAppContext *callback_data_ptr, 
    uint32_t instance_index);

#endif // EbAppContext_h