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

#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor(p), processor(p), progressbar(progress)
{
    CBpitchShift = std::make_unique<ComboBoxWithAttachment>(p.parameters, "pitchShift");
    addAndMakeVisible (CBpitchShift.get());
    CBpitchShift->setEditableText (false);
    CBpitchShift->setJustificationType (juce::Justification::centredLeft);
    CBpitchShift->setTextWhenNothingSelected (juce::String());
    CBpitchShift->addListener (this);

    CBpitchShift->setBounds (98, 78, 112, 19);

    s_DoAestimation.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_DoAestimation.get());
    s_DoAestimation->setRange (0, 0.99, 0.01);
    s_DoAestimation->setSliderStyle (juce::Slider::LinearHorizontal);
    s_DoAestimation->setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    s_DoAestimation->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    s_DoAestimation->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    s_DoAestimation->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_DoAestimation->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_DoAestimation->addListener (this);

    s_DoAestimation->setBounds (119, 40, 144, 32);

    s_postGain_dB.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_postGain_dB.get());
    s_postGain_dB->setRange (-6, 12, 0.01);
    s_postGain_dB->setSliderStyle (juce::Slider::LinearHorizontal);
    s_postGain_dB->setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    s_postGain_dB->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    s_postGain_dB->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    s_postGain_dB->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_postGain_dB->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_postGain_dB->addListener (this);

    s_postGain_dB->setBounds (336, 72, 144, 32);

    tb_enableDiff.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (tb_enableDiff.get());
    tb_enableDiff->setButtonText (juce::String());
    tb_enableDiff->addListener (this);

    tb_enableDiff->setBounds (440, 44, 32, 24);

    setSize (500, 112);

    hUS = processor.getFXHandle();

	/* fetch current configuration */
    s_DoAestimation->setValue((double)ultrasoniclib_getDoAaveragingCoeff(hUS), dontSendNotification);
    s_postGain_dB->setValue((double)ultrasoniclib_getPostGain_dB(hUS), dontSendNotification);
    tb_enableDiff->setToggleState((bool)ultrasoniclib_getEnableDiffuseness(hUS), dontSendNotification);

    /* tooltips */
    CBpitchShift->setTooltip("Pitch shift");

    addAndMakeVisible (publicationLink);
    publicationLink.setColour (HyperlinkButton::textColourId, Colours::lightblue);
    publicationLink.setBounds(getBounds().getWidth()-80, 4, 80, 12);
    publicationLink.setJustificationType(Justification::centredLeft);

    /* Specify screen refresh rate */
    startTimer(TIMER_GUI_RELATED, 40);

    /* warnings */
    currentWarning = k_warning_none;
}

