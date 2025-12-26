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

namespace ColoursUI {
    const juce::Colour bgDark1          { 0xff12242e }; //const juce::Colour bgDark1   { 0xff142833 };
    const juce::Colour bgDark2          { 0xff041316 }; //const juce::Colour bgDark2   { 0xff041518 };
    const juce::Colour panelFill        { 0x0ef4f4f4 }; //const juce::Colour panelFill { 0x10f4f4f4 };
    const juce::Colour panelFillLight   { 0x07f4f4f4 }; //const juce::Colour panelFillLight { 0x08f4f4f4 };
    const juce::Colour panelStroke { 0x67a0a0a0 };
    const juce::Colour panelStrokeLight { 0x35a0a0a0 };
    const juce::Colour textWhite { juce::Colours::white };
    const juce::Colour borderGrey { 0xffb9b9b9 };
    const juce::Colour accentCyan { 0xff00d8df };
    const juce::Colour accentOrange { 0xffdf8400 };
}

inline void drawPanel(juce::Graphics& g, juce::Rectangle<int> r, juce::Colour fill, juce::Colour stroke, int thickness = 1)
{
    g.setColour(fill);
    g.fillRect(r);
    g.setColour(stroke);
    g.drawRect(r, thickness);
}

inline void drawVerticalGradient(juce::Graphics& g, juce::Rectangle<int> r, juce::Colour top, juce::Colour bottom)
{
    g.setGradientFill(juce::ColourGradient(top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false));
    g.fillRect(r);
}

inline void drawLabel(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, float size,
                      juce::Justification just = juce::Justification::centredLeft, juce::Colour colour = ColoursUI::textWhite,
                      juce::Font::FontStyleFlags style = juce::Font::bold)
{
    g.setColour(colour);
    g.setFont(juce::FontOptions(size, style));
    g.drawText(text, r, just, true);
}


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
    startTimer(40);

    /* warnings */
    currentWarning = k_warning_none;
}

PluginEditor::~PluginEditor()
{
    stopTimer();
}

void PluginEditor::paint (juce::Graphics& g)
{
    using namespace ColoursUI;

    /* Background gradients */
    drawVerticalGradient(g, {0, 30, 530, 40}, bgDark1, bgDark2);
    drawVerticalGradient(g, {0, 70, 498, 42}, bgDark2, bgDark1);

    /* Top rounded bar */
    {
        juce::Rectangle<float> r {1.f, 2.f, 498.f, 31.f};
        g.setGradientFill(juce::ColourGradient(bgDark2,
                                               r.getX(), r.getBottom(),
                                               bgDark1,
                                               r.getRight() + 30.f, r.getY(),
                                               false));
        g.fillRoundedRectangle(r, 5.f);
        g.setColour(borderGrey);
        g.drawRoundedRectangle(r, 5.f, 2.f);
    }

    /* Panels */
    drawPanel(g, {10,40,476,32}, panelFill, panelStroke);
    drawPanel(g, {10,71,476,33}, panelFill, panelStroke);

    /* Borders (height = 120 px) */
    g.setColour(borderGrey);
    g.drawRect({0,   0, 532, 2}, 2);
    g.drawRect({0,   0,   2,120}, 2);
    g.drawRect({498, 3,   6,117}, 2);
    g.drawRect({0, 110, 512, 2}, 2);

    /* Title */
    drawLabel(g, {16,1,272,32}, "UltrasonicSuperHearing", 18.8f);

    /* Labels */
    drawLabel(g, {16,40,144,30}, "DoA Averaging:", 15.f);
    drawLabel(g, {15,71,96,35},  "Pitch Shift:",   15.f);
    drawLabel(g, {232,72,144,30}, "Post Gain (dB)", 15.f);
    drawLabel(g, {319,40,144,30}, "Enable Diff:",   15.f);

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

void PluginEditor::timerCallback()
{
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
