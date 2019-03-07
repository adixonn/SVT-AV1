/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

// main.cpp
//  -Contructs the following resources needed during the encoding process
//      -memory
//      -threads
//  -Configures the encoder
//  -Calls the encoder via the API
//  -Destructs the resources

/***************************************
 * Includes
 ***************************************/
#include "EbSimpleAppContext.h"
#include "EbApi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if _WIN32
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#define FOPEN(f,s,m) fopen_s(&f,s,m)
#else
#define fseeko64 fseek
#define ftello64 ftell
#define FOPEN(f,s,m) f=fopen(s,m)
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#endif

/** The AppExitConditionType type is used to define the App main loop exit
conditions.
*/
typedef enum AppExitConditionType 
{
    APP_ExitConditionNone = 0,
    APP_ExitConditionFinished,
    APP_ExitConditionError
} AppExitConditionType;

/****************************************
* Padding
****************************************/
#define LEFT_INPUT_PADDING 0
#define RIGHT_INPUT_PADDING 0
#define TOP_INPUT_PADDING 0
#define BOTTOM_INPUT_PADDING 0

 /**********************************
 * Constructor
 **********************************/
static void eb_config_ctor(EbConfig *config_ptr)
{
    config_ptr->input_file = NULL;
    config_ptr->bitstream_file = NULL;
    config_ptr->recon_file = NULL;
    config_ptr->encoder_bit_depth = 8;
    config_ptr->compressed_ten_bit_format = 0;
    config_ptr->source_width = 0;
    config_ptr->source_height = 0;
    config_ptr->frames_to_be_encoded = 0;
    config_ptr->channel_id = 0;
    config_ptr->stop_encoder = 0;

    return;
}

/**********************************
* Destructor
**********************************/
static void eb_config_dtor(EbConfig *config_ptr)
{

    if (config_ptr->input_file) {
        fclose(config_ptr->input_file);
        config_ptr->input_file = (FILE *)NULL;
    }

    if (config_ptr->recon_file) {
        fclose(config_ptr->recon_file);
        config_ptr->recon_file = (FILE *)NULL;
    }

    if (config_ptr->bitstream_file) {
        fclose(config_ptr->bitstream_file);
        config_ptr->bitstream_file = (FILE *)NULL;
    }

    return;
}

AppExitConditionType ProcessOutputReconBuffer(
    EbConfig             *config,
    EbAppContext         *appCallBack)
{
    EbBufferHeaderType    *headerPtr = appCallBack->recon_buffer; // needs to change for buffered input
    EbComponentType       *componentHandle = (EbComponentType*)appCallBack->svt_encoder_handle;
    AppExitConditionType    return_value = APP_ExitConditionNone;
    EbErrorType            recon_status = EB_ErrorNone;
    int32_t fseekReturnVal;
    // non-blocking call until all input frames are sent
    recon_status = eb_svt_get_recon(componentHandle, headerPtr);

    if (recon_status == EB_ErrorMax) {
        printf("\nError while outputing recon, code 0x%x\n", headerPtr->flags);
        return APP_ExitConditionError;
    }
    else if (recon_status != EB_NoErrorEmptyQueue) {
        //Sets the File position to the beginning of the file.
        rewind(config->recon_file);
        uint64_t frameNum = headerPtr->pts;
        while (frameNum>0) {
            fseekReturnVal = fseeko64(config->recon_file, headerPtr->n_filled_len, SEEK_CUR);

            if (fseekReturnVal != 0) {
                printf("Error in fseeko64  returnVal %i\n", fseekReturnVal);
                return APP_ExitConditionError;
            }
            frameNum = frameNum - 1;
        }

        fwrite(headerPtr->p_buffer, 1, headerPtr->n_filled_len, config->recon_file);

        // Update Output Port Activity State
        return_value = (headerPtr->flags & EB_BUFFERFLAG_EOS) ? APP_ExitConditionFinished : APP_ExitConditionNone;
    }
    return return_value;
}
AppExitConditionType ProcessOutputStreamBuffer(
    EbConfig             *config,
    EbAppContext         *appCallback,
    uint8_t           pic_send_done
)
{
    EbBufferHeaderType    *headerPtr;
    EbComponentType       *componentHandle = (EbComponentType*)appCallback->svt_encoder_handle;
    AppExitConditionType    return_value = APP_ExitConditionNone;
    EbErrorType            stream_status = EB_ErrorNone;
    // System performance variables
    static int64_t          frame_count = 0;

    // non-blocking call
    stream_status = eb_svt_get_packet(componentHandle, &headerPtr, pic_send_done);

    if (stream_status == EB_ErrorMax) {
        printf("\nError while encoding, code 0x%x\n", headerPtr->flags);
        return APP_ExitConditionError;
    }else if (stream_status != EB_NoErrorEmptyQueue) {
        fwrite(headerPtr->p_buffer, 1, headerPtr->n_filled_len, config->bitstream_file);

        // Update Output Port Activity State
        return_value = (headerPtr->flags & EB_BUFFERFLAG_EOS) ? APP_ExitConditionFinished : APP_ExitConditionNone;
        //printf("\b\b\b\b\b\b\b\b\b%9d", ++frame_count);
        printf("\nDecode Order:\t%ld\tdts:\t%ld\tpts:\t%ld\tSliceType:\t%d", (long int)frame_count++, (long int)headerPtr->dts , (long int)headerPtr->pts, (int)headerPtr->pic_type);

        fflush(stdout);

        // Release the output buffer
        eb_svt_release_out_buffer(&headerPtr);
    }
    return return_value;
}

