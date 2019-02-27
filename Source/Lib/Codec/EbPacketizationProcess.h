/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbPacketization_h
#define EbPacketization_h

#include "EbDefinitions.h"
#include "EbSystemResourceManager.h"
#ifdef __cplusplus
extern "C" {
#endif

    /**************************************
     * Type Declarations
     **************************************/
    typedef struct EbPPSConfig
    {
        uint8_t ppsId;
        uint8_t constrainedFlag;

    } EbPPSConfig;

    /**************************************
     * Context
     **************************************/
    typedef struct PacketizationContext
    {
        EbFifo      *entropy_coding_input_fifo_ptr;
        EbFifo      *rate_control_tasks_output_fifo_ptr;
        EbPPSConfig *ppsConfig;

        uint64_t   dpbDispOrder[8], dpbDecOrder[8];
        uint64_t   totShownFrames;
        uint64_t   dispOrderContinuityCount;

    } PacketizationContext;

    /**************************************
     * Extern Function Declarations
     **************************************/
    extern EbErrorType packetization_context_ctor(
        PacketizationContext **context_dbl_ptr,
        EbFifo                *entropy_coding_input_fifo_ptr,
        EbFifo                *rate_control_tasks_output_fifo_ptr);



    extern void* PacketizationKernel(void *input_ptr);
#ifdef __cplusplus
}
#endif
#endif // EbPacketization_h
