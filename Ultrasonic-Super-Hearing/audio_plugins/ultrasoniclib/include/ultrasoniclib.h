/*
 * Copyright 2020 Leo McCormack
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file ultrasoniclib.h
 * @brief Implements the signal processing used by the proof-of-concept device
 *        described in [1]
 *
 * @see [1] Pulkki, V., McCormack, L. & Gonzalez, R. 2021. Superhuman spatial
 *          hearing technology for ultrasonic frequencies. Scientific Reports
 *          11, 11608 (2021). https://doi.org/10.1038/s41598-021-90829-9
 *
 * @author Leo McCormack
 * @date 06.05.2020
 */

#ifndef __ULTRASONICLIB_H_INCLUDED__
#define __ULTRASONICLIB_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* ========================================================================== */
/*                             Presets + Constants                            */
/* ========================================================================== */

/** Available pitch shifting options */
typedef enum _ULTRASONICLIB_PITCHSHFT_OPTIONS{
    ULTRASONICLIB_PITCHSHFT_NONE = 1,
    ULTRASONICLIB_PITCHSHFT_DOWN_1_OCT,
    ULTRASONICLIB_PITCHSHFT_DOWN_2_OCT,
    ULTRASONICLIB_PITCHSHFT_DOWN_3_OCT,
    ULTRASONICLIB_PITCHSHFT_USE_CHANNEL_7

}ULTRASONICLIB_PITCHSHFT_OPTIONS;

/** Number of available pitch shifting options */
#define ULTRASONICLIB_MUM_PITCHSHFT_OPTIONS ( 5 )

/** Current status of the codec. */
typedef enum _ULTRASONICLIB_CODEC_STATUS {
    CODEC_STATUS_INITIALISED = 0, /**< Codec is initialised and ready to process
                                   *   input audio. */
    CODEC_STATUS_NOT_INITIALISED, /**< Codec has not yet been initialised, or
                                   *   the codec configuration has changed.
                                   *   Input audio should not be processed. */
    CODEC_STATUS_INITIALISING     /**< Codec is currently being initialised,
                                   *   input audio should not be processed. */
} ULTRASONICLIB_CODEC_STATUS;

/** Number of microphones in the array */
#define ULTRASONICLIB_NUM_INPUT_CHANNELS ( 6 )

/** Length of the optional progress bar text */
#define ULTRASONICLIB_PROGRESSBARTEXT_CHAR_LENGTH ( 256 )


/* ========================================================================== */
/*                               Main Functions                               */
/* ========================================================================== */

/**
 * Creates an instance of ultrasoniclib
 *
 * @param[in] phUS (&) address of ultrasoniclib handle
 */
void ultrasoniclib_create(void** const phUS);

/**
 * Destroys an instance of ultrasoniclib
 *
 * @param[in] phUS (&) address of ultrasoniclib handle
 */
void ultrasoniclib_destroy(void** const phUS);

/**
 * Initialises an instance of ultrasoniclib with default settings
 *
 * @param[in] hUS        ultrasoniclib handle
 * @param[in] samplerate Host samplerate.
 */
void ultrasoniclib_init(void* const hUS,
                        int samplerate);

/**
 * Intialises the codec variables, based on current global/user parameters
 *
 * @param[in] hUS ultrasoniclib handle
 */
void ultrasoniclib_initCodec(void* const hUS);

/**
 * Applies the DoA estimation, pitch shifting, and binaural rendering
 *
 * @param[in] hUS      ultrasoniclib handle
 * @param[in] inputs   Input channel buffers; 2-D array: nInputs x nSamples
 * @param[in] outputs  Output channel buffers; 2-D array: nOutputs x nSamples
 * @param[in] nInputs  Number of input channels
 * @param[in] nOutputs Number of output channels
 * @param[in] nSamples Number of samples in 'inputs'/'output' matrices
 */
void ultrasoniclib_process(void* const hUS,
                           float** const inputs,
                           float** const outputs,
                           int nInputs,
                           int nOutputs,
                           int nSamples);


/* ========================================================================== */
/*                                Set Functions                               */
/* ========================================================================== */

/**
 * Sets all intialisation flags to 1; re-initialising all settings/variables
 * as ultrasoniclib is currently configured, at next available opportunity.
 *
 * @param[in] hUS ultrasoniclib handle
 */
void ultrasoniclib_refreshParams(void* const hUS);

/** Sets pitch shift option (see ULTRASONICLIB_PITCHSHFT_OPTIONS enum) */
void ultrasoniclib_setPitchShiftOption(void* const hUS,
                                       ULTRASONICLIB_PITCHSHFT_OPTIONS newOption);

/** Sets the temporal averging coefficient */
void ultrasoniclib_setDoAaveragingCoeff(void* const hUS, float newValue);

/** Sets the gain applied to the output signals, in dB */
void ultrasoniclib_setPostGain_dB(void* const hUS, float newValue);

/**
 * Sets whether to enable/disable the amplitude "ducking" of the output signals
 * based on the analysed diffuseness value
 */
void ultrasoniclib_setEnableDiffuseness(void* const hUS, int newState);


/* ========================================================================== */
/*                                Get Functions                               */
/* ========================================================================== */

/**
 * Returns the processing framesize (i.e., number of samples processed with
 * every _process() call )
 */
int ultrasoniclib_getFrameSize(void);

/** Returns current codec status (see 'ULTRASONICLIB_CODEC_STATUS' enum) */
ULTRASONICLIB_CODEC_STATUS ultrasoniclib_getCodecStatus(void* const hUS);

/**
 * (Optional) Returns current intialisation/processing progress, between 0..1
 *  - 0: intialisation/processing has started
 *  - 1: intialisation/processing has ended
 */
float ultrasoniclib_getProgressBar0_1(void* const hUS);

/**
 * (Optional) Returns current intialisation/processing progress text
 *
 * @note "text" string should be (at least) of length:
 *       ultrasoniclib_PROGRESSBARTEXT_CHAR_LENGTH
 */
void ultrasoniclib_getProgressBarText(void* const hUS, char* text);

/**
 * Returns the pitch shift option (see ULTRASONICLIB_PITCHSHFT_OPTIONS enum)
 * enum)
 */
ULTRASONICLIB_PITCHSHFT_OPTIONS ultrasoniclib_getPitchShiftOption(void* const hUS);

/** Returns the number of channels required by the current configuration */
int ultrasoniclib_getNInputCHrequired(void* const hUS);

/** Returns the number of channels required by the current configuration */
int ultrasoniclib_getNOutputCHrequired(void* const hUS);

/** Returns the DAW/Host sample rate */
int ultrasoniclib_getDAWsamplerate(void* const hAmbi);

/** Returns the DoA averaging coefficient */
float ultrasoniclib_getDoAaveragingCoeff(void* const hUS);

/** Returns the gain being applied to the output signals */
float ultrasoniclib_getPostGain_dB(void* const hUS);

/** Returns whether the diffuseness values are being used to "duck" the output*/
int ultrasoniclib_getEnableDiffuseness(void* const hUS);
    
    
#ifdef __cplusplus
} /* extern "C" { */
#endif /* __cplusplus */

#endif /* __ULTRASONICLIB_H_INCLUDED__ */
