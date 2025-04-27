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
 * @file ultrasoniclib.c
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

#include "ultrasoniclib.h"
#include "ultrasonic_internal.h"

/* Specifications for the ultrasonic mic: */
#if 0
static float mic_radius = 0.0055f;
static float mic_dirs_deg[6][2] = { {45.0f,0.0f}, {135.0f,-45.0f}, {-45.0f,-45.0f}, {-135.0f,0.0f}, {135.0f,45.0f}, {-45.0f,45.0f} };
static float mic_dirs_rad[6][2] = { {0.785398163397448f,0.0f}, {2.35619449019235f,-0.785398163397448f}, {-0.785398163397448f,-0.785398163397448f}, {-2.35619449019235f,0.0f}, {2.35619449019235f,0.785398163397448f}, {-0.785398163397448f,0.785398163397448f} };
#endif
static float mic_dirs_xyz[6][3] = { {0.707106781186548f,0.707106781186548f,0.0f}, {-0.5f,0.5f,-0.707106781186548f}, {0.5f,-0.5f,-0.707106781186548f},
    {-0.707106781186548f,-0.707106781186548f,0.0f}, {-0.5f,0.5f,0.707106781186548f}, {0.5f,-0.5f,0.707106781186548f} };

void ultrasoniclib_create
(
    void ** const phUS
)
{
    ultrasoniclib_data* pData = (ultrasoniclib_data*)malloc1d(sizeof(ultrasoniclib_data));
    *phUS = (void*)pData;
 
    /* Default user parameters */
    ultrasoniclib_setPitchShiftOption(*phUS, ULTRASONICLIB_PITCHSHFT_DOWN_2_OCT);
    pData->doaAveragingCoeff = 0.9f;
    pData->postGain_dB = 0.0f;
    pData->enableDiff = 0;

    /* internals */
    pData->hSmb = NULL;
    pData->progressBar0_1 = 0.0f;
    pData->progressBarText = malloc1d(ULTRASONICLIB_PROGRESSBARTEXT_CHAR_LENGTH*sizeof(char));
    strcpy(pData->progressBarText,"");
    pData->sampleRate = 0.0f;

    /* create filterbank */
#ifdef USE_QMF_FILTERBANK
    qmf_create(&(pData->hQMF), ULTRASONICLIB_NUM_INPUT_CHANNELS+1/* pitch shifted channel */, NUM_EARS, HOP_SIZE, 0, QMF_BANDS_CH_TIME);
#else
    afSTFT_create(&(pData->hSTFT), ULTRASONICLIB_NUM_INPUT_CHANNELS+1/* pitch shifted channel */, NUM_EARS, HOP_SIZE, 0, 0, AFSTFT_BANDS_CH_TIME);
#endif
    pData->pressure = malloc1d(FRAME_SIZE*sizeof(float));
    pData->dataTD = (float**)malloc2d(ULTRASONICLIB_NUM_INPUT_CHANNELS+1/* pitch shifted channel */, FRAME_SIZE, sizeof(float));
    pData->binDataTD = (float**)malloc2d(NUM_EARS, FRAME_SIZE, sizeof(float));
    pData->dataFD = (float_complex***)malloc3d(NBANDS_ANA, ULTRASONICLIB_NUM_INPUT_CHANNELS+1/* pitch shifted channel */, TIME_SLOTS, sizeof(float_complex));
    pData->binDataFD = (float_complex***)malloc3d(NBANDS_ANA, NUM_EARS, TIME_SLOTS, sizeof(float_complex));
 
    /* hrir data */
    pData->useDefaultHRIRsFLAG=1;
    pData->hrirs = NULL;
    pData->hrir_dirs_deg = NULL;
    pData->sofa_filepath = NULL;

    /* vbap (amplitude normalised) */
    pData->hrtf_vbap_gtableIdx = NULL;
    pData->hrtf_vbap_gtableComp = NULL;

    /* HRTF filterbank coefficients */
    pData->itds_s = NULL;
    pData->hrtf_fb = NULL;
    pData->hrtf_fb_mag = NULL;
    
    /* flags */
    pData->procStatus = PROC_STATUS_NOT_ONGOING;
    pData->codecStatus = CODEC_STATUS_NOT_INITIALISED;
}

