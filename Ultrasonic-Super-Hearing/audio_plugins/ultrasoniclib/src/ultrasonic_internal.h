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
 * @file batmic_internal.h
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

#ifndef __ULTRASONICLIB_INTERNAL_H_INCLUDED__
#define __ULTRASONICLIB_INTERNAL_H_INCLUDED__

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ultrasoniclib.h"
#include "saf.h"
#include "saf_externals.h" /* to also include saf dependencies (cblas etc.) */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ========================================================================== */
/*                            Internal Parameters                             */
/* ========================================================================== */

#define USE_QMF_FILTERBANK_DISABLED

#ifndef FRAME_SIZE
# define FRAME_SIZE ( 512 )
#endif
#define HOP_SIZE ( 512 )
#ifdef USE_QMF_FILTERBANK
# define NBANDS_ANA ( HOP_SIZE)
# define NBANDS_SYN ( HOP_SIZE/4)
#else
# define NBANDS_ANA ( HOP_SIZE + 1 )
# define NBANDS_SYN ( HOP_SIZE/4 + 1)
#endif
#define TIME_SLOTS ( FRAME_SIZE / HOP_SIZE )
#define TIME_SLOT_DELAY ( 9 ) /* afSTFT delay, or 12 when using hybrid mode */
#define SPATIAL_ALIASING_FREQ ( 17e3 )
#define HUMAN_HEARING_MAX_FREQ ( 20e3 )
#define MAXIMUM_ANALYSIS_FREQ ( 55e3 )
#if (FRAME_SIZE % HOP_SIZE != 0)
# error "FRAME_SIZE must be an integer multiple of HOP_SIZE"
#endif

/** Current status of the processing loop */
typedef enum _ULTRASONICLIB_PROC_STATUS{
    PROC_STATUS_ONGOING = 0, /**< Codec is processing input audio, and should
                              *   not be reinitialised at this time.*/
    PROC_STATUS_NOT_ONGOING  /**< Codec is not processing input audio, and may
                              *   be reinitialised if needed.*/
}ULTRASONICLIB_PROC_STATUS;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__STDC_NO_ATOMICS__)
  typedef _Atomic ULTRASONICLIB_PITCHSHFT_OPTIONS _Atomic_ULTRASONICLIB_PITCHSHFT_OPTIONS;
  typedef _Atomic ULTRASONICLIB_CODEC_STATUS _Atomic_ULTRASONICLIB_CODEC_STATUS;
  typedef _Atomic ULTRASONICLIB_PROC_STATUS _Atomic_ULTRASONICLIB_PROC_STATUS;
#else
  typedef ULTRASONICLIB_PITCHSHFT_OPTIONS _Atomic_ULTRASONICLIB_PITCHSHFT_OPTIONS;
  typedef ULTRASONICLIB_CODEC_STATUS _Atomic_ULTRASONICLIB_CODEC_STATUS
  typedef ULTRASONICLIB_PROC_STATUS _Atomic_ULTRASONICLIB_PROC_STATUS;
#endif

/* ========================================================================== */
/*                                 Structures                                 */
/* ========================================================================== */