#define SIZE_OF_ONE_FRAME_IN_BYTES(width, height,is16bit) ( ( ((width)*(height)*3)>>1 )<<is16bit)
void ReadInputFrames(
    EbConfig                  *config,
    uint8_t                      is16bit,
    EbBufferHeaderType         *headerPtr)
{

    uint64_t  readSize;
    uint32_t  input_padded_width = config->input_padded_width;
    uint32_t  input_padded_height = config->input_padded_height;
    FILE   *input_file = config->input_file;
    uint8_t  *ebInputPtr;
    EbSvtEncInput* inputPtr = (EbSvtEncInput*)headerPtr->p_buffer;
    inputPtr->y_stride  = input_padded_width;
    inputPtr->cb_stride = input_padded_width >> 1;
    inputPtr->cr_stride = input_padded_width >> 1;
    {
        if (is16bit == 0 || (is16bit == 1 && config->compressed_ten_bit_format == 0)) {

            readSize = (uint64_t)SIZE_OF_ONE_FRAME_IN_BYTES(input_padded_width, input_padded_height, is16bit);

            headerPtr->n_filled_len = 0;

            {
                uint64_t lumaReadSize = (uint64_t)input_padded_width*input_padded_height << is16bit;
                ebInputPtr = inputPtr->luma;
                headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, lumaReadSize, input_file);
                ebInputPtr = inputPtr->cb;
                headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, lumaReadSize >> 2, input_file);
                ebInputPtr = inputPtr->cr;
                headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, lumaReadSize >> 2, input_file);
                inputPtr->luma = inputPtr->luma + ((config->input_padded_width*TOP_INPUT_PADDING + LEFT_INPUT_PADDING) << is16bit);
                inputPtr->cb = inputPtr->cb + (((config->input_padded_width >> 1)*(TOP_INPUT_PADDING >> 1) + (LEFT_INPUT_PADDING >> 1)) << is16bit);
                inputPtr->cr = inputPtr->cr + (((config->input_padded_width >> 1)*(TOP_INPUT_PADDING >> 1) + (LEFT_INPUT_PADDING >> 1)) << is16bit);

                 if (readSize != headerPtr->n_filled_len) {
                    config->stop_encoder = 1;
                 }
            }
        }
        // 10-bit Compressed Unpacked Mode
        else if (is16bit == 1 && config->compressed_ten_bit_format == 1) {

            // Fill the buffer with a complete frame
            headerPtr->n_filled_len = 0;

            uint64_t lumaReadSize = (uint64_t)input_padded_width*input_padded_height;
            uint64_t nbitlumaReadSize = (uint64_t)(input_padded_width / 4)*input_padded_height;

            ebInputPtr = inputPtr->luma;
            headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, lumaReadSize, input_file);
            ebInputPtr = inputPtr->cb;
            headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, lumaReadSize >> 2, input_file);
            ebInputPtr = inputPtr->cr;
            headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, lumaReadSize >> 2, input_file);

            inputPtr->luma = inputPtr->luma + config->input_padded_width*TOP_INPUT_PADDING + LEFT_INPUT_PADDING;
            inputPtr->cb = inputPtr->cb + (config->input_padded_width >> 1)*(TOP_INPUT_PADDING >> 1) + (LEFT_INPUT_PADDING >> 1);
            inputPtr->cr = inputPtr->cr + (config->input_padded_width >> 1)*(TOP_INPUT_PADDING >> 1) + (LEFT_INPUT_PADDING >> 1);


            ebInputPtr = inputPtr->luma_ext;
            headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, nbitlumaReadSize, input_file);
            ebInputPtr = inputPtr->cb_ext;
            headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, nbitlumaReadSize >> 2, input_file);
            ebInputPtr = inputPtr->cr_ext;
            headerPtr->n_filled_len += (uint32_t)fread(ebInputPtr, 1, nbitlumaReadSize >> 2, input_file);

            inputPtr->luma_ext = inputPtr->luma_ext + ((config->input_padded_width >> 2)*TOP_INPUT_PADDING + (LEFT_INPUT_PADDING >> 2));
            inputPtr->cb_ext = inputPtr->cb_ext + (((config->input_padded_width >> 1) >> 2)*(TOP_INPUT_PADDING >> 1) + ((LEFT_INPUT_PADDING >> 1) >> 2));
            inputPtr->cr_ext = inputPtr->cr_ext + (((config->input_padded_width >> 1) >> 2)*(TOP_INPUT_PADDING >> 1) + ((LEFT_INPUT_PADDING >> 1) >> 2));

            readSize = ((lumaReadSize * 3) >> 1) + ((nbitlumaReadSize * 3) >> 1);

            if (readSize != headerPtr->n_filled_len) {
                config->stop_encoder = 1;
            }

        }
    }
    // If we reached the end of file, loop over again
    if (feof(input_file) != 0) {
        //fseek(input_file, 0, SEEK_SET);
        config->stop_encoder = 1;
    }

    return;
}
#define  TEST_IDR 0
AppExitConditionType ProcessInputBuffer(
    EbConfig                  *config,
    EbAppContext              *appCallBack)
{
    uint8_t            is16bit = (uint8_t)(config->encoder_bit_depth > 8);
    EbBufferHeaderType     *headerPtr = appCallBack->inputPictureBuffer; // needs to change for buffered input
    EbComponentType        *componentHandle = (EbComponentType*)appCallBack->svt_encoder_handle;
    AppExitConditionType     return_value = APP_ExitConditionNone;
    static int32_t               frame_count = 0;

    if (config->stop_encoder == 0) {
        ReadInputFrames(
            config,
            is16bit,
            headerPtr);

        if (config->stop_encoder == 0) {
            //printf ("DISP: %d", frame_count);
            // Fill in Buffers Header control data
            headerPtr->flags = 0;
            headerPtr->p_app_private = NULL;
            headerPtr->pts         = frame_count++;
            headerPtr->pic_type   = EB_INVALID_PICTURE;
#if TEST_IDR
            if (frame_count == 200)
                headerPtr->pic_type = IDR_SLICE;
            if (frame_count == 150)
                headerPtr->pic_type = I_SLICE;
#endif
            // Send the picture
            eb_svt_enc_send_picture(componentHandle, headerPtr);
        }
        else {
            EbBufferHeaderType headerPtrLast;
            headerPtrLast.n_alloc_len = 0;
            headerPtrLast.n_filled_len = 0;
            headerPtrLast.n_tick_count = 0;
            headerPtrLast.p_app_private = NULL;
            headerPtrLast.flags = EB_BUFFERFLAG_EOS;
            headerPtrLast.p_buffer = NULL;
            headerPtr->flags = EB_BUFFERFLAG_EOS;

            eb_svt_enc_send_picture(componentHandle, &headerPtrLast);
        }
        return_value = (headerPtr->flags == EB_BUFFERFLAG_EOS) ? APP_ExitConditionFinished : return_value;
    }
    return return_value;
}