void ultrasoniclib_destroy
(
    void ** const phUS
)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(*phUS);

    if (pData != NULL) {
        /* not safe to free memory during intialisation/processing loop */
        while (pData->codecStatus == CODEC_STATUS_INITIALISING ||
               pData->procStatus == PROC_STATUS_ONGOING){
            SAF_SLEEP(10);
        }

        if (pData->hSmb != NULL)
            smb_pitchShift_destroy(&(pData->hSmb));
#ifdef USE_QMF_FILTERBANK
        qmf_destroy(&(pData->hQMF));
#else
        afSTFT_destroy(&(pData->hSTFT));
#endif
        free(pData->pressure);
        free(pData->dataTD);
        free(pData->binDataTD);
        free(pData->dataFD);
        free(pData->binDataFD);
        free(pData->hrtf_vbap_gtableComp);
        free(pData->hrtf_vbap_gtableIdx);
        free(pData->hrtf_fb);
        free(pData->hrtf_fb_mag);
        free(pData->itds_s);
        free(pData->hrirs);
        free(pData->hrir_dirs_deg);
        free(pData);
        pData = NULL;
    }
}

void ultrasoniclib_init
(
    void * const hUS,
    int          sampleRate
)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);

    if(pData->sampleRate != sampleRate){
        pData->sampleRate = sampleRate;
#ifdef USE_QMF_FILTERBANK
        qmf_getCentreFreqs(pData->hQMF, (float)sampleRate, NBANDS_ANA, pData->freqVector_ana);
        memcpy(pData->freqVector_syn, pData->freqVector_ana, NBANDS_SYN*sizeof(float));
#else
        getUniformFreqVector(HOP_SIZE*2, sampleRate,  pData->freqVector_ana);
        getUniformFreqVector((HOP_SIZE*2)/4, sampleRate/4,  pData->freqVector_syn);
#endif
        ultrasoniclib_setCodecStatus(hUS, CODEC_STATUS_NOT_INITIALISED);
    }

    memset(pData->prev_r_xyz, 0, NBANDS_ANA*3*sizeof(float));
}

void ultrasoniclib_initCodec
(
    void* const hUS
)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    int i, nTriangles;
    float* hrtf_vbap_gtable;

    if (pData->codecStatus != CODEC_STATUS_NOT_INITIALISED)
        return; /* re-init not required, or already happening */
    while (pData->procStatus == PROC_STATUS_ONGOING){
        /* re-init required, but we need to wait for the current processing loop to end */
        pData->codecStatus = CODEC_STATUS_INITIALISING; /* indicate that we want to init */
        SAF_SLEEP(10);
    }
#ifdef SAF_ENABLE_SOFA_READER_MODULE
    SAF_SOFA_ERROR_CODES error;
    saf_sofa_container sofa;
#endif

    /* for progress bar */
    pData->codecStatus = CODEC_STATUS_INITIALISING;
    strcpy(pData->progressBarText,"Initialising ultrasoniclib");
    pData->progressBar0_1 = 0.0f;

    /* destroy current handle*/
    if (pData->hSmb != NULL)
        smb_pitchShift_destroy(&(pData->hSmb));

    /* Create new handle*/
    smb_pitchShift_create(&(pData->hSmb), 1, 4096, 16, pData->sampleRate);

    /* load sofa file or load default hrir data */
