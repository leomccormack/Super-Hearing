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

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>("pitchShift", "PitchShift",
                                                                  juce::StringArray{"none","Down 1 Oct","Down 2 Oct","Down 3 Oct","Use CH7"}, 0,
                                                                  AudioParameterChoiceAttributes().withAutomatable(false)));
    return { params.begin(), params.end() };
}

void PluginProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "pitchShift"){
        ultrasoniclib_setPitchShiftOption(hUS, static_cast<ULTRASONICLIB_PITCHSHFT_OPTIONS>(newValue+1.001f));
    }
}

void PluginProcessor::setParameterValuesUsingInternalState()
{
    setParameterValue("pitchShift", ultrasoniclib_getPitchShiftOption(hUS)-1);
}

void PluginProcessor::setInternalStateUsingParameterValues()
{
    ultrasoniclib_setPitchShiftOption(hUS, static_cast<ULTRASONICLIB_PITCHSHFT_OPTIONS>(getParameterChoice("pitchShift")+1));
}

PluginProcessor::PluginProcessor() :
	AudioProcessor(BusesProperties()
		.withInput("Input", AudioChannelSet::discreteChannels(6), true)
	    .withOutput("Output", AudioChannelSet::discreteChannels(2), true)),
    ParameterManager(*this, createParameterLayout())
{
	nSampleRate = 0;
	ultrasoniclib_create(&hUS);
    
    /* Grab defaults */
    setParameterValuesUsingInternalState();
    
    startTimer(TIMER_PROCESSING_RELATED, 80);
}

PluginProcessor::~PluginProcessor()
{
	ultrasoniclib_destroy(&hUS);
}

void PluginProcessor::setCurrentProgram (int /*index*/)
{
}

const String PluginProcessor::getName() const
{
    return JucePlugin_Name;
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

void PluginProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    nHostBlockSize = samplesPerBlock;
    nNumInputs =  jmin(getTotalNumInputChannels(), 256);
    nNumOutputs = jmin(getTotalNumOutputChannels(), 256);
    nSampleRate = (int)(sampleRate + 0.5);

	ultrasoniclib_init(hUS, nSampleRate);
}

void PluginProcessor::releaseResources()
{
}

void PluginProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& /*midiMessages*/)
{
    ScopedNoDenormals noDenormals;
    
    int nCurrentBlockSize = nHostBlockSize = buffer.getNumSamples();
    nNumInputs = jmin(getTotalNumInputChannels(), buffer.getNumChannels(), 256);
    nNumOutputs = jmin(getTotalNumOutputChannels(), buffer.getNumChannels(), 256);
    float* const* bufferData = buffer.getArrayOfWritePointers();
    float* pFrameData[256];
    int frameSize = ultrasoniclib_getFrameSize();

	if(nCurrentBlockSize % frameSize == 0) { /* divisible by frame size */
        for(int frame = 0; frame < nCurrentBlockSize/frameSize; frame++) {
            for(int ch = 0; ch < jmin(buffer.getNumChannels(), 256); ch++)
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
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (MemoryBlock& destData)
{
    juce::ValueTree state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setTagName("ULTRASONICLIBAUDIOPLUGINSETTINGS");
    xml->setAttribute("VersionCode", JucePlugin_VersionCode); // added since 0x10001
    
    /* Now for the other DSP object parameters (that have no JUCE parameter counterpart) */
    xml->setAttribute("DOAAVERAGING", ultrasoniclib_getDoAaveragingCoeff(hUS));
    xml->setAttribute("POSTGAIN", ultrasoniclib_getPostGain_dB(hUS));
    xml->setAttribute("ENABLEDIFF", ultrasoniclib_getEnableDiffuseness(hUS));

    /* Save */
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    /* Load */
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName("ULTRASONICLIBAUDIOPLUGINSETTINGS")){
        if(!xmlState->hasAttribute("VersionCode")){ // pre-0x10001
            if(xmlState->hasAttribute("PITCHSHIFTOPTION"))
                ultrasoniclib_setPitchShiftOption(hUS, (ULTRASONICLIB_PITCHSHFT_OPTIONS)xmlState->getIntAttribute("PITCHSHIFTOPTION", ULTRASONICLIB_PITCHSHFT_DOWN_3_OCT));
            if(xmlState->hasAttribute("DOAAVERAGING"))
                ultrasoniclib_setDoAaveragingCoeff(hUS, (float)xmlState->getDoubleAttribute("DOAAVERAGING", 0.9f));
            if(xmlState->hasAttribute("POSTGAIN"))
                ultrasoniclib_setPostGain_dB(hUS, (float)xmlState->getDoubleAttribute("POSTGAIN", 0.9f));
            if(xmlState->hasAttribute("ENABLEDIFF"))
                ultrasoniclib_setEnableDiffuseness(hUS, xmlState->getIntAttribute("ENABLEDIFF", 0));

            
            setParameterValuesUsingInternalState();
        }
        else if(xmlState->getIntAttribute("VersionCode")>=0x10001){
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            
            /* Now for the other DSP object parameters (that have no JUCE parameter counterpart) */
            if(xmlState->hasAttribute("DOAAVERAGING"))
                ultrasoniclib_setDoAaveragingCoeff(hUS, (float)xmlState->getDoubleAttribute("DOAAVERAGING", 0.9f));
            if(xmlState->hasAttribute("POSTGAIN"))
                ultrasoniclib_setPostGain_dB(hUS, (float)xmlState->getDoubleAttribute("POSTGAIN", 0.9f));
            if(xmlState->hasAttribute("ENABLEDIFF"))
                ultrasoniclib_setEnableDiffuseness(hUS, xmlState->getIntAttribute("ENABLEDIFF", 0));
            
            /* Many hosts will also trigger parameterChanged() for all parameters after calling setStateInformation() */
            /* However, some hosts do not. Therefore, it is better to ensure that the internal state is always up-to-date by calling: */
            setInternalStateUsingParameterValues();
        }
        
        ultrasoniclib_refreshParams(hUS);
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

