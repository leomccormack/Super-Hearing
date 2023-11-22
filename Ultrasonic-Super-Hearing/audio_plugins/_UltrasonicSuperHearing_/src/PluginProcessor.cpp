/*
 ==============================================================================

 This file is part of UltrasonicSuperHearing.
 Copyright (c) 2020 - Leo McCormack.

 UltrasonicSuperHearing is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 UltrasonicSuperHearing is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with UltrasonicSuperHearing.  If not, see <http://www.gnu.org/licenses/>.

 ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginProcessor::PluginProcessor() : 
	AudioProcessor(BusesProperties()
		.withInput("Input", AudioChannelSet::discreteChannels(6), true)
	    .withOutput("Output", AudioChannelSet::discreteChannels(2), true))
{
	nSampleRate = 0;
	ultrasoniclib_create(&hUS);
    startTimer(TIMER_PROCESSING_RELATED, 80);
}

PluginProcessor::~PluginProcessor()
{
	ultrasoniclib_destroy(&hUS);
}

void PluginProcessor::setParameter (int index, float newValue)
{
	switch (index) { 
        case k_pitchShift:   ultrasoniclib_setPitchShiftOption(hUS, (ULTRASONICLIB_PITCHSHFT_OPTIONS)(int)(newValue*(float)(ULTRASONICLIB_MUM_PITCHSHFT_OPTIONS-1) + 1.5f)); break;
		default: break;
	}
}

void PluginProcessor::setCurrentProgram (int /*index*/)
{
}

float PluginProcessor::getParameter (int index)
{
    switch (index) {
        case k_pitchShift:    return (float)(ultrasoniclib_getPitchShiftOption(hUS)-1)/(float)(ULTRASONICLIB_MUM_PITCHSHFT_OPTIONS-1);
		default: return 0.0f;
	}
}

int PluginProcessor::getNumParameters()
{
	return k_NumOfParameters;
}

const String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

const String PluginProcessor::getParameterName (int index)
{
    switch (index){
        case k_pitchShift: return "pitchShift";
		default: return "NULL";
	}
}

const String PluginProcessor::getParameterText(int index)
{
    switch (index) {
        case k_pitchShift:
            switch(ultrasoniclib_getPitchShiftOption(hUS)){
                case ULTRASONICLIB_PITCHSHFT_NONE:       return "none";
                case ULTRASONICLIB_PITCHSHFT_DOWN_1_OCT: return "Down 1 Oct";
                case ULTRASONICLIB_PITCHSHFT_DOWN_2_OCT: return "Down 2 Oct";
                case ULTRASONICLIB_PITCHSHFT_DOWN_3_OCT: return "Down 3 Oct";
                case ULTRASONICLIB_PITCHSHFT_USE_CHANNEL_7: return "Use CH7";
                default: return "NULL";
            }
        default: return "NULL";
    }
}

const String PluginProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String PluginProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

const String PluginProcessor::getProgramName (int /*index*/)
{
    return String();
}


bool PluginProcessor::isInputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool PluginProcessor::isOutputChannelStereoPair (int /*index*/) const
{
    return true;
}


bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

void PluginProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    nHostBlockSize = samplesPerBlock;
    nNumInputs =  getTotalNumInputChannels();
    nNumOutputs = getTotalNumOutputChannels();
    nSampleRate = (int)(sampleRate + 0.5);

	ultrasoniclib_init(hUS, nSampleRate);
}

void PluginProcessor::releaseResources()
{
}

void PluginProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& /*midiMessages*/)
{
    int nCurrentBlockSize = nHostBlockSize = buffer.getNumSamples();
    nNumInputs = jmin(getTotalNumInputChannels(), buffer.getNumChannels());
    nNumOutputs = jmin(getTotalNumOutputChannels(), buffer.getNumChannels());
    float* const* bufferData = buffer.getArrayOfWritePointers();
    float* pFrameData[64/* max num channels in VST standard */];
    int frameSize = ultrasoniclib_getFrameSize();

	if(nCurrentBlockSize % frameSize == 0) { /* divisible by frame size */
        for(int frame = 0; frame < nCurrentBlockSize/frameSize; frame++) {
            for(int ch = 0; ch < buffer.getNumChannels(); ch++)
                pFrameData[ch] = &bufferData[ch][frame*frameSize];
            
			/* perform processing */
			ultrasoniclib_process(hUS, pFrameData, pFrameData, nNumInputs, nNumOutputs, frameSize);
		}
	}
	else
		buffer.clear();
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; 
}

AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (this);
}

//==============================================================================
void PluginProcessor::getStateInformation (MemoryBlock& destData)
{
	/* Create an outer XML element.. */ 
	XmlElement xml("ULTRASONICLIBAUDIOPLUGINSETTINGS");
 
	xml.setAttribute("PITCHSHIFTOPTION", ultrasoniclib_getPitchShiftOption(hUS));
    xml.setAttribute("DOAAVERAGING", ultrasoniclib_getDoAaveragingCoeff(hUS));
    xml.setAttribute("POSTGAIN", ultrasoniclib_getPostGain_dB(hUS));
    xml.setAttribute("ENABLEDIFF", ultrasoniclib_getEnableDiffuseness(hUS));
    
	copyXmlToBinary(xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	/* This getXmlFromBinary() function retrieves XML from the binary blob */
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState != nullptr) {
		/* make sure that it's actually the correct XML object */
		if (xmlState->hasTagName("ULTRASONICLIBAUDIOPLUGINSETTINGS")) {
            if(xmlState->hasAttribute("PITCHSHIFTOPTION"))
                ultrasoniclib_setPitchShiftOption(hUS, (ULTRASONICLIB_PITCHSHFT_OPTIONS)xmlState->getIntAttribute("PITCHSHIFTOPTION", ULTRASONICLIB_PITCHSHFT_DOWN_3_OCT));
            if(xmlState->hasAttribute("DOAAVERAGING"))
                ultrasoniclib_setDoAaveragingCoeff(hUS, (float)xmlState->getDoubleAttribute("DOAAVERAGING", 0.9f));
            if(xmlState->hasAttribute("POSTGAIN"))
                ultrasoniclib_setPostGain_dB(hUS, (float)xmlState->getDoubleAttribute("POSTGAIN", 0.9f));

            if(xmlState->hasAttribute("ENABLEDIFF"))
                ultrasoniclib_setEnableDiffuseness(hUS, xmlState->getIntAttribute("ENABLEDIFF", 0));

            ultrasoniclib_refreshParams(hUS);
        }
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