#ifdef SAF_ENABLE_SOFA_READER_MODULE
    if(!pData->useDefaultHRIRsFLAG && pData->sofa_filepath!=NULL){
        /* Load SOFA file */
        error = saf_sofa_open(&sofa, pData->sofa_filepath, SAF_SOFA_READER_OPTION_DEFAULT);

        /* Load defaults instead */
        if(error!=SAF_SOFA_OK || sofa.nReceivers!=NUM_EARS){
            pData->useDefaultHRIRsFLAG = 1;
            saf_print_warning("Unable to load the specified SOFA file, or it contained something other than 2 channels. Using default HRIR data instead.");
        }
        else{
            /* Copy SOFA data */
            pData->hrir_fs = (int)sofa.DataSamplingRate;
            pData->hrir_len = sofa.DataLengthIR;
            pData->N_hrir_dirs = sofa.nSources;
            pData->hrirs = realloc1d(pData->hrirs, pData->N_hrir_dirs*NUM_EARS*(pData->hrir_len)*sizeof(float));
            memcpy(pData->hrirs, sofa.DataIR, pData->N_hrir_dirs*NUM_EARS*(pData->hrir_len)*sizeof(float));
            pData->hrir_dirs_deg = realloc1d(pData->hrir_dirs_deg, pData->N_hrir_dirs*2*sizeof(float));
            cblas_scopy(pData->N_hrir_dirs, sofa.SourcePosition, 3, pData->hrir_dirs_deg, 2); /* azi */
            cblas_scopy(pData->N_hrir_dirs, &sofa.SourcePosition[1], 3, &pData->hrir_dirs_deg[1], 2); /* elev */
        }

        /* Clean-up */
        saf_sofa_close(&sofa);
    }
#else
    pData->useDefaultHRIRsFLAG = 1; /* Can only load the default HRIR data */
#endif
    if(pData->useDefaultHRIRsFLAG){
        /* Copy default HRIR data */
        pData->hrir_fs = __default_hrir_fs;
        pData->hrir_len = __default_hrir_len;
        pData->N_hrir_dirs = __default_N_hrir_dirs;
        pData->hrirs = realloc1d(pData->hrirs, pData->N_hrir_dirs*NUM_EARS*(pData->hrir_len)*sizeof(float));
        memcpy(pData->hrirs, (float*)__default_hrirs, pData->N_hrir_dirs*NUM_EARS*(pData->hrir_len)*sizeof(float));
        pData->hrir_dirs_deg = realloc1d(pData->hrir_dirs_deg, pData->N_hrir_dirs*2*sizeof(float));
        memcpy(pData->hrir_dirs_deg, (float*)__default_hrir_dirs_deg, pData->N_hrir_dirs*2*sizeof(float));
    }

    /* estimate the ITDs for each HRIR */
    pData->itds_s = realloc1d(pData->itds_s, pData->N_hrir_dirs*sizeof(float));
    estimateITDs(pData->hrirs, pData->N_hrir_dirs, pData->hrir_len, pData->hrir_fs, pData->itds_s);

    /* generate VBAP gain table */
    strcpy(pData->progressBarText,"Generating interpolation table");
    pData->progressBar0_1 = 0.6f;
    hrtf_vbap_gtable = NULL;
    pData->hrtf_vbapTableRes[0] = 2;
    pData->hrtf_vbapTableRes[1] = 5;
    generateVBAPgainTable3D(pData->hrir_dirs_deg, pData->N_hrir_dirs, pData->hrtf_vbapTableRes[0], pData->hrtf_vbapTableRes[1], 1, 0, 0.0f,
                            &hrtf_vbap_gtable, &(pData->N_hrtf_vbap_gtable), &(nTriangles));

    /* compress VBAP table (i.e. remove the zero elements) */
    pData->hrtf_vbap_gtableComp = realloc1d(pData->hrtf_vbap_gtableComp, pData->N_hrtf_vbap_gtable * 3 * sizeof(float));
    pData->hrtf_vbap_gtableIdx  = realloc1d(pData->hrtf_vbap_gtableIdx,  pData->N_hrtf_vbap_gtable * 3 * sizeof(int));
    compressVBAPgainTable3D(hrtf_vbap_gtable, pData->N_hrtf_vbap_gtable, pData->N_hrir_dirs, pData->hrtf_vbap_gtableComp, pData->hrtf_vbap_gtableIdx);

    /* convert hrirs to filterbank coefficients */
    strcpy(pData->progressBarText,"Applying HRIR diffuse-field EQ");
    pData->progressBar0_1 = 0.8f;
    pData->hrtf_fb = realloc1d(pData->hrtf_fb, NBANDS_SYN * NUM_EARS * (pData->N_hrir_dirs)*sizeof(float_complex));
#ifdef USE_QMF_FILTERBANK
    HRIRs2HRTFs_qmf(pData->hrirs, pData->N_hrir_dirs, pData->hrir_len, HOP_SIZE/4, 0, pData->hrtf_fb);
