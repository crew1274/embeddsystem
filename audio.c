/*
 * audio.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <xdc/std.h>

#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/audio1/auddec1.h>

#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Sound.h>
#include <ti/sdo/dmai/Buffer.h>
#include <ti/sdo/dmai/Loader.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/ce/Adec1.h>

#include "../demo.h"
#include "audio.h"

/******************************************************************************
 * audioThrFxn
 ******************************************************************************/
Void *audioThrFxn(Void *arg)
{
    AudioEnv               *envp                = (AudioEnv *) arg;
    Void                   *status              = THREAD_SUCCESS;
    Sound_Attrs             sAttrs              = Sound_Attrs_STEREO_DEFAULT;
    Buffer_Attrs            bAttrs              = Buffer_Attrs_DEFAULT;
    Loader_Attrs            lAttrs              = Loader_Attrs_DEFAULT;
    AUDDEC1_Params          defaultParams       = Adec1_Params_DEFAULT;
    AUDDEC1_DynamicParams   defaultDynParams    = Adec1_DynamicParams_DEFAULT;
    Engine_Handle           hEngine             = NULL;
    Sound_Handle            hSound              = NULL;
    Loader_Handle           hLoader             = NULL;
    Adec1_Handle            hAd1                = NULL;
    Buffer_Handle           hOutBuf             = NULL;
    AUDDEC1_Params         *params;
    AUDDEC1_DynamicParams  *dynParams;
    Buffer_Handle           hInBuf;


    /* MMN Blank : Open the codec engine */
    hEngine =  Engine_open(envp->engineName, NULL, NULL);

    if (hEngine == NULL) {
        ERR("Failed to open codec engine %s\n", envp->engineName);
        cleanup(THREAD_FAILURE);
    }

    /* Use supplied params if any, otherwise use defaults */
    params = envp->params ? envp->params : &defaultParams;
    dynParams = envp->dynParams ? envp->dynParams : &defaultDynParams;

    /* MMN Blank : Create the audio decoder */
    hAd1 = Adec1_create(hEngine, envp->audioDecoder, params, dynParams);

    if (hAd1 == NULL) {
        ERR("Failed to create audio decoder %s\n", envp->audioDecoder);
        cleanup(THREAD_FAILURE);
    }

    /* MMN Blank : Create an output buffer for decoded data */
    hOutBuf = Buffer_create(Adec1_getOutBufSize((hAd1), &bAttrs);

    if (hOutBuf == NULL) {
        ERR("Failed to allocate output buffer\n");
        cleanup(THREAD_FAILURE);
    }

    /* Ask the codec how much input data it needs */
    lAttrs.readSize = Adec1_getInBufSize(hAd1);

    /* Make the total ring buffer larger */
    lAttrs.readBufSize = lAttrs.readSize * 10;

    /* MMN Blank : Create the file loader for reading encoded data */
    hLoader = Loader_create(envp->audioFile, &lAttrs);

    if (hLoader == NULL) {
        ERR("Failed to create loader\n");
        cleanup(THREAD_FAILURE);
    }

    /* MMN Blank : Signal that initialization is done and wait for other threads */
    Rendezvous_meet(envp->hRendezvousInit);

    /* MMN Blank : Prime the file loader */
    Loader_prime(hLoader, &hInBuf);

    /* MMN Blank : Decode the audio buffer */
    if (Adec1_process(hAd1, hInBuf, hOutBuf) < 0) {
        ERR("Failed to decode audio buffer\n");
        cleanup(THREAD_FAILURE);
    }

    /* Set the sample rate for the user interface */
    gblSetSamplingFrequency(Adec1_getSampleRate(hAd1));

    /* Create the sound device */
    sAttrs.sampleRate = Adec1_getSampleRate(hAd1);
    sAttrs.soundStd = Sound_Std_ALSA;
    //MMN Blank
    hSound = Sound_create(&sAttrs);

    if (hSound == NULL) {
        ERR("Failed to create audio device\n");
        cleanup(THREAD_FAILURE);
    }

    while (!gblGetQuit()) {
        /* Increment the number of bytes decoded for the user interface */
        gblIncSoundBytesProcessed(Buffer_getNumBytesUsed(hInBuf));

        /* MMN Blank : Write the decoded samples to the sound device */
        if (Sound_write(hSound, hOutBuf) < 0) {
            ERR("Failed to write audio buffer\n");
            cleanup(THREAD_FAILURE);
        }

        /* Pause processing? */
        Pause_test(envp->hPauseProcess);

        /* MMN Blank : Load a new frame from the file system */
        if (Loader_getFrame(hLoader, hInBuf) < 0) {
            ERR("Failed to read a frame of encoded data\n");
            cleanup(THREAD_FAILURE);
        }

        /* Check if the clip has ended */
        if (Buffer_getUserPtr(hInBuf) == NULL) {
            /* Wait for the video clip to finish, if applicable */
            Rendezvous_meet(envp->hRendezvousLoop);

            /* If we are to loop the clip, start over */
            if (envp->loop) {
                /* Recreate the audio codec */
                Adec1_delete(hAd1);
                hAd1 = Adec1_create(hEngine, envp->audioDecoder,
                                    params, dynParams);

                if (hAd1 == NULL) {
                    ERR("Failed to create audio decoder %s\n",
                        envp->audioDecoder);
                    cleanup(THREAD_FAILURE);
                }

                /* Re-prime the file loader */
                Loader_prime(hLoader, &hInBuf);
            }
            else {
                printf("End of clip reached, exiting..\n");
                cleanup(THREAD_SUCCESS);
            }
        }

        /* Decode the audio buffer */
        if (Adec1_process(hAd1, hInBuf, hOutBuf) < 0) {
            ERR("Failed to decode audio buffer\n");
            cleanup(THREAD_FAILURE);
        }
    }

cleanup:
    /* Make sure the other threads aren't waiting for us */
    Rendezvous_force(envp->hRendezvousInit);
    Rendezvous_force(envp->hRendezvousLoop);
    Pause_off(envp->hPauseProcess);

    /* Meet up with other threads before cleaning up */
    Rendezvous_meet(envp->hRendezvousCleanup);

    /* MMN Blank : Clean up the thread before exiting */
    if (hLoader) {
        Loader.delete(hLoader);
    }

    if (hAd1) {
        Adec1_delete(hAd1)
    }

    if (hSound) {
        Sound_delete(hSound);
    }

    if (hOutBuf) {
        Buffer_delete(hOutBuf);
    }

    if (hEngine) {
        Engine_delete(hEngine);
    }

    return status;
}

