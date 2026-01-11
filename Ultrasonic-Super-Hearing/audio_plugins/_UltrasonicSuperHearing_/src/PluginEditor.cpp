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
    const juce::Colour borderGreyDarker { borderGrey.darker(0.2f) };
    const juce::Colour accentCyan { 0xff00d8df };
    const juce::Colour accentOrange { 0xffdf8400 };
}

inline void drawPanelRect(juce::Graphics& g,
                          juce::Rectangle<float> r,
                          juce::Colour fill,
                          juce::Colour stroke,
                          float thickness = 1)
{
    g.setColour(fill);
    g.fillRect(r);

    auto topTint    = fill.withAlpha(0.0f);
    auto bottomTint = fill.withAlpha(0.03f);
    g.setGradientFill(juce::ColourGradient{
        topTint,
        0.0f, (float)r.getY(),
        bottomTint,
        0.0f, (float)r.getBottom(),
        false
    });
    g.fillRect(r);
    g.setColour(stroke);
    g.drawRect(r, thickness);
}

inline void drawPanel(juce::Graphics& g,
                      juce::Rectangle<float> r,
                      juce::Colour fill,
                      juce::Colour stroke,
                      float thickness = 1,
                      float cornerSize = 5.0f)
{
    juce::Rectangle<float> rr = r.reduced(2.0f, 2.0f);
    g.setColour(fill);
    g.fillRoundedRectangle(rr, cornerSize);

    auto topTint    = fill.withAlpha(0.0f);
    auto bottomTint = fill.withAlpha(0.03f);
    g.setGradientFill(juce::ColourGradient{
        topTint,
        0.0f, (float)r.getY(),
        bottomTint,
        0.0f, (float)r.getBottom(),
        false
    });
    g.fillRoundedRectangle(rr, cornerSize);
    g.setColour(stroke);
    g.drawRoundedRectangle(rr, cornerSize, thickness);
}

inline void drawVerticalGradient(juce::Graphics& g, juce::Rectangle<int> r, juce::Colour top, juce::Colour bottom)
{
    g.setGradientFill(juce::ColourGradient(top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false));
    g.fillRect(r);
}

inline void drawLabel(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text,
                      float size, juce::Justification just = juce::Justification::centredLeft,
                      juce::Colour colour = ColoursUI::textWhite,
                      juce::Font::FontStyleFlags style = juce::Font::bold)
{
    g.setColour(colour);

    static auto interRegular = juce::Typeface::createSystemTypefaceFor(
        BinaryData::InterRegular_ttf,
        BinaryData::InterRegular_ttfSize
    );

    static auto interBold = juce::Typeface::createSystemTypefaceFor(
        BinaryData::InterBold_ttf,
        BinaryData::InterBold_ttfSize
    );

    juce::FontOptions font = juce::FontOptions(
        (style & juce::Font::bold) ? interBold : interRegular
    ).withHeight(size).withKerningFactor(0.03f);
    
    g.setFont(font);
    g.drawText(text, r, just, true);
}