#else
    HRIRs2HRTFs_afSTFT(pData->hrirs, pData->N_hrir_dirs, pData->hrir_len, HOP_SIZE/4, 0, 0, pData->hrtf_fb);
#endif
    diffuseFieldEqualiseHRTFs(pData->N_hrir_dirs, pData->itds_s, pData->freqVector_syn, NBANDS_SYN, NULL, 1, 1, pData->hrtf_fb);

    /* calculate magnitude responses */
    pData->hrtf_fb_mag = realloc1d(pData->hrtf_fb_mag, NBANDS_SYN*NUM_EARS*(pData->N_hrir_dirs)*sizeof(float));
    for(i=0; i<NBANDS_SYN*NUM_EARS* (pData->N_hrir_dirs); i++)
        pData->hrtf_fb_mag[i] = cabsf(pData->hrtf_fb[i]);

    /* done! */
    strcpy(pData->progressBarText,"Done!");
    pData->progressBar0_1 = 1.0f;
    pData->codecStatus = CODEC_STATUS_INITIALISED;
}

void ultrasoniclib_process
(
    void  *  const hUS,
    float ** const inputs,
    float ** const outputs,
    int            nInputs,
    int            nOutputs,
    int            nSamples
)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    int ch, ear, band, t, band_count, enableDiff;
    float postGainLIN;
    float abs_sig[ULTRASONICLIB_NUM_INPUT_CHANNELS], abs_sig_xyz[ULTRASONICLIB_NUM_INPUT_CHANNELS][3];
    float doa_deg[2], doa_xyz[3];
    float norm, diff, rho[3];

    postGainLIN = powf(10.0f, pData->postGain_dB/20.0f);
    enableDiff = pData->enableDiff;

    if ( (FRAME_SIZE==nSamples) && (pData->codecStatus == CODEC_STATUS_INITIALISED) ){
        pData->procStatus = PROC_STATUS_ONGOING;

        /* Store input frame */
        for(ch=0; ch<SAF_MIN(ULTRASONICLIB_NUM_INPUT_CHANNELS,nInputs); ch++)
            memcpy(pData->dataTD[ch], inputs[ch], FRAME_SIZE*sizeof(float));
        for(; ch<ULTRASONICLIB_NUM_INPUT_CHANNELS; ch++)
            memset(pData->dataTD[ch], 0, FRAME_SIZE*sizeof(float));

        /* Channel 7 is the pitch shifted signal */
        if(pData->pitchShiftOption==ULTRASONICLIB_PITCHSHFT_USE_CHANNEL_7){
            for(ch=ULTRASONICLIB_NUM_INPUT_CHANNELS; ch<SAF_MIN(ULTRASONICLIB_NUM_INPUT_CHANNELS+1,nInputs); ch++)
                memcpy(pData->dataTD[ch], inputs[ch], FRAME_SIZE*sizeof(float));
        }
        else /* Pitch shifting of channel 6 (the one facing up), with pitch shift output to channel 7 */
            smb_pitchShift_apply(pData->hSmb, pData->pitchShift_factor, nSamples, pData->dataTD[5], pData->dataTD[6]);

        /* forward time-frequency transform */
#ifdef USE_QMF_FILTERBANK
        qmf_analysis(pData->hQMF, pData->dataTD, FRAME_SIZE, pData->dataFD);
#else
        afSTFT_forward(pData->hSTFT, pData->dataTD, FRAME_SIZE, pData->dataFD);
#endif

        /* DoA estimation (first 6 channels are the mic array signals) */
        memset(doa_xyz, 0, 3*sizeof(float));
        memset(rho, 0, 3*sizeof(float));
        band_count = 0;
        for(band=0; band<NBANDS_ANA; band++){
            if(pData->freqVector_ana[band] >= SPATIAL_ALIASING_FREQ && pData->freqVector_ana[band] <= MAXIMUM_ANALYSIS_FREQ){
                /* Compute signal Magnitudes  */
                for(ch=0; ch<ULTRASONICLIB_NUM_INPUT_CHANNELS; ch++){
                    abs_sig[ch] = 0.0f;
                    for(t=0; t<TIME_SLOTS; t++)
                        abs_sig[ch] += cabsf(pData->dataFD[band][ch][t]);
                }

                /* Apply to the sensor unit vectors */
                for(ch=0; ch<ULTRASONICLIB_NUM_INPUT_CHANNELS; ch++){
                    abs_sig_xyz[ch][0] = abs_sig[ch] * mic_dirs_xyz[ch][0];
                    abs_sig_xyz[ch][1] = abs_sig[ch] * mic_dirs_xyz[ch][1];
                    abs_sig_xyz[ch][2] = abs_sig[ch] * mic_dirs_xyz[ch][2];
                }

                /* Sum scaled vectors */
                memset(pData->r_xyz[band], 0, 3*sizeof(float));
                for(ch=0; ch<ULTRASONICLIB_NUM_INPUT_CHANNELS; ch++){
                    pData->r_xyz[band][0] += abs_sig_xyz[ch][0];
                    pData->r_xyz[band][1] += abs_sig_xyz[ch][1];
                    pData->r_xyz[band][2] += abs_sig_xyz[ch][2];
                }

                if(enableDiff){
                    norm = L2_norm3(pData->r_xyz[band])+2.23e-8f;
                    rho[0] += pData->r_xyz[band][0]/norm;
                    rho[1] += pData->r_xyz[band][1]/norm;
                    rho[2] += pData->r_xyz[band][2]/norm;
                }

                /* Average over time */
                pData->r_xyz[band][0] = pData->prev_r_xyz[band][0] * (pData->doaAveragingCoeff) + pData->r_xyz[band][0] * (1.0f-pData->doaAveragingCoeff);
                pData->r_xyz[band][1] = pData->prev_r_xyz[band][1] * (pData->doaAveragingCoeff) + pData->r_xyz[band][1] * (1.0f-pData->doaAveragingCoeff);
                pData->r_xyz[band][2] = pData->prev_r_xyz[band][2] * (pData->doaAveragingCoeff) + pData->r_xyz[band][2] * (1.0f-pData->doaAveragingCoeff);

                /* For next frame */
                pData->prev_r_xyz[band][0] = pData->r_xyz[band][0];
                pData->prev_r_xyz[band][1] = pData->r_xyz[band][1];
                pData->prev_r_xyz[band][2] = pData->r_xyz[band][2];

                /* average over frequency */
                doa_xyz[0] += pData->r_xyz[band][0];
                doa_xyz[1] += pData->r_xyz[band][1];
                doa_xyz[2] += pData->r_xyz[band][2];
                band_count++;
            }
        }
        doa_xyz[0] /= (float)band_count;
        doa_xyz[1] /= (float)band_count;
        doa_xyz[2] /= (float)band_count;
        if(enableDiff){
            rho[0] /= (float)band_count;
            rho[1] /= (float)band_count;
            rho[2] /= (float)band_count;
            diff = 1.0f - L2_norm3(rho);
        }
        else
            diff = 0.0f;

        /* Binauralise the pitch shifted channel (7) in the estimated DoA */
        memset(FLATTEN3D(pData->binDataFD), 0, NBANDS_ANA*NUM_EARS*TIME_SLOTS * sizeof(float_complex));
        for (band = 0; band < NBANDS_SYN; band++){

            /* Only binauralise pitch shifted signals above the new human threshold of hearing */
            if( ( pData->freqVector_syn[band] >= pData->pitchShift_factor*HUMAN_HEARING_MAX_FREQ) ){
                /* Interpolate HRTF based on estimated DoA for this bin */
                unitCart2sph(doa_xyz, 1, 1, doa_deg);
                ultrasoniclib_interpHRTFs(hUS, doa_deg[0], doa_deg[1], band, pData->hrtf_interp[band]);

                /* Binauralise pitch shifted channel */
                for (ear = 0; ear < NUM_EARS; ear++)
                    for (t = 0; t < TIME_SLOTS; t++)
                        pData->binDataFD[band][ear][t] = crmulf(ccaddf(pData->binDataFD[band][ear][t],
                                                                       ccmulf(pData->dataFD[band][6][t], pData->hrtf_interp[band][ear])), (1.0-diff));
            }
            else
                memset(FLATTEN2D(pData->binDataFD[band]), 0, NUM_EARS*TIME_SLOTS*sizeof(float_complex));
        } 

        /* Backward- time-frequency transform */
#ifdef USE_QMF_FILTERBANK
        qmf_synthesis(pData->hQMF, pData->binDataFD, FRAME_SIZE, pData->binDataTD);
#else
        afSTFT_backward(pData->hSTFT, pData->binDataFD, FRAME_SIZE, pData->binDataTD);
#endif
        cblas_sscal(NUM_EARS*FRAME_SIZE, postGainLIN, FLATTEN2D(pData->binDataTD), 1);

        /* Copy output frame */
        for(ch=0; ch<SAF_MIN(NUM_EARS,nOutputs); ch++)
            memcpy(outputs[ch], pData->binDataTD[ch], FRAME_SIZE*sizeof(float));
        for(; ch<nOutputs; ch++)
            memset(outputs[ch], 0, FRAME_SIZE*sizeof(float));
    }

    pData->procStatus = PROC_STATUS_NOT_ONGOING;
}


