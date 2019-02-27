/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbEntropyCodingObject_h
#define EbEntropyCodingObject_h

#include "EbDefinitions.h"
#include "EbCabacContextModel.h"
#include "EbBitstreamUnit.h"
#ifdef __cplusplus
extern "C" {
#endif
    typedef struct Bitstream {
        EbPtr outputBitstreamPtr;
    } Bitstream;

    typedef struct EntropyCoder {
        EbPtr cabacEncodeContextPtr;
        FRAME_CONTEXT   *fc;              /* this frame entropy */
        aom_writer       ecWriter;
        EbPtr           ecOutputBitstreamPtr;
#if TILES
        uint64_t   ec_frame_size;
#endif
    } EntropyCoder;

    extern EbErrorType bitstream_ctor(
        Bitstream **bitstream_dbl_ptr,
        uint32_t buffer_size);

    extern EbErrorType entropy_coder_ctor(
        EntropyCoder **entropy_coder_dbl_ptr,
        uint32_t buffer_size);

    extern EbPtr entropy_coder_get_bitstream_ptr(
        EntropyCoder *entropy_coder_ptr);
#ifdef __cplusplus
}
#endif
#endif // EbEntropyCodingObject_h