/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbPictureManager_h
#define EbPictureManager_h

#include "EbDefinitions.h"
#include "EbSystemResourceManager.h"
#ifdef __cplusplus
extern "C" {
#endif
    /***************************************
     * Context
     ***************************************/
    typedef struct PictureManagerContext_s
    {
        EbFifo_t                 *pictureInputFifoPtr;
        EbFifo_t                 *pictureManagerOutputFifoPtr;
        EbFifo_t                **picture_control_set_fifo_ptr_array;

    } PictureManagerContext_t;

    /***************************************
     * Extern Function Declaration
     ***************************************/
    extern EbErrorType PictureManagerContextCtor(
        PictureManagerContext_t **context_dbl_ptr,
        EbFifo_t                 *pictureInputFifoPtr,
        EbFifo_t                 *pictureManagerOutputFifoPtr,
        EbFifo_t                **picture_control_set_fifo_ptr_array);



    extern void* PictureManagerKernel(void *input_ptr);
#ifdef __cplusplus
}
#endif
#endif // EbPictureManager_h