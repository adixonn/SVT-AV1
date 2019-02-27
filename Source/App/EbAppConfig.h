/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbAppConfig_h
#define EbAppConfig_h

#include <stdio.h>

#include "EbApi.h"

#ifdef __GNUC__
#define fseeko64 fseek
#define ftello64 ftell
#endif
// Define Cross-Platform 64-bit fseek() and ftell()
#ifdef _MSC_VER
typedef __int64 off64_t;
#define fseeko64 _fseeki64
#define ftello64 _ftelli64

#elif _WIN32 // MinGW

#endif

#ifndef _RSIZE_T_DEFINED
typedef size_t RSize;
#define _RSIZE_T_DEFINED
#endif  /* _RSIZE_T_DEFINED */

#ifndef _ERRNO_T_DEFINED
#define _ERRNO_T_DEFINED
typedef int32_t Errno;
#endif  /* _ERRNO_T_DEFINED */

/** The AppExitConditionType type is used to define the App main loop exit
conditions.
*/
typedef enum AppExitConditionType 
{
    APP_ExitConditionNone = 0,
    APP_ExitConditionFinished,
    APP_ExitConditionError
} AppExitConditionType;

/** The AppPortActiveType type is used to define the state of output ports in
the App.
*/
typedef enum AppPortActiveType 
{
    APP_PortActive = 0,
    APP_PortInactive
} AppPortActiveType;

/** The EB_PTR type is intended to be used to pass pointers to and from the svt
API.  This is a 32 bit pointer and is aligned on a 32 bit word boundary.
*/
typedef void * EB_PTR;

/** The EB_NULL type is used to define the C style NULL pointer.
*/
#define EB_NULL ((void*) 0)

// memory map to be removed and replaced by malloc / free
typedef enum EbPtrType 
{
    EB_N_PTR = 0,                                   // malloc'd pointer
    EB_A_PTR = 1,                                   // malloc'd pointer aligned
    EB_MUTEX = 2,                                   // mutex
    EB_SEMAPHORE = 3,                                   // semaphore
    EB_THREAD = 4                                    // thread handle
}EbPtrType;

typedef void * EbPtr;

typedef struct EbMemoryMapEntry
{
    EbPtr                     ptr;                       // points to a memory pointer
    EbPtrType                 ptr_type;                   // pointer type
} EbMemoryMapEntry;

extern    EbMemoryMapEntry        *app_memory_map;            // App Memory table
extern    uint32_t                  *app_memory_map_index;       // App Memory index
extern    uint64_t                  *total_app_memory;          // App Memory malloc'd
extern    uint32_t                   app_malloc_count;

#define MAX_APP_NUM_PTR                             (0x186A0 << 2)             // Maximum number of pointers to be allocated for the app

#define EB_APP_MALLOC(type, pointer, n_elements, pointer_class, return_type) \
    pointer = (type)malloc(n_elements); \
    if (pointer == (type)EB_NULL){ \
        return return_type; \
            } \
                else { \
        app_memory_map[*(app_memory_map_index)].ptr_type = pointer_class; \
        app_memory_map[(*(app_memory_map_index))++].ptr = pointer; \
        if (n_elements % 8 == 0) { \
            *total_app_memory += (n_elements); \
                        } \
                                else { \
            *total_app_memory += ((n_elements) + (8 - ((n_elements) % 8))); \
            } \
        } \
    if (*(app_memory_map_index) >= MAX_APP_NUM_PTR) { \
        return return_type; \
                } \
    app_malloc_count++;

#define EB_APP_MALLOC_NR(type, pointer, n_elements, pointer_class,return_type) \
    (void)return_type; \
    pointer = (type)malloc(n_elements); \
    if (pointer == (type)EB_NULL){ \
        return_type = EB_ErrorInsufficientResources; \
        printf("Malloc has failed due to insuffucient resources"); \
        return; \
            } \
                else { \
        app_memory_map[*(app_memory_map_index)].ptr_type = pointer_class; \
        app_memory_map[(*(app_memory_map_index))++].ptr = pointer; \
        if (n_elements % 8 == 0) { \
            *total_app_memory += (n_elements); \
                        } \
                                else { \
            *total_app_memory += ((n_elements) + (8 - ((n_elements) % 8))); \
            } \
        } \
    if (*(app_memory_map_index) >= MAX_APP_NUM_PTR) { \
        return_type = EB_ErrorInsufficientResources; \
        printf("Malloc has failed due to insuffucient resources"); \
        return; \
                } \
    app_malloc_count++;

