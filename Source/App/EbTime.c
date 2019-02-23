/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/
#include <stdint.h>
#ifdef _WIN32
#include <stdlib.h>
//#if  (WIN_ENCODER_TIMING || WIN_DECODER_TIMING)
#include <time.h>
#include <windows.h>
//#endif

#elif defined(__linux__) || defined(__APPLE__)
#include <stdio.h>
#include <stdlib.h>
//#if   (LINUX_ENCODER_TIMING || LINUX_DECODER_TIMING)
#include <sys/time.h>
#include <time.h>
//#endif

#else
#error OS/Platform not supported.
#endif

void eb_start_time(
    uint64_t *start_seconds,
    uint64_t *start_useconds)
{

#if defined(__linux__) || defined(__APPLE__) //(LINUX_ENCODER_TIMING || LINUX_DECODER_TIMING)
    struct timeval start;
    gettimeofday(&start, NULL);
    *start_seconds=start.tv_sec;
    *start_useconds=start.tv_usec;
#elif _WIN32 //(WIN_ENCODER_TIMING || WIN_DECODER_TIMING)
    *start_seconds = (uint64_t) clock();
    (void) (*start_useconds);
#else
    (void) (*start_useconds);
    (void) (*start_seconds);
#endif

}
void eb_finish_time(
    uint64_t *finish_seconds,
    uint64_t *finish_useconds)
{

#if defined(__linux__) || defined(__APPLE__) //(LINUX_ENCODER_TIMING || LINUX_DECODER_TIMING)
    struct timeval finish;
    gettimeofday(&finish, NULL);
    *finish_seconds=finish.tv_sec;
    *finish_useconds=finish.tv_usec;
#elif _WIN32 //(WIN_ENCODER_TIMING || WIN_DECODER_TIMING)
    *finish_seconds= (uint64_t)clock();
    (void) (*finish_useconds);
#else
    (void) (*finish_useconds);
    (void) (*finish_seconds);
#endif

}
void eb_compute_overall_elapsed_time(
    uint64_t start_seconds,
    uint64_t start_useconds,
    uint64_t finish_seconds,
    uint64_t finish_useconds,
    double *duration){
#if defined(__linux__) || defined(__APPLE__) //(LINUX_ENCODER_TIMING || LINUX_DECODER_TIMING)
    long   mtime, seconds, useconds;
    seconds  = finish_seconds - start_seconds;
    useconds = finish_useconds - start_useconds;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    *duration = (double)mtime/1000;
    //printf("\nElapsed time: %3.3ld seconds\n", mtime/1000);
#elif _WIN32 //(WIN_ENCODER_TIMING || WIN_DECODER_TIMING)
    //double  duration;
    *duration = (double)(finish_seconds - start_seconds) / CLOCKS_PER_SEC;
    //printf("\nElapsed time: %3.3f seconds\n", *duration);
    (void) (start_useconds);
    (void) (finish_useconds);
#else
    (void) (start_useconds);
    (void) (start_seconds);
    (void) (finish_useconds);
    (void) (finish_seconds);

#endif

}
void eb_sleep(
    uint64_t milli_seconds){

    if(milli_seconds) {
#if defined(__linux__) || defined(__APPLE__)
        struct timespec req,rem;
        req.tv_sec=(int32_t)(milli_seconds/1000);
        milli_seconds -= req.tv_sec * 1000;
        req.tv_nsec = milli_seconds * 1000000UL;
        nanosleep(&req,&rem);
#elif _WIN32
        Sleep((DWORD) milli_seconds);
#else
#error OS Not supported
#endif
    }
}
void eb_injector(
    uint64_t processed_frame_count,
    uint32_t injector_frame_rate){

#if defined(__linux__) || defined(__APPLE__)
    uint64_t                  currentTimesSeconds = 0;
    uint64_t                  currentTimesuSeconds = 0;
    static uint64_t           startTimesSeconds;
    static uint64_t           startTimesuSeconds;
#elif _WIN32
    static LARGE_INTEGER    startCount;               // this is the start time
    static LARGE_INTEGER    counterFreq;              // performance counter frequency
    LARGE_INTEGER           nowCount;                 // this is the current time
#else
#error OS Not supported
#endif

    double                 injectorInterval  = (double)(1<<16)/(double)injector_frame_rate;     // 1.0 / injector frame rate (in this case, 1.0/encodRate)
    double                  elapsedTime;
    double                  predictedTime;
    int32_t                     bufferFrames = 1;         // How far ahead of time should we let it get
    int32_t                     milliSecAhead;
    static int32_t              firstTime = 0;

    if (firstTime == 0)
    {
        firstTime = 1;

#if defined(__linux__) || defined(__APPLE__)
        eb_start_time((uint64_t*)&startTimesSeconds, (uint64_t*)&startTimesuSeconds);
#elif _WIN32
        QueryPerformanceFrequency(&counterFreq);
        QueryPerformanceCounter(&startCount);
#endif
    }
    else
    {

#if defined(__linux__) || defined(__APPLE__)
        eb_finish_time((uint64_t*)&currentTimesSeconds, (uint64_t*)&currentTimesuSeconds);
        eb_compute_overall_elapsed_time(startTimesSeconds, startTimesuSeconds, currentTimesSeconds, currentTimesuSeconds, &elapsedTime);
#elif _WIN32
        QueryPerformanceCounter(&nowCount);
        elapsedTime = (double)(nowCount.QuadPart - startCount.QuadPart) / (double)counterFreq.QuadPart;
#endif

        predictedTime = (processed_frame_count - bufferFrames) * injectorInterval;
        milliSecAhead = (int32_t)(1000 * (predictedTime - elapsedTime ));
        if (milliSecAhead>0)
        {
            //  timeBeginPeriod(1);
            eb_sleep(milliSecAhead);
            //  timeEndPeriod (1);
        }
    }
}
