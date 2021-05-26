%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This script implements the DoA estimator described in [1] and tests it
% based on simulating a spatial recording using either measurements of the
% 6 sensor ultrasonic microphone array, or an analytical description of it.
%
% [1] Politis, A., Delikaris-Manias, S. and Pulkki, V., 2015, April. 
%     Direction-of-arrival and diffuseness estimation above spatial 
%     aliasing for symmetrical directional microphone arrays. In 2015 IEEE 
%     International Conference on Acoustics, Speech and Signal Processing 
%     (ICASSP) (pp. 6-10). IEEE.
%
% Author: Leo McCormack
% Date: 22.02.2020
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all,close all %#ok
addpath('resources_')

% Check that dependencies have been added to MATLAB's list of paths
if ~exist('Spherical-Harmonic-Transform', 'dir')
    error('Spherical-Harmonic-Transform library not found. Find it here: https://github.com/polarch/Spherical-Harmonic-Transform')
end
if ~exist('Spherical-Array-Processing', 'dir')
    error('Spherical-Array-Processing library not found. Find it here: https://github.com/polarch/Spherical-Array-Processing')
end
if ~exist('Array-Response-Simulator', 'dir')
    error('Array-Response-Simulator library not found. Find it here: https://github.com/polarch/Array-Response-Simulator')
end
 

%% User parameters
% The simulation of the spatial scene may be based either on ANALYTICAL
% descriptions of the 6 sensor microphone array, or based on the
% MEASURED of the real array made in an anechoic chambre
simulationData = 'MEASURED'; % {'ANALYTICAL', 'MEASURED'}
fs = 192e3;        % sampling rate
Lfilt = 1024;      % filter length (for the simulation/encoding filters)
lSig = fs;         % length of the synthetic signal
TEST_SNR_dB = 15;  % Signal to noise ratio of the simulation

% rigid microphone of radius 5.5mm 
order = 1;
R = 0.0055; %  
mic_dirs_deg = ...
    [90 0;     
    0 -90;     
    0 0;      
    -90 0;      
    180 0;     
    0 90];      
mic_dirs_rad = mic_dirs_deg*pi/180; 
mic_dirs_xyz = unitSph2cart(mic_dirs_rad);
nMics = size(mic_dirs_deg,1);

% Apply rotation to match the orientation the array was in during the
% listening test [0,0 is on the cube vertex surrounded by sensors 3,1,6]
Rzyx = euler2rotationMatrix(-pi/4,pi/4,0,'zyx');
mic_dirs_xyz = mic_dirs_xyz*Rzyx;
mic_dirs_rad = unitCart2sph(mic_dirs_xyz);
mic_dirs_deg = mic_dirs_rad*180/pi; 
plotMicArray(mic_dirs_deg, R);