/***************************************
 * Encoder App Main
 ***************************************/
int32_t main(int32_t argc, char* argv[])
{
    EbErrorType            return_error = EB_ErrorNone;            // Error Handling
    AppExitConditionType    exitConditionOutput = APP_ExitConditionNone , exitConditionInput = APP_ExitConditionNone , exitConditionRecon = APP_ExitConditionNone;    // Processing loop exit condition
    EbConfig             *config;        // Encoder Configuration
    EbAppContext         *appCallback;   // Instances App callback data

    // Print Encoder Info
    printf("-------------------------------------\n");
    printf("SVT-AV1 Encoder Simple Sample Application v1.2.0\n");
    printf("Platform:   %u bit\n", (unsigned) sizeof(void*)*8);
#if ( defined( _MSC_VER ) && (_MSC_VER < 1910) )
    printf("Compiler: VS13\n");
#elif ( defined( _MSC_VER ) && (_MSC_VER >= 1910) )
    printf("Compiler: VS17\n");
#elif defined(__INTEL_COMPILER)
    printf("Compiler: Intel\n");
#elif defined(__GNUC__)
    printf("Compiler: GCC\n");
#else
    printf("Compiler: unknown\n");
#endif

    printf("APP Build date: %s %s\n",__DATE__,__TIME__);
    fflush(stdout);
    {
        // Initialize config
        config = (EbConfig*)malloc(sizeof(EbConfig));
        if (config){
            eb_config_ctor(config);
            if (argc != 6 && argc != 7) {
                printf("Usage: ./SvtHevcEncSimpleApp in.yuv out.ivf width height bitdepth recon.yuv(optional)\n");
                return_error = EB_ErrorBadParameter;
            }
            else {
                // Get info for config
                FILE * fin;
                FOPEN(fin,argv[1], "rb");
                if (!fin) {
                    printf("Invalid input file \n");
                    return_error = EB_ErrorBadParameter;
                }
                else
                    config->input_file = fin;

                FILE * fout;
                FOPEN(fout,argv[2], "wb");
                if (!fout) {
                    printf("Invalid input file \n");
                    return_error = EB_ErrorBadParameter;
                }
                else
                    config->bitstream_file = fout;

                uint32_t width = 0, height = 0;

                width = strtoul(argv[3], NULL, 0);
                height = strtoul(argv[4], NULL, 0);
                if ((width&&height) == 0){
                    printf("Invalid video dimensions\n");
                    return_error = EB_ErrorBadParameter;
                }

                config->input_padded_width  = config->source_width = width;
                config->input_padded_height = config->source_height = height;

                uint32_t bdepth = width = strtoul(argv[5], NULL, 0);
                if ((bdepth != 8) && (bdepth != 10)){
                    printf("Invalid bit depth\n");
                    return_error = EB_ErrorBadParameter;
                }
                config->encoder_bit_depth = bdepth;

                if (argc == 7) {
                    FILE * frec;
                    FOPEN(frec, argv[6], "wb");
                    if (!frec) {
                        printf("Invalid recon file \n");
                        return_error = EB_ErrorBadParameter;
                    }
                    else
                        config->recon_file = frec;
                }

            }
        }
        if (return_error == EB_ErrorNone && (config != NULL)) {

            // Initialize appCallback
            appCallback = (EbAppContext*)malloc(sizeof(EbAppContext));
            if (appCallback){
                EbAppContextCtor(appCallback,config);

                return_error = init_encoder(config, appCallback, 0);

                printf("Encoding          ");
                fflush(stdout);

                // Input Loop Thread
                exitConditionOutput = APP_ExitConditionNone;
                exitConditionRecon = APP_ExitConditionNone;
                while (exitConditionOutput == APP_ExitConditionNone) {
                    exitConditionInput = ProcessInputBuffer(config, appCallback);
                    if (config->recon_file) {
                        exitConditionRecon = ProcessOutputReconBuffer(config, appCallback);
                    }
                    exitConditionOutput = ProcessOutputStreamBuffer(config, appCallback, (exitConditionInput == APP_ExitConditionNone || (exitConditionRecon == APP_ExitConditionNone && config->recon_file) ? 0 : 1));
                }
                return_error = de_init_encoder(appCallback, 0);
                // Destruct the App memory variables
                EbAppContextDtor(appCallback);
                free(appCallback);

            }else
                printf("Error allocating EbAppContext structure");

            printf("\n");
            fflush(stdout);

        }
        else {
            printf("Error in configuration, could not begin encoding! ... \n");
        }

        if (config){
            eb_config_dtor(config);
            free(config);
        }
    }
    printf("Encoder finished\n");

    return (return_error == 0) ? 0 : 1;
}