/* sets */

void ultrasoniclib_refreshParams(void* const hUS)
{
    ultrasoniclib_setCodecStatus(hUS, CODEC_STATUS_NOT_INITIALISED);
}

void ultrasoniclib_setPitchShiftOption(void* const hUS, ULTRASONICLIB_PITCHSHFT_OPTIONS newOption)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);

    pData->pitchShiftOption = newOption;
    switch(newOption){
        case ULTRASONICLIB_PITCHSHFT_NONE: pData->pitchShift_factor = 1.0f; break;
        case ULTRASONICLIB_PITCHSHFT_DOWN_1_OCT: pData->pitchShift_factor = 0.5f; break;
        case ULTRASONICLIB_PITCHSHFT_DOWN_2_OCT: pData->pitchShift_factor = 0.25f; break;
        case ULTRASONICLIB_PITCHSHFT_DOWN_3_OCT: pData->pitchShift_factor = 0.125f; break;
        case ULTRASONICLIB_PITCHSHFT_USE_CHANNEL_7: break;
    }
}

void ultrasoniclib_setDoAaveragingCoeff(void* const hUS, float newValue)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    pData->doaAveragingCoeff = SAF_CLAMP(newValue, 0.0f, 0.99f);
}

void ultrasoniclib_setPostGain_dB(void* const hUS, float newValue)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    pData->postGain_dB = newValue;
}