%% Simulate microphone array signals
% Based on a single source in a diffuse-field with a specific SNR
switch simulationData
    case 'ANALYTICAL'
        src_dirs_deg = [-90 45];  
         
        % Simulate directional transfer functions for the rigid microphone
        % array. Type and order of approximation:
        arrayType = 'rigid';
        N_order = 30;  

        % Obtain responses  
        [~,grid_dirs_rad] = getTdesign(21);
        src_dirs_rad = src_dirs_deg*pi/180;
        [h_mic_d, H_mic_d] = simulateSphArray(Lfilt, mic_dirs_rad, grid_dirs_rad, arrayType, R, N_order, fs);
        [h_mic_s, H_mic_s] = simulateSphArray(Lfilt, mic_dirs_rad, src_dirs_rad, arrayType, R, N_order, fs); 
 
        % See e.g.:
        %
        % [1] Earl G. Williams, "Fourier Acoustics: Sound Radiation and Nearfield 
        %     Acoustical Holography", Academic Press, 1999
        % 
        % [2] Heinz Teutsch, "Modal Array Signal Processing: Principles and 
        %     Applications of Acoustic Wavefield Decomposition", Springer, 2007
 
        % "Record" a synthetic sound scene
        nSrc = size(src_dirs_deg,1);
        nDiff = size(grid_dirs_rad,1);
        s_sig = randn(lSig, nSrc)./nSrc;   % incoherent noise sources
        d_sig = randn(lSig, nDiff)./nDiff; % incoherent noise sources, to serve as the diffuse sources
        micSig_s = matrixConvolver(s_sig, permute(h_mic_s, [1 3 2]), Lfilt/2);  % source component
        micSig_d = matrixConvolver(d_sig, permute(h_mic_d, [1 3 2]), Lfilt/2);  % diffuse component

        % direct to diffuse balance - for microphone pressure signals
        scaleFactor_dB = 10*log10(rms(sum(micSig_s,2))/rms(sum(micSig_d,2))) - TEST_SNR_dB;
        micSig_d = 10.^(scaleFactor_dB/10) .* micSig_d;   
        test_snr = 10*log10( rms(sum(micSig_s,2))/rms(sum(micSig_d,2))); % check that we actually have the target SNR
        micSig = micSig_s + micSig_d;
        micSig = 0.99.*micSig./max(abs(micSig(:)));
 
        % Evaluate the performance of the microphone array for spherical harmonic acquisition
        % Filters for simulating  
        maxG_dB = 10; % maximum allowed gain amplification of the encoding filters 
        [H_filt, h_filt] = arraySHTfiltersTheory_radInverse(R, nMics, order, Lfilt, fs, maxG_dB);
        aziElev2aziPolar = @(dirs) [dirs(:,1) pi/2-dirs(:,2)]; % function to convert from azimuth-inclination to azimuth-elevation
        Y_mics = sqrt(4*pi) * getSH(order, aziElev2aziPolar(mic_dirs_rad), 'real'); % real SH matrix for microphone directions
        h_filt_r = replicatePerOrder(h_filt/sqrt(4*pi), 2);
        for kk=1:Lfilt/2+1
            M_mic2sh_radinv(:,:,kk) = diag(replicatePerOrder(H_filt(kk,:)/sqrt(4*pi),2))*Y_mics.'; %#ok   
        end
        Y_grid = sqrt(4*pi) * getSH(order, aziElev2aziPolar(grid_dirs_rad), 'real'); % real SH matrix for microphone directions
        evaluateSHTfilters(M_mic2sh_radinv, H_mic_d, fs, Y_grid); % v2 only evaulates on horizontal

    case 'MEASURED' 
        % Some example IR measurements of the array, which were made in an
        % anechoic chamber for 12 directions
        load('UltrasonicMic_IRs.mat')
           
        % Select measured array transfer function:
        measurement_idx = 6; % 6->[90,0]
        src_dirs_deg = micMeas_dirs_deg(measurement_idx,:);
        H_mic = micMeas(:,:,measurement_idx);
        
        % Source signal 
        [src_sig, fs_in] = audioread('Myotis_bechsteinii_5_o_192.wav'); % recording taken from: http://www.batcalls.com/
        if fs_in ~= fs, src_sig=resample(src_sig,fs,fs_in); end
        src_sig = src_sig(1:lSig,:);
        
        % Synthesise sensor signals 
        micSig_s = matrixConvolver(src_sig, permute(H_mic, [1 3 2]), 512);   % source components
        micSig_n = randn(size(micSig_s)); %  noise
        
        % Balance
        scaleFactor_dB = 10*log10(rms(sum(micSig_s,2))/rms(sum(micSig_n,2))) - TEST_SNR_dB;
        micSig_n = 10.^(scaleFactor_dB/10) .* micSig_n;   
        test_snr = 10*log10( rms(sum(micSig_s,2))/rms(sum(micSig_n,2))); % check that we actually have the target SNR
        micSig = micSig_s + micSig_n;
        micSig = 0.99.*micSig./max(abs(micSig(:)));
