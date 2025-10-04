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
 * @file ultrasonic_internal.c
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

void ultrasoniclib_setCodecStatus(void* const hBM, ULTRASONICLIB_CODEC_STATUS newStatus)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hBM);
    if(newStatus==CODEC_STATUS_NOT_INITIALISED){
        /* Pause until current initialisation is complete */
        while(pData->codecStatus == CODEC_STATUS_INITIALISING)
            SAF_SLEEP(10);
    }
    pData->codecStatus = newStatus;
}

void ultrasoniclib_interpHRTFs
(
    void* const hBM,
    float azimuth_deg,
    float elevation_deg,
    int band,
    float_complex h_intrp[NUM_EARS]
)
{
    ultrasoniclib_data *pData = (ultrasoniclib_data*)(hBM);
    int i;
    int aziIndex, elevIndex, N_azi, idx3d;
    float_complex ipd;
    float aziRes, elevRes, weights[3], itds3[3],  itdInterp;
    float magnitudes3[3][NUM_EARS], magInterp[NUM_EARS];

    /* find closest pre-computed VBAP direction */
    aziRes = (float)pData->hrtf_vbapTableRes[0];
    elevRes = (float)pData->hrtf_vbapTableRes[1];
    N_azi = (int)(360.0f / aziRes + 0.5f) + 1;
    aziIndex = (int)(matlab_fmodf(azimuth_deg + 180.0f, 360.0f) / aziRes + 0.5f);
    elevIndex = (int)((elevation_deg + 90.0f) / elevRes + 0.5f);
    idx3d = elevIndex * N_azi + aziIndex;
    for (i = 0; i < 3; i++)
        weights[i] = pData->hrtf_vbap_gtableComp[idx3d*3 + i];

    /* retrieve the 3 itds and hrtf magnitudes */
    for (i = 0; i < 3; i++) {
        itds3[i] = pData->itds_s[pData->hrtf_vbap_gtableIdx[idx3d*3+i]];
        magnitudes3[i][0] = pData->hrtf_fb_mag[band*NUM_EARS*(pData->N_hrir_dirs) + 0*(pData->N_hrir_dirs) + pData->hrtf_vbap_gtableIdx[idx3d*3+i]];
        magnitudes3[i][1] = pData->hrtf_fb_mag[band*NUM_EARS*(pData->N_hrir_dirs) + 1*(pData->N_hrir_dirs) + pData->hrtf_vbap_gtableIdx[idx3d*3+i]];

    }

    /* interpolate hrtf magnitudes and itd */
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 1, 1, 3, 1.0f,
                (float*)weights, 3,
                (float*)itds3, 1, 0.0f,
                &itdInterp, 1);
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 1, 2, 3, 1.0f,
                (float*)weights, 3,
                (float*)magnitudes3, 2, 0.0f,
                (float*)magInterp, 2);


    /* introduce interaural phase difference */
    ipd = cmplxf(0.0f, (matlab_fmodf(2.0f*SAF_PI*(pData->freqVector_syn[band]) * itdInterp + SAF_PI, 2.0f*SAF_PI) - SAF_PI)/2.0f);
    h_intrp[0] = crmulf(cexpf(ipd), magInterp[0]);
    h_intrp[1] = crmulf(conjf(cexpf(ipd)), magInterp[1]);
}

