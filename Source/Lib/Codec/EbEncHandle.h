/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbEncHandle_h
#define EbEncHandle_h

#include "EbDefinitions.h"
#include "EbApi.h"
#include "EbPictureBufferDesc.h"
#include "EbSystemResourceManager.h"
#include "EbSequenceControlSet.h"

#include "EbResourceCoordinationResults.h"
#include "EbPictureDemuxResults.h"
#include "EbRateControlResults.h"

/**************************************
 * Component Private Data
 **************************************/
typedef struct EbEncHandle
{
    // Encode Instances & Compute Segments
    uint32_t                                  encodeInstanceTotalCount;
    uint32_t                                 *compute_segments_total_count_array;

    // Config Set Counts
    uint32_t                                  sequenceControlSetPoolTotalCount;

    // Full Results Count
    uint32_t                                  pictureControlSetPoolTotalCount;

    // Picture Buffer Counts
    uint32_t                                  reconPicturePoolTotalCount;
    uint32_t                                  coeffPicturePoolTotalCount;
    uint32_t                                  referencePicturePoolTotalCount;
    uint32_t                                  paReferencePicturPooleBufferInitCount;

    // Config Set Pool & Active Array
    EbSystemResource_t                     *sequenceControlSetPoolPtr;
    EbFifo                              **sequenceControlSetPoolProducerFifoPtrArray;
    EbSequenceControlSetInstance_t        **sequence_control_set_instance_array;

    // Full Results
    EbSystemResource_t                    **pictureControlSetPoolPtrArray;
    EbFifo                             ***pictureControlSetPoolProducerFifoPtrDblArray;

    //ParentControlSet
    EbSystemResource_t                    **pictureParentControlSetPoolPtrArray;
    EbFifo                             ***pictureParentControlSetPoolProducerFifoPtrDblArray;

    // Picture Buffers
    EbSystemResource_t                    **referencePicturePoolPtrArray;
    EbSystemResource_t                    **paReferencePicturePoolPtrArray;

    // Picture Buffer Producer Fifos
    EbFifo                             ***referencePicturePoolProducerFifoPtrDblArray;
    EbFifo                             ***paReferencePicturePoolProducerFifoPtrDblArray;

    // Thread Handles
    EbHandle                               resourceCoordinationThreadHandle;
    EbHandle                               pictureEnhancementThreadHandle;
    EbHandle                              *pictureAnalysisThreadHandleArray;
    EbHandle                               pictureDecisionThreadHandle;
    EbHandle                              *motionEstimationThreadHandleArray;
    EbHandle                               initialRateControlThreadHandle;
    EbHandle                              *sourceBasedOperationsThreadHandleArray;
    EbHandle                               pictureManagerThreadHandle;
    EbHandle                               rateControlThreadHandle;
    EbHandle                              *modeDecisionConfigurationThreadHandleArray;
    EbHandle                              *encDecThreadHandleArray;
    EbHandle                              *entropyCodingThreadHandleArray;
#if FILT_PROC
    EbHandle                              *dlfThreadHandleArray;
    EbHandle                              *cdefThreadHandleArray;
    EbHandle                              *restThreadHandleArray;
#endif
    EbHandle                               packetizationThreadHandle;

    // Contexts
    EbPtr                                  resourceCoordinationContextPtr;
    EbPtr                                  pictureEnhancementContextPtr;
    EbPtr                                 *pictureAnalysisContextPtrArray;
    EbPtr                                  pictureDecisionContextPtr;
    EbPtr                                 *motionEstimationContextPtrArray;
    EbPtr                                  initialRateControlContextPtr;
    EbPtr                                 *sourceBasedOperationsContextPtrArray;
    EbPtr                                  pictureManagerContextPtr;
    EbPtr                                  rateControlContextPtr;
    EbPtr                                 *modeDecisionConfigurationContextPtrArray;
    EbPtr                                 *encDecContextPtrArray;
    EbPtr                                 *entropyCodingContextPtrArray;
#if FILT_PROC
    EbPtr                                 *dlfContextPtrArray;
    EbPtr                                 *cdefContextPtrArray;
    EbPtr                                 *restContextPtrArray;
#endif
    EbPtr                                  packetizationContextPtr;

    // System Resource Managers
    EbSystemResource_t                     *input_buffer_resource_ptr;
    EbSystemResource_t                    **output_stream_buffer_resource_ptr_array;
    EbSystemResource_t                    **output_recon_buffer_resource_ptr_array;
    EbSystemResource_t                    **output_statistics_buffer_resource_ptr_array;
    EbSystemResource_t                     *resourceCoordinationResultsResourcePtr;
    EbSystemResource_t                     *pictureAnalysisResultsResourcePtr;
    EbSystemResource_t                     *pictureDecisionResultsResourcePtr;
    EbSystemResource_t                     *motionEstimationResultsResourcePtr;
    EbSystemResource_t                     *initialRateControlResultsResourcePtr;
    EbSystemResource_t                     *pictureDemuxResultsResourcePtr;
    EbSystemResource_t                     *rateControlTasksResourcePtr;
    EbSystemResource_t                     *rateControlResultsResourcePtr;
    EbSystemResource_t                     *encDecTasksResourcePtr;
    EbSystemResource_t                     *encDecResultsResourcePtr;
    EbSystemResource_t                     *entropyCodingResultsResourcePtr;
#if FILT_PROC
    EbSystemResource_t                     *dlfResultsResourcePtr;
    EbSystemResource_t                     *cdefResultsResourcePtr;
    EbSystemResource_t                     *restResultsResourcePtr;
#endif

    // Inter-Process Producer Fifos
    EbFifo                              **input_buffer_producer_fifo_ptr_array;
    EbFifo                             ***output_stream_buffer_producer_fifo_ptr_dbl_array;
    EbFifo                             ***output_recon_buffer_producer_fifo_ptr_dbl_array;
    EbFifo                             ***output_statistics_buffer_producer_fifo_ptr_dbl_array;
    EbFifo                              **resourceCoordinationResultsProducerFifoPtrArray;
    EbFifo                              **pictureAnalysisResultsProducerFifoPtrArray;
    EbFifo                              **pictureDecisionResultsProducerFifoPtrArray;
    EbFifo                              **motionEstimationResultsProducerFifoPtrArray;
    EbFifo                              **initialRateControlResultsProducerFifoPtrArray;
    EbFifo                              **pictureDemuxResultsProducerFifoPtrArray;
    EbFifo                              **pictureManagerResultsProducerFifoPtrArray;
    EbFifo                              **rateControlTasksProducerFifoPtrArray;
    EbFifo                              **rateControlResultsProducerFifoPtrArray;
    EbFifo                              **encDecTasksProducerFifoPtrArray;
    EbFifo                              **encDecResultsProducerFifoPtrArray;
    EbFifo                              **entropyCodingResultsProducerFifoPtrArray;
#if FILT_PROC
    EbFifo                              **dlfResultsProducerFifoPtrArray;
    EbFifo                              **cdefResultsProducerFifoPtrArray;
    EbFifo                              **restResultsProducerFifoPtrArray;
#endif

    // Inter-Process Consumer Fifos
    EbFifo                              **input_buffer_consumer_fifo_ptr_array;
    EbFifo                             ***output_stream_buffer_consumer_fifo_ptr_dbl_array;
    EbFifo                             ***output_recon_buffer_consumer_fifo_ptr_dbl_array;
    EbFifo                             ***output_statistics_buffer_consumer_fifo_ptr_dbl_array;
    EbFifo                              **resourceCoordinationResultsConsumerFifoPtrArray;
    EbFifo                              **pictureAnalysisResultsConsumerFifoPtrArray;
    EbFifo                              **pictureDecisionResultsConsumerFifoPtrArray;
    EbFifo                              **motionEstimationResultsConsumerFifoPtrArray;
    EbFifo                              **initialRateControlResultsConsumerFifoPtrArray;
    EbFifo                              **pictureDemuxResultsConsumerFifoPtrArray;
    EbFifo                              **rateControlTasksConsumerFifoPtrArray;
    EbFifo                              **rateControlResultsConsumerFifoPtrArray;
    EbFifo                              **encDecTasksConsumerFifoPtrArray;
    EbFifo                              **encDecResultsConsumerFifoPtrArray;
    EbFifo                              **entropyCodingResultsConsumerFifoPtrArray;
#if FILT_PROC
    EbFifo                              **dlfResultsConsumerFifoPtrArray;
    EbFifo                              **cdefResultsConsumerFifoPtrArray;
    EbFifo                              **restResultsConsumerFifoPtrArray;
#endif
    // Callbacks
    EbCallback                          **app_callback_ptr_array;

    // Memory Map
    EbMemoryMapEntry                       *memory_map;
    uint32_t                                memory_map_index;
    uint64_t                                total_lib_memory;

} EbEncHandle;


#endif // EbEncHandle_h