end

 
%% DoA estimation
% Forward STFT
nFFT = 512;
nSH = (order+1)*(order+1);
hopsize = nFFT/2;
framesize = nFFT*4; 
nBins_anl = nFFT/2+1; % nBins used for analysis
timeslots = framesize/hopsize;
numTimeslots = ceil(size(micSig,1)/nFFT);
micSigTF = zeros(nFFT, numTimeslots, nMics);
for mic = 1:nMics
    micSigTF(:,:,mic) = fft(buffer(micSig(:,mic),nFFT));
end

% DoA estimation initialisations
signalLengthSamples = size(micSig,1); 
aliasingFreq = 20e3;
if strcmp('MEASURED', simulationData), minFreq = aliasingFreq;
else, minFreq = 10e3; end
freqVector = (0:nFFT/2)'*fs/nFFT;
[~,minIdx] = min(abs(freqVector-minFreq));
[~,aliasIdx] = min(abs(freqVector-aliasingFreq));

% Pre-allocate parameter storage
azim = zeros(nBins_anl, floor(numTimeslots/timeslots)-1);
elev = zeros(nBins_anl, floor(numTimeslots/timeslots)-1);
energy = zeros(nBins_anl, floor(numTimeslots/timeslots)-1);

% Main Loop
startIndex = 1;
frameIndex = 1;
progress = 1;  
while startIndex+timeslots < numTimeslots
    if (startIndex+timeslots)*10/numTimeslots > progress
        fprintf('*');
        progress=progress+1;
    end
    
    % Load time slots
    TSrange = startIndex + (0:timeslots-1);    
    micFrame = permute(micSigTF(:,TSrange,:), [3 2 1]); % (nCH,timeslots,nBands) 
     
    % Analysis above minimum frequency, but below aliasing frequency
    for band=1:nBins_anl
        if freqVector(band)>minFreq && freqVector(band)<aliasingFreq
            % encode to spherical harmonics
            shFrame = M_mic2sh_radinv(:,:,band) * micFrame(:,:,band);
            
            % DoA estimation via active-intensity
            WXYZ = [shFrame(1,:); shFrame(4,:)/sqrt(3); shFrame(2,:)/sqrt(3); shFrame(3,:)/sqrt(3); ];  
            pvCOV = (WXYZ*WXYZ'); 
            I = real(pvCOV(2:4,1)); 
            [azim(band,frameIndex), elev(band,frameIndex)] = cart2sph(I(1,:), I(2,:), I(3,:));  
            energy(band,frameIndex) = 0.5.*trace(pvCOV);  
 
        elseif freqVector(band)>=aliasingFreq
            % DoA estimation via inter-sensor magnitude steering 
            r_doa = sum((sum(abs(micFrame(:,:,band)),2)*ones(1,3)) .* mic_dirs_xyz, 1).';
            [azim(band,frameIndex), elev(band,frameIndex)] = cart2sph(r_doa(1,:), r_doa(2,:), r_doa(3,:));
            
            % energy of sensor 6 (looking up)
            energy(band,frameIndex) = sum(abs(micFrame(6,:,band)).^2, 2); 
        end
    end
    
    startIndex = startIndex + timeslots;
    frameIndex = frameIndex+1;  
end

fprintf('\nCompleted\n');
 

%% Plots
azim = azim*180/pi;
elev = elev*180/pi;

figure, semilogx(freqVector, [azim(:, end/2), elev(:, end/2)]), xlim([20e3 96e3]), grid on
legend('azimuth', 'elevation'), xlabel('freq (Hz)'), ylabel('angle (degrees)')
    
figure, subplot(3,1,1), imagesc(azim(minIdx:end,:))
colorbar, axis xy, caxis([-180 180]), title('azimuth (degrees)')
xlabel('time'), ylabel('frequency')
subplot(3,1,2), imagesc(elev(minIdx:end,:))
colorbar, axis xy, caxis([-90 90]), title('elevation (degrees)')
xlabel('time'), ylabel('frequency')
subplot(3,1,3), imagesc(10*log(energy(minIdx:end,:)))
colorbar, axis xy, caxis([-80 0]), title('energy (dB)')
xlabel('time'), ylabel('frequency')

 