/* string copy */
extern Errno strcpy_ss(char *dest, RSize dmax, const char *src);

/* fitted string copy */
extern Errno strncpy_ss(char *dest, RSize dmax, const char *src, RSize slen);

/* string length */
extern RSize strnlen_ss(const char *s, RSize smax);

#define EB_STRNCPY(dst, src, count) \
    strncpy_ss(dst, sizeof(dst), src, count)

#define EB_STRCPY(dst, size, src) \
    strcpy_ss(dst, size, src)

#define EB_STRCMP(target,token) \
    strcmp(target,token)

#define EB_STRLEN(target, max_size) \
    strnlen_ss(target, max_size)

#define EB_APP_MEMORY() \
    printf("Total Number of Mallocs in App: %d\n", app_malloc_count); \
    printf("Total App Memory: %.2lf KB\n\n",*total_app_memory/(double)1024);

#define MAX_CHANNEL_NUMBER      6
#define MAX_NUM_TOKENS          200

#ifdef _MSC_VER
#define FOPEN(f,s,m) fopen_s(&f,s,m)
#else
#define FOPEN(f,s,m) f=fopen(s,m)
#endif

/****************************************
* Padding
****************************************/
#define LEFT_INPUT_PADDING 0
#define RIGHT_INPUT_PADDING 0
#define TOP_INPUT_PADDING 0
#define BOTTOM_INPUT_PADDING 0


typedef struct EbPerformanceContext {

    /****************************************
     * Computational Performance Data
     ****************************************/
    uint64_t                  lib_start_time[2];       // [sec, micro_sec] including init time
    uint64_t                  encode_start_time[2];    // [sec, micro_sec] first frame sent

    double                    total_execution_time;    // includes init
    double                    total_encode_time;       // not including init

    uint64_t                  totalLatency;
    uint32_t                  maxLatency;

    uint64_t                  startsTime;
    uint64_t                  startuTime;
    uint64_t                  frameCount;

    double                    averageSpeed;
    double                    averageLatency;

    uint64_t                  byteCount;

}EbPerformanceContext;