/** Main struct for ultrasoniclib */
typedef struct _ultrasoniclib
{
    /* time-frequency transform and buffers */
    float freqVector_ana[NBANDS_ANA]; /**< Centre frequencies used for the analysis */
    float freqVector_syn[NBANDS_SYN]; /**< Centre frequencies used for the synthesis */
#ifdef USE_QMF_FILTERBANK
    void* hQMF;                       /**< Filterbank handle */
#else
    void* hSTFT;                      /**< Filterbank handle */
#endif
    float* pressure;                  /**< pressure signal; #FRAME_SIZE x 1 */
    float** dataTD;                   /**< Input time-domain microphone array signals; #ULTRASONICLIB_NUM_INPUT_CHANNELS x #FRAME_SIZE */
    float** binDataTD;                /**< Output time-domain binaural signals; #NUM_EARS x #FRAME_SIZE */
    float_complex*** dataFD;          /**< Input time-frequency-domain microphone array signals; #NBANDS_ANA x #ULTRASONICLIB_NUM_INPUT_CHANNELS x #TIME_SLOTS */
    float_complex*** binDataFD;       /**< Output time-frequency-domain binaural signals; #NBANDS_SYN x #NUM_EARS x #TIME_SLOTS */
    float sampleRate;                 /**< Current host sampling rate */

    /* DoA estimation */
    float prev_r_xyz[NBANDS_ANA][3];  /**< Previous DoA estimates per band (as unit Cartesian vectors), used for temporal averaging */
    float r_xyz[NBANDS_ANA][3];       /**< New DoA estimates (as unit Cartesian vectors) */

    /* internal */
    _Atomic_ULTRASONICLIB_CODEC_STATUS codecStatus; /**< see #ULTRASONICLIB_CODEC_STATUS */
    _Atomic_FLOAT32 progressBar0_1;                 /**< Current initialisation progress between 0..1 */
    char* progressBarText;                          /**< Current initialisation progress as a string */
    _Atomic_ULTRASONICLIB_PROC_STATUS procStatus;   /**< see #ULTRASONICLIB_PROC_STATUS */

    /* pitch shifting */
    void* hSmb;                          /**< Pitch-shifter handle*/
    float pitchShiftedFrame[FRAME_SIZE]; /**< Current pitch-shifted frame */
    float pitchShift_factor;             /**< e.g. 1: no shift, 0.5: down one octave, 0.25: down two octaves */

    /* Binauraliser */
    char* sofa_filepath;             /**< Optionally a SOFA file can be loaded for the HRIRs used for binauralisation */
    float* hrirs;                    /**< HRIR data; N_hrir_dirs x #NUM_EARS x hrir_len */
    float* hrir_dirs_deg;            /**< HRIR directions, in degrees; N_hrir_dirs x 2 */
    int N_hrir_dirs;                 /**< Number of HRIR directions in the current grid */
    int hrir_len;                    /**< Length of the HRIRs */
    int hrir_fs;                     /**< Sampling rate used to measure the HRIRs */
    int hrtf_vbapTableRes[2];        /**< Interpolation table resolution for the [azimuth, elevation], in degrees */
    int N_hrtf_vbap_gtable;          /**< Number of directions in the interpolation table */
    int* hrtf_vbap_gtableIdx;        /**< Interpolation table indices; N_hrtf_vbap_gtable x 3 */
    float* hrtf_vbap_gtableComp;     /**< Interpolation weights (for triangular interpolation, i.e. amplitude-normalised VBSP gains); N_hrtf_vbap_gtable x 3 */
    int useDefaultHRIRsFLAG;         /**< 0: use SOFA file found at sofa_filepath, 1: use default HRIR data */
    float* itds_s;                   /**< interaural-time differences for each HRIR (in seconds); nBands x 1 */
    float_complex* hrtf_fb;          /**< hrtf filterbank coefficients; nBands x nCH x N_hrirs */
    float* hrtf_fb_mag;              /**< magnitudes of the hrtf filterbank coefficients; nBands x nCH x N_hrirs */
    float_complex hrtf_interp[NBANDS_SYN][NUM_EARS]; /**< Interpolated HRTFs per band */

    /* user parameters */ 
    _Atomic_ULTRASONICLIB_PITCHSHFT_OPTIONS pitchShiftOption; /**< see #ULTRASONICLIB_PITCHSHFT_OPTIONS */
    _Atomic_FLOAT32 doaAveragingCoeff;         /**< Averaging coefficient (1-pole filter) for averaging the DoA estimates over time */
    _Atomic_FLOAT32 postGain_dB;               /**< Gain to be applied to the output signals, in decibels */
    _Atomic_INT32 enableDiff;                  /**< 1: enable the amplitude "ducking" based on the diffuseness parameter, 0: disabled */
    
} ultrasoniclib_data;


/* ========================================================================== */
/*                             Internal Functions                             */
/* ========================================================================== */

/** Sets codec status (see 'ULTRASONICLIB_CODEC_STATUS' enum) */
void ultrasoniclib_setCodecStatus(void* const hUS,
                                  ULTRASONICLIB_CODEC_STATUS newStatus);

/**
 * Interpolates between (up to) 3 HRTFs via amplitude-normalised VBAP gains.
 *
 * The HRTF magnitude responses and HRIR ITDs are interpolated seperately before
 * re-introducing the phase.
 *
 * @param[in]  hUS           batmic handle
 * @param[in]  azimuth_deg   Source azimuth in DEGREES
 * @param[in]  elevation_deg Source elevation in DEGREES
 * @param[in]  band          Band index
 * @param[out] h_intrp       Interpolated HRTF
 */
void ultrasoniclib_interpHRTFs(void* const hUS,
                               float azimuth_deg,
                               float elevation_deg,
                               int band,
                               float_complex h_intrp[NUM_EARS]);
    
    
#ifdef __cplusplus
} /* extern "C" { */
#endif /* __cplusplus */

#endif /* __ULTRASONICLIB_INTERNAL_H_INCLUDED__ */