PluginEditor::~PluginEditor()
{
    CBpitchShift = nullptr;
    s_DoAestimation = nullptr;
    s_postGain_dB = nullptr;
    tb_enableDiff = nullptr;
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);

    {
        int x = 0, y = 70, width = 498, height = 42;
        juce::Colour fillColour1 = juce::Colour (0xff1c3949), fillColour2 = juce::Colour (0xff071e22);
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             8.0f - 0.0f + x,
                                             120.0f - 70.0f + y,
                                             fillColour2,
                                             8.0f - 0.0f + x,
                                             96.0f - 70.0f + y,
                                             false));
        g.fillRect (x, y, width, height);
    }

    {
        int x = 10, y = 71, width = 476, height = 33;
        juce::Colour fillColour = juce::Colour (0x10c7c7c7);
        juce::Colour strokeColour = juce::Colour (0x1fffffff);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 0, y = 30, width = 530, height = 40;
        juce::Colour fillColour1 = juce::Colour (0xff1c3949), fillColour2 = juce::Colour (0xff071e22);
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             8.0f - 0.0f + x,
                                             32.0f - 30.0f + y,
                                             fillColour2,
                                             8.0f - 0.0f + x,
                                             56.0f - 30.0f + y,
                                             false));
        g.fillRect (x, y, width, height);
    }

    {
        float x = 1.0f, y = 2.0f, width = 498.0f, height = 31.0f;
        juce::Colour fillColour1 = juce::Colour (0xff061c20), fillColour2 = juce::Colour (0xff1c3949);
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             0.0f - 1.0f + x,
                                             32.0f - 2.0f + y,
                                             fillColour2,
                                             528.0f - 1.0f + x,
                                             32.0f - 2.0f + y,
                                             false));
        g.fillRoundedRectangle (x, y, width, height, 5.000f);
        g.setColour (strokeColour);
        g.drawRoundedRectangle (x, y, width, height, 5.000f, 2.000f);
    }

    {
        int x = 10, y = 40, width = 476, height = 32;
        juce::Colour fillColour = juce::Colour (0x10c7c7c7);
        juce::Colour strokeColour = juce::Colour (0x1fffffff);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 16, y = 1, width = 272, height = 32;
        juce::String text (TRANS("UltrasonicSuperHearing"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (18.80f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 0, y = 0, width = 532, height = 2;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 0, y = 0, width = 2, height = 120;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 498, y = 3, width = 6, height = 117;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 0, y = 110, width = 512, height = 2;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 15, y = 71, width = 96, height = 35;
        juce::String text (TRANS("Pitch Shift:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 16, y = 40, width = 144, height = 30;
        juce::String text (TRANS("DoA Averaging:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 232, y = 72, width = 144, height = 30;
        juce::String text (TRANS("Post Gain (dB)"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 319, y = 40, width = 144, height = 30;
        juce::String text (TRANS("Enable Diff:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }
    
    /* display version/date built */
	g.setColour(Colours::white);
	g.setFont(juce::FontOptions (11.00f, Font::plain));
	g.drawText(TRANS("Ver ") + JucePlugin_VersionString + BUILD_VER_SUFFIX + TRANS(", Build Date ") + __DATE__ + TRANS(" "),
		230, 16, 530, 11,
		Justification::centredLeft, true);

    /* display warning message */
    g.setColour(Colours::red);
    g.setFont(juce::FontOptions (11.00f, Font::plain));
    switch (currentWarning){
        case k_warning_none:
            break;
        case k_warning_supported_fs:
            g.drawText(TRANS("Set sample rate to 192kHz"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_frameSize:
            g.drawText(TRANS("Set frame size to multiple of ") + String(ultrasoniclib_getFrameSize()),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NInputCH:
            g.drawText(TRANS("Insufficient number of input channels (") + String(processor.getTotalNumInputChannels()) +
                       TRANS("/") + String(ultrasoniclib_getNInputCHrequired(hUS)) + TRANS(")"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NOutputCH:
            g.drawText(TRANS("Insufficient number of output channels (") + String(processor.getTotalNumOutputChannels()) +
                       TRANS("/") + String(ultrasoniclib_getNOutputCHrequired(hUS)) + TRANS(")"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
    }
}

void PluginEditor::resized()
{
	repaint();
}

void PluginEditor::comboBoxChanged (juce::ComboBox* /*comboBoxThatHasChanged*/)
{
}

void PluginEditor::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == s_DoAestimation.get())
    {
        ultrasoniclib_setDoAaveragingCoeff(hUS, (float)s_DoAestimation->getValue());
    }
    else if (sliderThatWasMoved == s_postGain_dB.get())
    {
        ultrasoniclib_setPostGain_dB(hUS, (float)s_postGain_dB->getValue());
    }
}

void PluginEditor::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == tb_enableDiff.get())
    {
        ultrasoniclib_setEnableDiffuseness(hUS, (int)tb_enableDiff->getToggleState());
    }
}

void PluginEditor::timerCallback(int timerID)
{
    switch(timerID){
        case TIMER_PROCESSING_RELATED:
            /* handled in PluginProcessor */
            break;

        case TIMER_GUI_RELATED:
            /* parameters whos values can change internally should be periodically refreshed */

            /* Progress bar */
            if(ultrasoniclib_getCodecStatus(hUS)==CODEC_STATUS_INITIALISING){
                addAndMakeVisible(progressbar);
                progress = (double)ultrasoniclib_getProgressBar0_1(hUS);
                char text[ULTRASONICLIB_PROGRESSBARTEXT_CHAR_LENGTH];
                ultrasoniclib_getProgressBarText(hUS, (char*)text);
                progressbar.setTextToDisplay(String(text));
            }
            else
                removeChildComponent(&progressbar);

            /* display warning message, if needed */
            if ( ((ultrasoniclib_getDAWsamplerate(hUS) < 192e3)) ){
                currentWarning = k_warning_supported_fs;
                repaint(0,0,getWidth(),32);
            }
            else if ((processor.getCurrentBlockSize() % ultrasoniclib_getFrameSize()) != 0){
                currentWarning = k_warning_frameSize;
                repaint(0,0,getWidth(),32);
            }
            else if ((processor.getCurrentNumInputs() < ultrasoniclib_getNInputCHrequired(hUS))){
                currentWarning = k_warning_NInputCH;
                repaint(0,0,getWidth(),32);
            }
            else if ((processor.getCurrentNumOutputs() < ultrasoniclib_getNOutputCHrequired(hUS))){
                currentWarning = k_warning_NOutputCH;
                repaint(0,0,getWidth(),32);
            }
    }
}