typedef struct EbConfig
{
    /****************************************
     * File I/O
     ****************************************/
    FILE                    *configFile;
    FILE                    *inputFile;
    FILE                    *bitstreamFile;
    FILE                    *reconFile;
    FILE                    *errorLogFile;
    FILE                    *bufferFile;

    FILE                    *qpFile;

    EbBool                  y4mInput;
    unsigned char           y4mBuf[9];

    EbBool                  use_qp_file;

    uint32_t                 frameRate;
    uint32_t                 frameRateNumerator;
    uint32_t                 frameRateDenominator;
    uint32_t                 injector_frame_rate;
    uint32_t                 injector;
    uint32_t                 speed_control_flag;
    uint32_t                 encoderBitDepth;
    uint32_t                 compressedTenBitFormat;
    uint32_t                 sourceWidth;
    uint32_t                 sourceHeight;

    uint32_t                 inputPaddedWidth;
    uint32_t                 inputPaddedHeight;

    int64_t                  framesToBeEncoded;
    int32_t                  framesEncoded;
    int32_t                  bufferedInput;
    uint8_t                **sequenceBuffer;

    uint8_t                  latencyMode;

    /****************************************
     * // Interlaced Video
     ****************************************/
    EbBool                  interlacedVideo;
    EbBool                  separateFields;

    /*****************************************
     * Coding Structure
     *****************************************/
    uint32_t                 base_layer_switch_mode;
    uint8_t                  encMode;
    int32_t                  intraPeriod;
    uint32_t                 intraRefreshType;
    uint32_t                 hierarchicalLevels;
    uint32_t                 predStructure;


    /****************************************
     * Quantization
     ****************************************/
    uint32_t                 qp;

    /****************************************
     * Film Grain
     ****************************************/
    EbBool                  film_grain_denoise_strength;
     /****************************************
     * DLF
     ****************************************/
    EbBool                  disable_dlf_flag;

    /****************************************
     * Local Warped Motion
     ****************************************/
    EbBool                  enable_warped_motion;

    /****************************************
     * ME Tools
     ****************************************/
    EbBool                  use_default_me_hme;
    EbBool                  enableHmeFlag;
    EbBool                  enable_hme_level0_flag;
    EbBool                  enable_hme_level1_flag;
    EbBool                  enable_hme_level2_flag;
    EbBool                  ext_block_flag;
    EbBool                  in_loop_me_flag;

    /****************************************
     * ME Parameters
     ****************************************/
    uint32_t                 search_area_width;
    uint32_t                 search_area_height;

    /****************************************
     * HME Parameters
     ****************************************/
    uint32_t                 numberHmeSearchRegionInWidth ;
    uint32_t                 numberHmeSearchRegionInHeight;
    uint32_t                 hme_level0_total_search_area_width;
    uint32_t                 hme_level0_total_search_area_height;
    uint32_t                 hmeLevel0ColumnIndex;
    uint32_t                 hmeLevel0RowIndex;
    uint32_t                 hmeLevel1ColumnIndex;
    uint32_t                 hmeLevel1RowIndex;
    uint32_t                 hmeLevel2ColumnIndex;
    uint32_t                 hmeLevel2RowIndex;
    uint32_t                 hmeLevel0SearchAreaInWidthArray[EB_HME_SEARCH_AREA_COLUMN_MAX_COUNT];
    uint32_t                 hmeLevel0SearchAreaInHeightArray[EB_HME_SEARCH_AREA_ROW_MAX_COUNT];
    uint32_t                 hmeLevel1SearchAreaInWidthArray[EB_HME_SEARCH_AREA_COLUMN_MAX_COUNT];
    uint32_t                 hmeLevel1SearchAreaInHeightArray[EB_HME_SEARCH_AREA_ROW_MAX_COUNT];
    uint32_t                 hmeLevel2SearchAreaInWidthArray[EB_HME_SEARCH_AREA_COLUMN_MAX_COUNT];
    uint32_t                 hmeLevel2SearchAreaInHeightArray[EB_HME_SEARCH_AREA_ROW_MAX_COUNT];

    /****************************************
     * MD Parameters
     ****************************************/
    EbBool                  constrained_intra;

#if TILES
    int32_t                  tile_columns;
    int32_t                  tile_rows;
#endif

    /****************************************
     * Rate Control
     ****************************************/
    uint32_t                 scene_change_detection;
    uint32_t                 rateControlMode;
    uint32_t                 look_ahead_distance;
    uint32_t                 targetBitRate;
    uint32_t                 max_qp_allowed;
    uint32_t                 min_qp_allowed;

    /****************************************
     * Optional Features
     ****************************************/

    EbBool                   improve_sharpness;
    uint32_t                 high_dynamic_range_input;
    uint32_t                 access_unit_delimiter;
    uint32_t                 buffering_period_sei;
    uint32_t                 picture_timing_sei;
    EbBool                   registered_user_data_sei_flag;
    EbBool                   unregistered_user_data_sei_flag;
    EbBool                   recovery_point_sei_flag;
    uint32_t                 enable_temporal_id;

    /****************************************
     * Annex A Parameters
     ****************************************/
    uint32_t                 profile;
    uint32_t                 tier;
    uint32_t                 level;

    /****************************************
     * On-the-fly Testing
     ****************************************/
    uint32_t                 testUserData;
    EbBool                   eosFlag;

    /****************************************
    * Optimization Type
    ****************************************/
    uint32_t                  asmType;

    /****************************************
     * Computational Performance Data
     ****************************************/
    EbPerformanceContext  performanceContext;

    /****************************************
    * Instance Info
    ****************************************/
    uint32_t                channel_id;
    uint32_t                active_channel_count;
    uint32_t                logicalProcessors;
    int32_t                 targetSocket;
    EbBool                 stopEncoder;         // to signal CTRL+C Event, need to stop encoding.

    uint64_t                processed_frame_count;
    uint64_t                processedByteCount;

} EbConfig;

extern void eb_config_ctor(EbConfig *config_ptr);

extern void eb_config_dtor(EbConfig *config_ptr);

extern EbErrorType read_command_line(
    int32_t argc, 
    char *const argv[], 
    EbConfig **config, 
    uint32_t  numChannels,    
    EbErrorType *return_errors);

extern uint32_t get_help(int32_t argc, char *const argv[]);

extern uint32_t get_number_of_channels(int32_t argc, char *const argv[]);

#endif //EbAppConfig_h