inline void drawPluginBackgroundAndBanner(juce::Graphics& g,
                                          juce::Rectangle<int> bounds)
{
    using namespace ColoursUI;
    
    const int w = bounds.getWidth();
    const int h = bounds.getHeight();
    const float cornerSize = 12.0f;
    auto bgNewDark1 = bgDark1.brighter(0.06f);
    auto bgNewDark2 = bgDark2.darker(0.3f);
    
    g.fillAll(borderGreyDarker);

    /* Vertical gradient */
    {
        juce::ColourGradient grad(bgNewDark1, 0.f, 30.f,
                                  bgNewDark1, 0.f, float(h), false);

        grad.addColour(0.0, bgNewDark1);
        grad.addColour(0.1, bgDark1);
        grad.addColour(0.3, bgDark2);
        grad.addColour(0.5, bgNewDark2);
        grad.addColour(0.7, bgDark2);
        grad.addColour(0.9, bgDark1);
        grad.addColour(1.0, bgNewDark1);

        g.setGradientFill(grad);
        g.fillRoundedRectangle({0, 0, (float)w, (float)h}, cornerSize);
    }
    
    /* Bottom right radial gradient */
    {
        const float diameter = (float) juce::jmax(bounds.getWidth(), bounds.getHeight());
        const float cx = (float) bounds.getRight();
        const float cy = (float) bounds.getBottom();

        juce::Colour baseColour = Colours::black;
        juce::Colour centre = baseColour.withAlpha(0.25f);
        juce::Colour edge   = baseColour.withAlpha(0.0f);

        juce::ColourGradient radial(
            centre, cx, cy,
            edge,   cx, cy + diameter * 0.5f,
            true
        );

        g.setGradientFill(radial);
        g.fillEllipse(cx - diameter * 0.5f,
                      cy - diameter * 0.5f,
                      diameter,
                      diameter);
    }
    
    /* Top banner */
    {
        juce::Rectangle<float> r {1.f, 1.f, float(w - 2), 32.f};
        g.setGradientFill(juce::ColourGradient(bgDark2,
                                               r.getX(), r.getBottom(),
                                               bgDark1,
                                               r.getRight(), r.getY(),
                                               false));
        
        juce::Path p;
        p.addRoundedRectangle(r.getX(), r.getY(), r.getWidth(), r.getHeight(), 12.0f, 12.0f, true, true, false, false);
        g.fillPath(p);
        g.setColour(borderGreyDarker);
        g.strokePath(p, juce::PathStrokeType(2.0f));
    }

    /* Outer border */
    g.setColour(borderGreyDarker);
    g.drawRoundedRectangle({1, 1, (float)w-2, (float)h-2}, 12.0f, 2.0f);
    g.drawRoundedRectangle({0, 0, (float)w, (float)h}, 11.0f, 3.0f);
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

    drawPluginBackgroundAndBanner(g, getBounds());

    /* Panels */
    drawPanel(g, {10,40,476,32}, panelFill, panelStroke);
    drawPanel(g, {10,71,476,33}, panelFill, panelStroke);

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
    g.setFont(juce::FontOptions (11.00f, Font::plain));
    switch (currentWarning){
        case k_warning_none:
            break;
        case k_warning_supported_fs:
            g.setColour(Colours::red);
            g.drawText(TRANS("Set sample rate to 192kHz"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NInputCH:
            g.setColour(Colours::red);
            g.drawText(TRANS("Insufficient number of input channels (") + String(processor.getTotalNumInputChannels()) +
                       TRANS("/") + String(ultrasoniclib_getNInputCHrequired(hUS)) + TRANS(")"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NOutputCH:
            g.setColour(Colours::red);
            g.drawText(TRANS("Insufficient number of output channels (") + String(processor.getTotalNumOutputChannels()) +
                       TRANS("/") + String(ultrasoniclib_getNOutputCHrequired(hUS)) + TRANS(")"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_frameSize:
            g.setColour(Colours::yellow);
            g.drawText(TRANS("Set host block size to \"") + String(ultrasoniclib_getFrameSize()) + TRANS("\" for lowest latency"),
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
    else if ((processor.getCurrentNumInputs() < ultrasoniclib_getNInputCHrequired(hUS))){
        currentWarning = k_warning_NInputCH;
        repaint(0,0,getWidth(),32);
    }
    else if ((processor.getCurrentNumOutputs() < ultrasoniclib_getNOutputCHrequired(hUS))){
        currentWarning = k_warning_NOutputCH;
        repaint(0,0,getWidth(),32);
    }
    else if ((processor.getCurrentBlockSize() != ultrasoniclib_getFrameSize())){
        currentWarning = k_warning_frameSize;
        repaint(0,0,getWidth(),32);
    }
    else if(currentWarning){
        currentWarning = k_warning_none;
        repaint(0,0,getWidth(),32);
    }
}
