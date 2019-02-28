/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbMotionVectorUnit_h
#define EbMotionVectorUnit_h

#include "EbDefinitions.h"
#include "EbDefinitions.h"
#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(push, 1)

    typedef union Mv 
    {
        struct
        {
            signed short x;
            signed short y;
        };
        uint32_t mv_union;
    } Mv;

#pragma pack(pop)

#pragma pack(push, 1)
    typedef struct Mvd
    {
        signed   mvdX : 16;
        signed   mvdY : 16;
        unsigned refIdx : 1;
        unsigned : 7;
        unsigned predIdx : 1;
        unsigned : 7;

    } Mvd;
#pragma pack(pop)

    typedef struct MvUnit
    {
        Mv            mv[MAX_NUM_OF_REF_PIC_LIST];
        uint8_t         predDirection;
    } MvUnit;

#ifdef __cplusplus
}
#endif
#endif // EbMotionVectorUnit_h