void ultrasoniclib_setEnableDiffuseness(void* const hUS, int newState)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    pData->enableDiff = newState;
}


/* gets */

int ultrasoniclib_getFrameSize(void)
{
    return FRAME_SIZE;
}

ULTRASONICLIB_CODEC_STATUS ultrasoniclib_getCodecStatus(void* const hUS)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    return pData->codecStatus;
}

float ultrasoniclib_getProgressBar0_1(void* const hUS)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    return pData->progressBar0_1;
}

void ultrasoniclib_getProgressBarText(void* const hUS, char* text)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    memcpy(text, pData->progressBarText, ULTRASONICLIB_PROGRESSBARTEXT_CHAR_LENGTH*sizeof(char));
}

float ultrasoniclib_getPitchShiftFactor(void* const hUS)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    return pData->pitchShift_factor;
}

ULTRASONICLIB_PITCHSHFT_OPTIONS ultrasoniclib_getPitchShiftOption(void* const hUS)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    return pData->pitchShiftOption;
}

int ultrasoniclib_getNInputCHrequired(void* const hUS)
{
    (void)hUS;
    return ULTRASONICLIB_NUM_INPUT_CHANNELS;
}

int ultrasoniclib_getNOutputCHrequired(void* const hUS)
{
    (void)hUS;
    return NUM_EARS;
}

int ultrasoniclib_getDAWsamplerate(void* const hUS)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    return pData->sampleRate;
}

float ultrasoniclib_getDoAaveragingCoeff(void* const hUS)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    return pData->doaAveragingCoeff;
}

float ultrasoniclib_getPostGain_dB(void* const hUS)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    return pData->postGain_dB;
}

int ultrasoniclib_getEnableDiffuseness(void* const hUS)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hUS);
    return pData->enableDiff;
}
